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

#ifndef FORWARDING_GIT_REPOSITORY_H
#define FORWARDING_GIT_REPOSITORY_H

#include "GitRepository.h"
#include "ForwardingGitRepositoryTransaction.h"

class ForwardingGitRepository : public GitRepository
{
    
public:
    
    ForwardingGitRepository(const QString &n, GitRepository *r, const QString &p);
    ~ForwardingGitRepository();

    int setupIncremental(int &);
    void restoreLog();
    void reloadBranches();
    int createBranch(const QString &branch, int revnum, const QString &branchFrom, int revFrom);
    int deleteBranch(const QString &branch, int revnum);
    GitRepositoryTransaction *newTransaction(const QString &branch, const QString &svnprefix, int revnum);
    void createAnnotatedTag(const QString &name, const QString &svnprefix, int revnum, const QByteArray &author, uint dt, const QByteArray &log);
    
	void close();
    void finalizeTags();
    void commit();
    
    bool branchExists(const QString& branch) const;
    const QByteArray branchNote(const QString& branch) const;
    void setBranchNote(const QString& branch, const QByteArray& noteText);
    bool hasPrefix() const;
    const QString& getName() const;
    GitRepository *getEffectiveRepository();
    
private:
    
    QString name;
    GitRepository *repo;
    QString prefix;
};

#endif
