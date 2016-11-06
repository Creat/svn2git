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

#include "Svn.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <apr_lib.h>
#include <apr_getopt.h>
#include <apr_general.h>

#include "SvnPrivate.h"

void Svn::initialize()
{
    // initialize APR or exit
    if (apr_initialize() != APR_SUCCESS) 
    {
        fprintf(stderr, "You lose at apr_initialize().\n");
        exit(1);
    }

    // static destructor
    static struct Destructor { ~Destructor() { apr_terminate(); } } destructor;
}

Svn::Svn(const QString &pathToRepository) : privateClass(new SvnPrivate(pathToRepository))
{
}

Svn::~Svn()
{
    delete privateClass;
}

void Svn::setMatchRules(const QList<QList<RuleMatch> > &allMatchRules)
{
    privateClass->allMatchRules = allMatchRules;
}

void Svn::setRepositories(const QHash<QString, GitRepository*> &repositories)
{
    privateClass->repositories = repositories;
}

void Svn::setIdentityMap(const QHash<QByteArray, QByteArray> &identityMap)
{
    privateClass->identities = identityMap;
}

void Svn::setIdentityDomain(const QString &identityDomain)
{
    privateClass->userdomain = identityDomain;
}

int Svn::youngestRevision()
{
    return privateClass->youngestRevision();
}

bool Svn::exportRevision(int revnum)
{
    return privateClass->exportRevision(revnum) == EXIT_SUCCESS;
}

