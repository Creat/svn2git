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

#include "SvnRevision.h"

#include <QMap>
#include <QDebug>
#include <QByteArray>
#include <QMapIterator>

#include <svn_pools.h>
#include <svn_types.h>
#include <svn_repos.h>

#include "git/GitRepository.h"
#include "git/GitRepositoryTransaction.h"

#include "SvnHelper.h"

#include "commandline/CommandLineParser.h"

SvnRevision::SvnRevision(int revision, svn_fs_t* f, apr_pool_t* parent_pool) : 
    pool(parent_pool), 
    fs(f), 
    fs_root(0), 
    revnum(revision), 
    propsFetched(false)
{
    ruledebug = CommandLineParser::instance()->contains( QLatin1String("debug-rules"));
}

int SvnRevision::open()
{
    SVN_INT_ERR(svn_fs_revision_root(&fs_root, fs, revnum, pool));
    return EXIT_SUCCESS;
}

void SvnRevision::splitPathName(const RuleMatch& rule, const QString& pathName, QString* svnprefix_p, QString* repository_p, QString* effectiveRepository_p, QString* branch_p, QString* path_p)
{
    QString svnprefix = pathName;
    svnprefix.truncate(rule.rx.matchedLength());

    if (svnprefix_p) 
    {
        *svnprefix_p = svnprefix;
    }

    if (repository_p) 
    {
        *repository_p = svnprefix;
        repository_p->replace(rule.rx, rule.repository);
        
        foreach (RuleMatchSubstitution subst, rule.repo_substs) 
        {
            subst.apply(*repository_p);
        }
    }

    if (effectiveRepository_p) 
    {
        *effectiveRepository_p = svnprefix;
        effectiveRepository_p->replace(rule.rx, rule.repository);
        
        foreach (RuleMatchSubstitution subst, rule.repo_substs) 
        {
            subst.apply(*effectiveRepository_p);
        }
        
        GitRepository *repository = repositories.value(*effectiveRepository_p, 0);
        
        if (repository) 
        {
            *effectiveRepository_p = repository->getEffectiveRepository()->getName();
        }
    }

    if (branch_p) 
    {
        *branch_p = svnprefix;
        branch_p->replace(rule.rx, rule.branch);
        
        foreach (RuleMatchSubstitution subst, rule.branch_substs) 
        {
            subst.apply(*branch_p);
        }
    }

    if (path_p) 
    {
        QString prefix = svnprefix;
        prefix.replace(rule.rx, rule.prefix);
        *path_p = prefix + pathName.mid(svnprefix.length());
    }
}

