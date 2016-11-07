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

#ifndef SVN_H
#define SVN_H

#include <QHash>
#include <QList>
#include <QString>
#include <QByteArray>

#include "rules/RuleMatch.h"

class SvnPrivate;
class GitRepository;

class Svn
{
     
public:
    
    static void initialize();

    Svn(const QString &pathToRepository);
    ~Svn();

    void setMatchRules(const QList<QList<RuleMatch> >& matchRules);
    void setRepositories(const QHash<QString, GitRepository*>& repositories);
    void setIdentityMap(const QHash<QByteArray, QByteArray>& identityMap);
    void setIdentityDomain(const QString& identityDomain);

    int youngestRevision();
    bool exportRevision(int revnum);

private:
    
    SvnPrivate* const privateClass;
};

#endif
