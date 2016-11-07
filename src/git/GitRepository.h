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

#ifndef GIT_REPOSITORY_H
#define GIT_REPOSITORY_H

#include <QHash>

#include "rules/RuleRepository.h"

class QString;
class QByteArray;
class RuleRepository;
class GitRepositoryTransaction;

class GitRepository
{
    
public:
    
    static GitRepository *createRepository(const RuleRepository& rule, const QHash<QString, GitRepository*> &repositories);
    
    virtual int setupIncremental(int &cutoff) = 0;
    virtual void restoreLog() = 0;
    virtual ~GitRepository() {};

    virtual void reloadBranches() = 0;
    virtual int createBranch(const QString& branch, int revnum, const QString& branchFrom, int revFrom) = 0;
    virtual int deleteBranch(const QString& branch, int revnum) = 0;
    virtual GitRepositoryTransaction *newTransaction(const QString& branch, const QString& svnprefix, int revnum) = 0;

    virtual void createAnnotatedTag(const QString& name, const QString& svnprefix, int revnum, const QByteArray& author, uint dt, const QByteArray &log) = 0;
    
    virtual void close() = 0;
    virtual void finalizeTags() = 0;
    virtual void commit() = 0;

    static const QByteArray formatMetadataMessage(const QByteArray& svnprefix, int revnum, const QByteArray& tag = QByteArray());

    virtual bool branchExists(const QString& branch) const = 0;
    virtual const QByteArray branchNote(const QString& branch) const = 0;
    virtual void setBranchNote(const QString& branch, const QByteArray& noteText) = 0;

    virtual bool hasPrefix() const = 0;

    virtual const QString& getName() const = 0;
    virtual GitRepository *getEffectiveRepository() = 0;
};


#endif