int SvnRevision::prepareTransactions()
{
    // find out what was changed in this revision:
    apr_hash_t *changes;
    SVN_INT_ERR(svn_fs_paths_changed2(&changes, fs_root, pool));

    QMap<QByteArray, svn_fs_path_change2_t*> map;
    for (apr_hash_index_t *i = apr_hash_first(pool, changes); i; i = apr_hash_next(i)) 
    {
        const void *vkey;
        void *value;
        apr_hash_this(i, &vkey, NULL, &value);
        const char *key = reinterpret_cast<const char *>(vkey);
        svn_fs_path_change2_t *change = reinterpret_cast<svn_fs_path_change2_t *>(value);
        // If we mix path deletions with path adds/replaces we might erase a
        // branch after that it has been reset -> history truncated
        
        if (map.contains(QByteArray(key))) 
        {
            // If the same path is deleted and added, we need to put the
            // deletions into the map first, then the addition.
            if (change->change_kind == svn_fs_path_change_delete) 
            {
                // XXX
            }
            
            fprintf(stderr, "\nDuplicate key found in rev %d: %s\n", revnum, key);
            fprintf(stderr, "This needs more code to be handled, file a bug report\n");
            fflush(stderr);
            exit(1);
        }
        map.insertMulti(QByteArray(key), change);
    }

    QMapIterator<QByteArray, svn_fs_path_change2_t*> i(map);
    
    while (i.hasNext()) 
    {
        i.next();
        if (exportEntry(i.key(), i.value(), changes) == EXIT_FAILURE)
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int SvnRevision::fetchRevProps()
{
    if( propsFetched )
    {
        return EXIT_SUCCESS;
    }

    apr_hash_t* revprops;
    SVN_INT_ERR(svn_fs_revision_proplist(&revprops, fs, revnum, pool));
    svn_string_t* svnauthor = (svn_string_t*)apr_hash_get(revprops, "svn:author", APR_HASH_KEY_STRING);
    svn_string_t* svndate = (svn_string_t*)apr_hash_get(revprops, "svn:date", APR_HASH_KEY_STRING);
    svn_string_t* svnlog = (svn_string_t*)apr_hash_get(revprops, "svn:log", APR_HASH_KEY_STRING);

    if (svnlog)
    {
        log = svnlog->data;
    }
    else
    {
        log.clear();
    }
    
    authorident = svnauthor ? identities.value(svnauthor->data) : QByteArray();
    epoch = svndate ? SvnHelper::getEpoch(svndate->data) : 0;
    
    if (authorident.isEmpty()) 
    {
        if (!svnauthor || svn_string_isempty(svnauthor))
        {
            authorident = "nobody <nobody@localhost>";
        }
        else
        {
            authorident = svnauthor->data + QByteArray(" <") + svnauthor->data + QByteArray("@") + userdomain.toUtf8() + QByteArray(">");
        }
    }
    
    propsFetched = true;
    
    return EXIT_SUCCESS;
}

int SvnRevision::commit()
{
    // now create the commit
    if (fetchRevProps() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }
    
    foreach (GitRepository* repo, repositories.values()) 
    {
        repo->commit();
    }

    foreach (GitRepositoryTransaction *txn, transactions) 
    {
        txn->setAuthor(authorident);
        txn->setDateTime(epoch);
        txn->setLog(log);

        txn->commit();
        delete txn;
    }

    return EXIT_SUCCESS;
}

int SvnRevision::exportEntry(const char *key, const svn_fs_path_change2_t *change, apr_hash_t *changes)
{
    AprAutoPool revpool(pool.data());
    QString current = QString::fromUtf8(key);

    // was this copied from somewhere?
    svn_revnum_t rev_from = SVN_INVALID_REVNUM;
    const char *path_from = NULL;
    
    if (change->change_kind != svn_fs_path_change_delete) 
    {
        // svn_fs_copied_from would fail on deleted paths, because the path
        // obviously no longer exists in the current revision
        SVN_INT_ERR(svn_fs_copied_from(&rev_from, &path_from, fs_root, key, revpool));
    }

    // is this a directory?
    svn_boolean_t is_dir;
    SVN_INT_ERR(svn_fs_is_dir(&is_dir, fs_root, key, revpool));

    // Adding newly created directories
    if (is_dir && change->change_kind == svn_fs_path_change_add && path_from == NULL && CommandLineParser::instance()->contains("empty-dirs")) 
    {
        QString keyQString = key;
        
        // Skipping SVN-directory-layout
        if (keyQString.endsWith("/trunk") || keyQString.endsWith("/branches") || keyQString.endsWith("/tags")) 
        {
            //qDebug() << "Skipping SVN-directory-layout:" << keyQString;
            return EXIT_SUCCESS;
        }
        
        needCommit = true;
        //qDebug() << "Adding directory:" << key;
    }
    // svn:ignore-properties
    else if (is_dir && (change->change_kind == svn_fs_path_change_add || change->change_kind == svn_fs_path_change_modify) && path_from == NULL && CommandLineParser::instance()->contains("svn-ignore")) 
    {
        needCommit = true;
    }
    else if (is_dir) 
    {
        if (change->change_kind == svn_fs_path_change_modify || change->change_kind == svn_fs_path_change_add) 
        {
            if (path_from == NULL) 
            {
                // freshly added directory, or modified properties
                // Git doesn't handle directories, so we don't either
                //qDebug() << "   mkdir ignored:" << key;
                return EXIT_SUCCESS;
            }

            qDebug() << "   " << key << "was copied from" << path_from << "rev" << rev_from;
        } 
        else if (change->change_kind == svn_fs_path_change_replace) 
        {
            if (path_from == NULL)
            {
                qDebug() << "   " << key << "was replaced";
            }
            else
            {
                qDebug() << "   " << key << "was replaced from" << path_from << "rev" << rev_from;
            }
        } 
        else if (change->change_kind == svn_fs_path_change_reset) 
        {
            qCritical() << "   " << key << "was reset, panic!";
            return EXIT_FAILURE;
        } 
        else 
        {
            // if change_kind == delete, it shouldn't come into this arm of the 'is_dir' test
            qCritical() << "   " << key << "has unhandled change kind " << change->change_kind << ", panic!";
            return EXIT_FAILURE;
        }
    } 
    else if (change->change_kind == svn_fs_path_change_delete) 
    {
        is_dir = SvnHelper::wasDir(fs, revnum - 1, key, revpool);
    }

    if (is_dir)
    {
        current += '/';
    }

    //MultiRule: loop start
    //Replace all returns with continue,
    bool isHandled = false;
    
    foreach ( const QList<RuleMatch> matchRules, allMatchRules ) 
    {
        // find the first rule that matches this pathname
        QList<RuleMatch>::ConstIterator match = SvnHelper::findMatchRule(matchRules, revnum, current);
        if (match != matchRules.constEnd()) 
        {
            const RuleMatch &rule = *match;
            
            if ( exportDispatch(key, change, path_from, rev_from, changes, current, rule, matchRules, revpool) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            
            isHandled = true;
        } 
        else if (is_dir && path_from != NULL) 
        {
            qDebug() << current << "is a copy-with-history, auto-recursing";
            
            if ( recurse(key, change, path_from, matchRules, rev_from, changes, revpool) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            
            isHandled = true;
        } 
        else if (is_dir && change->change_kind == svn_fs_path_change_delete) 
        {
            qDebug() << current << "deleted, auto-recursing";
            
            if ( recurse(key, change, path_from, matchRules, rev_from, changes, revpool) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            isHandled = true;
        }
    }
    
    if ( isHandled ) 
    {
        return EXIT_SUCCESS;
    }
    
    if (SvnHelper::wasDir(fs, revnum - 1, key, revpool)) 
    {
        qDebug() << current << "was a directory; ignoring";
    } 
    else if (change->change_kind == svn_fs_path_change_delete) 
    {
        qDebug() << current << "is being deleted but I don't know anything about it; ignoring";
    } 
    else 
    {
        qCritical() << current << "did not match any rules; cannot continue";
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

int SvnRevision::exportDispatch(const char *key, const svn_fs_path_change2_t *change, const char *path_from, svn_revnum_t rev_from, apr_hash_t *changes, const QString &current, const RuleMatch &rule, const QList<RuleMatch> &matchRules, apr_pool_t *pool)
{
    //if(ruledebug)
    //  qDebug() << "rev" << revnum << qPrintable(current) << "matched rule:" << rule.lineNumber << "(" << rule.rx.pattern() << ")";
    switch (rule.action) 
    {
        case Ignore:
        {
            //if(ruledebug)
            //    qDebug() << "  " << "ignoring.";
            return EXIT_SUCCESS;
        }

        case Recurse:
        {
            if(ruledebug)
            {
                qDebug() << "rev" << revnum << qPrintable(current) << "matched rule:" << rule.info() << "  " << "recursing.";
            }
            
            return recurse(key, change, path_from, matchRules, rev_from, changes, pool);
        }

        case Export:
        {
            if(ruledebug)
            {
                qDebug() << "rev" << revnum << qPrintable(current) << "matched rule:" << rule.info() << "  " << "exporting.";
            }
        
            if (exportInternal(key, change, path_from, rev_from, current, rule, matchRules) == EXIT_SUCCESS)
            {
                return EXIT_SUCCESS;
            }
            
            if (change->change_kind != svn_fs_path_change_delete) 
            {
                if(ruledebug)
                {
                    qDebug() << "rev" << revnum << qPrintable(current) << "matched rule:" << rule.info() << "  " << "Unable to export non path removal.";
                }
                
                return EXIT_FAILURE;
            }
        
            // we know that the default action inside recurse is to recurse further or to ignore,
            // either of which is reasonably safe for deletion
            qWarning() << "WARN: deleting unknown path" << current << "; auto-recursing";
            
            return recurse(key, change, path_from, matchRules, rev_from, changes, pool);
        }
    }

    // never reached
    return EXIT_FAILURE;
}

int SvnRevision::exportInternal(const char *key, const svn_fs_path_change2_t *change, const char *path_from, svn_revnum_t rev_from, const QString &current, const RuleMatch &rule, const QList<RuleMatch> &matchRules)
{
    needCommit = true;
    QString svnprefix, repository, effectiveRepository, branch, path;
    splitPathName(rule, current, &svnprefix, &repository, &effectiveRepository, &branch, &path);

    GitRepository *repo = repositories.value(repository, 0);
    
    if (!repo) 
    {
        if (change->change_kind != svn_fs_path_change_delete)
        {
            qCritical() << "Rule" << rule << "references unknown repository" << repository;
        }
        
        return EXIT_FAILURE;
    }

    printf(".");
    fflush(stdout);
//                qDebug() << "   " << qPrintable(current) << "rev" << revnum << "->"
//                         << qPrintable(repository) << qPrintable(branch) << qPrintable(path);

    if (change->change_kind == svn_fs_path_change_delete && current == svnprefix && path.isEmpty() && !repo->hasPrefix()) 
    {
        if(ruledebug)
        {
            qDebug() << "repository" << repository << "branch" << branch << "deleted";
        }
        
        return repo->deleteBranch(branch, revnum);
    }

    QString previous;
    QString prevsvnprefix, prevrepository, preveffectiverepository, prevbranch, prevpath;

    if (path_from != NULL) 
    {
        previous = QString::fromUtf8(path_from);
        
        if (SvnHelper::wasDir(fs, rev_from, path_from, pool.data())) 
        {
            previous += '/';
        }
        
        QList<RuleMatch>::ConstIterator prevmatch = SvnHelper::findMatchRule(matchRules, rev_from, previous, NoIgnoreRule);
        
        if (prevmatch != matchRules.constEnd()) 
        {
            splitPathName(*prevmatch, previous, &prevsvnprefix, &prevrepository, &preveffectiverepository, &prevbranch, &prevpath);

        } 
        else 
        {
            qWarning() << "WARN: SVN reports a \"copy from\" @" << revnum << "from" << path_from << "@" << rev_from << "but no matching rules found! Ignoring copy, treating as a modification";
            
            path_from = NULL;
        }
    }

    // current == svnprefix => we're dealing with the contents of the whole branch here
    if (path_from != NULL && current == svnprefix && path.isEmpty()) 
    {
        if (previous != prevsvnprefix) 
        {
            // source is not the whole of its branch
            qDebug() << qPrintable(current) << "is a partial branch of repository" << qPrintable(prevrepository) << "branch" << qPrintable(prevbranch) << "subdir" << qPrintable(prevpath);
        } 
        else if (preveffectiverepository != effectiveRepository) 
        {
            qWarning() << "WARN:" << qPrintable(current) << "rev" << revnum << "is a cross-repository copy (from repository" << qPrintable(prevrepository) << "branch" << qPrintable(prevbranch) << "path" << qPrintable(prevpath) << "rev" << rev_from << ")";
        } 
        else if (path != prevpath) 
        {
            qDebug() << qPrintable(current) << "is a branch copy which renames base directory of all contents" << qPrintable(prevpath) << "to" << qPrintable(path);
            // FIXME: Handle with fast-import 'file rename' facility
            //        ??? Might need special handling when path == / or prevpath == /
        } 
        else 
        {
            if (prevbranch == branch) 
            {
                // same branch and same repository
                qDebug() << qPrintable(current) << "rev" << revnum << "is reseating branch" << qPrintable(branch) << "to an earlier revision" << qPrintable(previous) << "rev" << rev_from;
            } 
            else 
            {
                // same repository but not same branch
                // this means this is a plain branch
                qDebug() << qPrintable(repository) << ": branch" << qPrintable(branch) << "is branching from" << qPrintable(prevbranch);
            }

            if (repo->createBranch(branch, revnum, prevbranch, rev_from) == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if(CommandLineParser::instance()->contains("svn-branches")) 
            {
                GitRepositoryTransaction *txn = transactions.value(repository + branch, 0);
                
                if (!txn) 
                {
                    txn = repo->newTransaction(branch, svnprefix, revnum);
                    
                    if (!txn)
                    {
                        return EXIT_FAILURE;
                    }

                    transactions.insert(repository + branch, txn);
                }
                
                if(ruledebug)
                {
                    qDebug() << "Create a true SVN copy of branch (" << key << "->" << branch << path << ")";
                }
                
                txn->deleteFile(path);
                SvnHelper::recursiveDumpDir(txn, fs_root, key, path, pool);
            }
            
            if (rule.annotate) 
            {
                // create an annotated tag
                fetchRevProps();
                repo->createAnnotatedTag(branch, svnprefix, revnum, authorident,
                                         epoch, log);
            }
            
            return EXIT_SUCCESS;
        }
    }
    
    GitRepositoryTransaction* txn = transactions.value(repository + branch, 0);
    
    if (!txn) 
    {
        txn = repo->newTransaction(branch, svnprefix, revnum);
        
        if (!txn)
        {
            return EXIT_FAILURE;
        }

        transactions.insert(repository + branch, txn);
    }

    //
    // If this path was copied from elsewhere, use it to infer _some_
    // merge points.  This heuristic is fairly useful for tracking
    // changes across directory re-organizations and wholesale branch
    // imports.
    //
    if (path_from != NULL && preveffectiverepository == effectiveRepository && prevbranch != branch) 
    {
        if(ruledebug)
        {
            qDebug() << "copy from branch" << prevbranch << "to branch" << branch << "@rev" << rev_from;
        }
        
        txn->noteCopyFromBranch (prevbranch, rev_from);
    }

    if (change->change_kind == svn_fs_path_change_replace && path_from == NULL) 
    {
        if(ruledebug)
        {
            qDebug() << "replaced with empty path (" << branch << path << ")";
        }
        
        txn->deleteFile(path);
    }
    
    if (change->change_kind == svn_fs_path_change_delete) 
    {
        if(ruledebug)
        {
            qDebug() << "delete (" << branch << path << ")";
        }
        
        txn->deleteFile(path);
    } 
    else if (!current.endsWith('/')) 
    {
        if(ruledebug)
        {
            qDebug() << "add/change file (" << key << "->" << branch << path << ")";
        }
        
        SvnHelper::dumpBlob(txn, fs_root, key, path, pool);
    } 
    else 
    {
        if(ruledebug)
        {
            qDebug() << "add/change dir (" << key << "->" << branch << path << ")";
        }

        // Check unknown svn-properties
        if (((path_from == NULL && change->prop_mod==1) || (path_from != NULL && change->change_kind == svn_fs_path_change_add)) && CommandLineParser::instance()->contains("propcheck")) 
        {
            if (fetchUnknownProps(pool, key, fs_root) != EXIT_SUCCESS) 
            {
                qWarning() << "Error checking svn-properties (" << key << ")";
            }
        }

        int ignoreSet = false;

        // Add GitIgnore with svn:ignore
        if (((path_from == NULL && change->prop_mod == 1) || (path_from != NULL && change->change_kind == svn_fs_path_change_add)) && CommandLineParser::instance()->contains("svn-ignore")) 
        {
            QString svnignore;
            
            // TODO: Check if svn:ignore or other property was changed, but always set on copy/rename (path_from != NULL)
            if (fetchIgnoreProps(&svnignore, pool, key, fs_root) != EXIT_SUCCESS) 
            {
                qWarning() << "Error fetching svn-properties (" << key << ")";
            } 
            else if (!svnignore.isNull()) 
            {
                addGitIgnore(pool, key, path, fs_root, txn, svnignore.toStdString().c_str());
                ignoreSet = true;
            }
        }

        // Add GitIgnore for empty directories (if GitIgnore was not set previously)
        if (CommandLineParser::instance()->contains("empty-dirs") && ignoreSet == false) 
        {
            if (addGitIgnore(pool, key, path, fs_root, txn) == EXIT_SUCCESS) 
            {
                return EXIT_SUCCESS;
            } 
            else 
            {
                ignoreSet = true;
            }
        }

        if (ignoreSet == false) 
        {
            txn->deleteFile(path);
        }
        
        SvnHelper::recursiveDumpDir(txn, fs_root, key, path, pool);
    }

    return EXIT_SUCCESS;
}

int SvnRevision::recurse(const char *path, const svn_fs_path_change2_t *change, const char *path_from, const QList<RuleMatch> &matchRules, svn_revnum_t rev_from, apr_hash_t *changes, apr_pool_t *pool)
{
    svn_fs_root_t *fs_root = this->fs_root;
    
    if (change->change_kind == svn_fs_path_change_delete)
    {
        SVN_INT_ERR(svn_fs_revision_root(&fs_root, fs, revnum - 1, pool));
    }

    // get the dir listing
    svn_node_kind_t kind;
    SVN_INT_ERR(svn_fs_check_path(&kind, fs_root, path, pool));
    
    if(kind == svn_node_none) 
    {
        qWarning() << "WARN: Trying to recurse using a nonexistant path" << path << ", ignoring";
        return EXIT_SUCCESS;
    } 
    else if(kind != svn_node_dir) 
    {
        qWarning() << "WARN: Trying to recurse using a non-directory path" << path << ", ignoring";
        return EXIT_SUCCESS;
    }

    apr_hash_t *entries;
    SVN_INT_ERR(svn_fs_dir_entries(&entries, fs_root, path, pool));
    AprAutoPool dirpool(pool);

    // While we get a hash, put it in a map for sorted lookup, so we can
    // repeat the conversions and get the same git commit hashes.
    QMap<QByteArray, svn_node_kind_t> map;
    
    for (apr_hash_index_t *i = apr_hash_first(pool, entries); i; i = apr_hash_next(i)) 
    {
        dirpool.clear();
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
        QByteArray entry = path + QByteArray("/") + i.key();
        QByteArray entryFrom;
    
        if (path_from)
        {
            entryFrom = path_from + QByteArray("/") + i.key();
        }

        // check if this entry is in the changelist for this revision already
        svn_fs_path_change2_t *otherchange = (svn_fs_path_change2_t*)apr_hash_get(changes, entry.constData(), APR_HASH_KEY_STRING);
        
        if (otherchange && otherchange->change_kind == svn_fs_path_change_add) 
        {
            qDebug() << entry << "rev" << revnum << "is in the change-list, deferring to that one";
            continue;
        }

        QString current = QString::fromUtf8(entry);
        
        if (i.value() == svn_node_dir)
        {
            current += '/';
        }

        // find the first rule that matches this pathname
        QList<RuleMatch>::ConstIterator match = SvnHelper::findMatchRule(matchRules, revnum, current);
        if (match != matchRules.constEnd()) 
        {
            if (exportDispatch(entry, change, entryFrom.isNull() ? 0 : entryFrom.constData(), rev_from, changes, current, *match, matchRules, dirpool) == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }
        } 
        else 
        {
            if (i.value() == svn_node_dir) 
            {
                qDebug() << current << "rev" << revnum << "did not match any rules; auto-recursing";
                
                if (recurse(entry, change, entryFrom.isNull() ? 0 : entryFrom.constData(), matchRules, rev_from, changes, dirpool) == EXIT_FAILURE)
                {
                    return EXIT_FAILURE;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

int SvnRevision::addGitIgnore(apr_pool_t* pool, const char* key, QString path, svn_fs_root_t* fs_root, GitRepositoryTransaction* txn, const char* content)
{
    // Check for number of subfiles if no content
    if (!content) 
    {
        apr_hash_t *entries;
        SVN_INT_ERR(svn_fs_dir_entries(&entries, fs_root, key, pool));
        
        // Return if any subfiles
        if (apr_hash_count(entries)!=0) 
        {
            return EXIT_FAILURE;
        }
    }

    // Add gitignore-File
    QString gitIgnorePath = path + ".gitignore";
    if (content) 
    {
        QIODevice *io = txn->addFile(gitIgnorePath, 33188, strlen(content));
        io->write(content);
        io->putChar('\n');
    } 
    else 
    {
        QIODevice *io = txn->addFile(gitIgnorePath, 33188, 0);
        io->putChar('\n');
    }

    return EXIT_SUCCESS;
}

int SvnRevision::fetchIgnoreProps(QString* ignore, apr_pool_t* pool, const char* key, svn_fs_root_t* fs_root)
{
    // Get svn:ignore
    svn_string_t* prop = NULL;
    SVN_INT_ERR(svn_fs_node_prop(&prop, fs_root, key, "svn:ignore", pool));
    
    if (prop) 
    {
        *ignore = QString(prop->data);
    } 
    else 
    {
        *ignore = QString();
    }

    return EXIT_SUCCESS;
}

int SvnRevision::fetchUnknownProps(apr_pool_t* pool, const char* key, svn_fs_root_t* fs_root)
{
    // Check all properties
    apr_hash_t* table;
    SVN_INT_ERR(svn_fs_node_proplist(&table, fs_root, key, pool));
    apr_hash_index_t* hi;
    void* propVal;
    const void* propKey;
    
    for (hi = apr_hash_first(pool, table); hi; hi = apr_hash_next(hi)) 
    {
        apr_hash_this(hi, &propKey, NULL, &propVal);
        
        if (strcmp((char*)propKey, "svn:ignore")!=0) 
        {
            qWarning() << "WARN: Unknown svn-property" << (char*)propKey << "set to" << ((svn_string_t*)propVal)->data << "for" << key;
        }
    }

    return EXIT_SUCCESS;
}
