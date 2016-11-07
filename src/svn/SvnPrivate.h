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

#ifndef SVN_PRIVATE_H
#define SVN_PRIVATE_H

#include <QList>
#include <QHash>
#include <QString>
#include <QByteArray>

#include <svn_types.h>

#include "AprAutoPool.h"

#include "rules/RuleMatch.h"

class GitRepository;

struct svn_fs_t;

class SvnPrivate
{
    
public:
    
    SvnPrivate(const QString& pathToRepository);
    ~SvnPrivate();
    
    int youngestRevision();
    int exportRevision(int revnum);
    int openRepository(const QString& pathToRepository);
    
    QList<QList<RuleMatch> > allMatchRules;
    QHash<QString, GitRepository*> repositories;
    QHash<QByteArray, QByteArray> identities;
    QString userdomain;

private: 
    
    AprAutoPool global_pool;
    AprAutoPool scratch_pool;
    
    svn_fs_t* fs;
    svn_revnum_t youngest_rev;
};

#endif
