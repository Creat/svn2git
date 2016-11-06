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

#ifndef SVN_HELPER_H
#define SVN_HELPER_H

#include <QList>
#include <QString>

#include "rules/RuleMatch.h"

class QIODevice;
class GitRepositoryTransaction;

struct svn_fs_t;
struct svn_fs_root_t;
struct svn_stream_t;
struct svn_error_t;
struct apr_pool_t;

typedef size_t apr_size_t;

enum RuleType { AnyRule = 0, NoIgnoreRule = 0x01, NoRecurseRule = 0x02 };

class SvnHelper
{
    
public:
    
    static QList<RuleMatch>::ConstIterator findMatchRule(const QList<RuleMatch>& matchRules, int revnum, const QString& current, int ruleMask = AnyRule);
    static int pathMode(svn_fs_root_t* fs_root, const char *pathname, apr_pool_t* pool);
    svn_error_t* deviceWrite(void* baton, const char* data, apr_size_t* len); 
    static svn_stream_t* streamForDevice(QIODevice* device, apr_pool_t* pool);
    static int dumpBlob(GitRepositoryTransaction* txn, svn_fs_root_t* fs_root, const char* pathname, const QString &finalPathName, apr_pool_t* pool);
    static int recursiveDumpDir(GitRepositoryTransaction* txn, svn_fs_root_t* fs_root, const QByteArray &pathname, const QString &finalPathName, apr_pool_t* pool);
    static bool wasDir(svn_fs_t* fs, int revnum, const char *pathname, apr_pool_t* pool);
    static time_t getEpoch(const char* svn_date);
    static svn_error_t* QIODevice_write(void* baton, const char* data, apr_size_t* len);
};

#endif
