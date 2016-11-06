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

#ifndef SVN_REVISION_H
#define SVN_REVISION_H

#include <QHash>

#include <svn_fs.h>

#include "AprAutoPool.h"
#include "rules/RuleMatch.h"

class GitRepository;
class GitRepositoryTransaction;

class SvnRevision
{
    
public:
    
    SvnRevision(int revision, svn_fs_t* f, apr_pool_t* parent_pool);

    int open();
    int prepareTransactions();
    int fetchRevProps();
    int commit();

    int exportEntry(const char* path, const svn_fs_path_change2_t* change, apr_hash_t* changes);
    int exportDispatch(const char* path, const svn_fs_path_change2_t* change, const char* path_from, svn_revnum_t rev_from, apr_hash_t* changes, const QString& current, const RuleMatch& rule, const QList<RuleMatch>& matchRules, apr_pool_t* pool);
    int exportInternal(const char* path, const svn_fs_path_change2_t* change, const char* path_from, svn_revnum_t rev_from, const QString& current, const RuleMatch& rule, const QList<RuleMatch>& matchRules);
    int recurse(const char* path, const svn_fs_path_change2_t* change, const char* path_from, const QList<RuleMatch>& matchRules, svn_revnum_t rev_from, apr_hash_t* changes, apr_pool_t* pool);
    int addGitIgnore(apr_pool_t* pool, const char* key, QString path, svn_fs_root_t* fs_root, GitRepositoryTransaction* txn, const char* content = NULL);
    int fetchIgnoreProps(QString* ignore, apr_pool_t* pool, const char* key, svn_fs_root_t* fs_root);
    int fetchUnknownProps(apr_pool_t* pool, const char* key, svn_fs_root_t* fs_root);
    
    AprAutoPool pool;
    QHash<QString, GitRepositoryTransaction*> transactions;
    QList<QList<RuleMatch> > allMatchRules;
    QHash<QString, GitRepository*> repositories;
    QHash<QByteArray, QByteArray> identities;
    QString userdomain;

    svn_fs_t* fs;
    svn_fs_root_t* fs_root;
    int revnum;

    // must call fetchRevProps first:
    QByteArray authorident;
    QByteArray log;
    uint epoch;
    bool ruledebug;
    bool propsFetched;
    bool needCommit;
    
private:
    
    void splitPathName(const RuleMatch& rule, const QString& pathName, QString* svnprefix_p, QString* repository_p, QString* effectiveRepository_p, QString* branch_p, QString* path_p);
};

#endif
