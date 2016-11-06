/*
 *  Copyright (C) 2007  Thiago Macieira <thiago@kde.org>
 *  Copyright (C) 2016  Daniel Dewald <daniel.dewald@innogames.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SvnHelper.h"

#include <QMap>
#include <QIODevice>
#include <QMapIterator>

#include <svn_fs.h>
#include <svn_pools.h>
#include <svn_repos.h>
#include <svn_types.h>

#include "AprAutoPool.h"

#include "rules/RuleStats.h"

#include "git/GitRepositoryTransaction.h"

#include "CommandLineParser.h"

QList<RuleMatch>::ConstIterator SvnHelper::findMatchRule(const QList<RuleMatch>& matchRules, int revnum, const QString& current, int ruleMask)
{
    QList<RuleMatch>::ConstIterator it = matchRules.constBegin(), end = matchRules.constEnd();
    
    for ( ; it != end; ++it) 
    {
        if (it->minRevision > revnum)
        {
            continue;
        }
        
        if (it->maxRevision != -1 && it->maxRevision < revnum)
        {
            continue;
        }
        
        if (it->action == Ignore && ruleMask & NoIgnoreRule)
        {
            continue;
        }
        
        if (it->action == Recurse && ruleMask & NoRecurseRule)
        {
            continue;
        }
        
        if (it->rx.indexIn(current) == 0) 
        {
            RuleStats::instance()->ruleMatched(*it, revnum);
            return it;
        }
    }

    // no match
    return end;
}

int SvnHelper::pathMode(svn_fs_root_t *fs_root, const char *pathname, apr_pool_t *pool)
{
    svn_string_t *propvalue;
    SVN_INT_ERR(svn_fs_node_prop(&propvalue, fs_root, pathname, "svn:executable", pool));
    int mode = 0100644;
    
    if (propvalue)
    {
        mode = 0100755;
    }

    return mode;
}

svn_error_t* SvnHelper::deviceWrite(void *baton, const char *data, apr_size_t *len)
{
    QIODevice *device = reinterpret_cast<QIODevice *>(baton);
    device->write(data, *len);

    while (device->bytesToWrite() > 32*1024) 
    {
        if (!device->waitForBytesWritten(-1)) 
        {
            qFatal("Failed to write to process: %s", qPrintable(device->errorString()));
            return svn_error_createf(APR_EOF, SVN_NO_ERROR, "Failed to write to process: %s", qPrintable(device->errorString()));
        }
    }
    
    return SVN_NO_ERROR;
}

svn_error_t* SvnHelper::QIODevice_write(void* baton, const char* data, apr_size_t* len)
{
    QIODevice *device = reinterpret_cast<QIODevice *>(baton);
    device->write(data, *len);

    while (device->bytesToWrite() > 32*1024) 
    {
        if (!device->waitForBytesWritten(-1)) 
        {
            qFatal("Failed to write to process: %s", qPrintable(device->errorString()));
            return svn_error_createf(APR_EOF, SVN_NO_ERROR, "Failed to write to process: %s", qPrintable(device->errorString()));
        }
    }
    
    return SVN_NO_ERROR;
}

svn_stream_t* SvnHelper::streamForDevice(QIODevice *device, apr_pool_t *pool)
{
    svn_stream_t *stream = svn_stream_create(device, pool);
    svn_stream_set_write(stream, QIODevice_write);

    return stream;
}

int SvnHelper::dumpBlob(GitRepositoryTransaction* txn, svn_fs_root_t* fs_root, const char* pathname, const QString& finalPathName, apr_pool_t* pool)
{
    AprAutoPool dumppool(pool);
    // what type is it?
    
    int mode = pathMode(fs_root, pathname, dumppool);

    svn_filesize_t stream_length;

    SVN_INT_ERR(svn_fs_file_length(&stream_length, fs_root, pathname, dumppool));

    svn_stream_t *in_stream, *out_stream;
    if (!CommandLineParser::instance()->contains("dry-run")) 
    {
        // open the file
        SVN_INT_ERR(svn_fs_file_contents(&in_stream, fs_root, pathname, dumppool));
    }

    // maybe it's a symlink?
    svn_string_t *propvalue;
    SVN_INT_ERR(svn_fs_node_prop(&propvalue, fs_root, pathname, "svn:special", dumppool));
    
    if (propvalue) 
    {
        apr_size_t len = strlen("link ");
        
        if (!CommandLineParser::instance()->contains("dry-run")) 
        {
            QByteArray buf;
            buf.reserve(len);
            SVN_INT_ERR(svn_stream_read_full(in_stream, buf.data(), &len));
            
            if (len == strlen("link ") && strncmp(buf, "link ", len) == 0) 
            {
                mode = 0120000;
                stream_length -= len;
            } 
            else 
            {
                //this can happen if a link changed into a file in one commit
                qWarning("file %s is svn:special but not a symlink", pathname);
                // re-open the file as we tried to read "link "
                svn_stream_close(in_stream);
                SVN_INT_ERR(svn_fs_file_contents(&in_stream, fs_root, pathname, dumppool));
            }
        }
    }

    QIODevice *io = txn->addFile(finalPathName, mode, stream_length);

    if (!CommandLineParser::instance()->contains("dry-run")) 
    {
        // open a generic svn_stream_t for the QIODevice
        out_stream = streamForDevice(io, dumppool);
        SVN_INT_ERR(svn_stream_copy3(in_stream, out_stream, NULL, NULL, dumppool));

        // print an ending newline
        io->putChar('\n');
    }

    return EXIT_SUCCESS;
}

int SvnHelper::recursiveDumpDir(GitRepositoryTransaction* txn, svn_fs_root_t* fs_root, const QByteArray& pathname, const QString& finalPathName, apr_pool_t* pool)
{
    // get the dir listing
    apr_hash_t* entries;
    SVN_INT_ERR(svn_fs_dir_entries(&entries, fs_root, pathname, pool));
    AprAutoPool dirpool(pool);

    // While we get a hash, put it in a map for sorted lookup, so we can
    // repeat the conversions and get the same git commit hashes.
    QMap<QByteArray, svn_node_kind_t> map;
    
    for (apr_hash_index_t *i = apr_hash_first(pool, entries); i; i = apr_hash_next(i)) 
    {
        const void *vkey;
        void *value;
        apr_hash_this(i, &vkey, NULL, &value);
        svn_fs_dirent_t *dirent = reinterpret_cast<svn_fs_dirent_t *>(value);
        map.insertMulti(QByteArray(dirent->name), dirent->kind);
    }

    QMapIterator<QByteArray, svn_node_kind_t> i(map);
    while (i.hasNext()) 
    {
        dirpool.clear();
        i.next();
        QByteArray entryName = pathname + '/' + i.key();
        QString entryFinalName = finalPathName + QString::fromUtf8(i.key());

        if (i.value() == svn_node_dir) 
        {
            entryFinalName += '/';
            if (recursiveDumpDir(txn, fs_root, entryName, entryFinalName, dirpool) == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }
        } 
        else if (i.value() == svn_node_file) 
        {
            printf("+");
            fflush(stdout);
            
            if (dumpBlob(txn, fs_root, entryName, entryFinalName, dirpool) == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

bool SvnHelper::wasDir(svn_fs_t* fs, int revnum, const char* pathname, apr_pool_t* pool)
{
    AprAutoPool subpool(pool);
    svn_fs_root_t* fs_root;
    
    if (svn_fs_revision_root(&fs_root, fs, revnum, subpool) != SVN_NO_ERROR)
    {
        return false;
    }

    svn_boolean_t is_dir;
    
    if (svn_fs_is_dir(&is_dir, fs_root, pathname, subpool) != SVN_NO_ERROR)
    {
        return false;
    }

    return is_dir;
}

time_t SvnHelper::getEpoch(const char* svn_date)
{
    struct tm tm;
    memset(&tm, 0, sizeof tm);
    QByteArray date(svn_date, strlen(svn_date) - 8);
    strptime(date, "%Y-%m-%dT%H:%M:%S", &tm);
    
    return timegm(&tm);
}

