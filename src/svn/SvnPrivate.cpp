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

#include "SvnPrivate.h"

#include <QFile>
#include <QDebug>

#include <svn_fs.h>
#include <svn_pools.h>
#include <svn_repos.h>

#include "SvnRevision.h"

SvnPrivate::SvnPrivate(const QString& pathToRepository) :
    global_pool(NULL), 
    scratch_pool(NULL)
{
    if( openRepository(pathToRepository) != EXIT_SUCCESS) 
    {
        qCritical() << "Failed to open repository";
        exit(1);
    }

    // get the youngest revision
    svn_fs_youngest_rev(&youngest_rev, fs, global_pool);
}

SvnPrivate::~SvnPrivate()
{
    
}

int SvnPrivate::youngestRevision()
{
    return youngest_rev;
}

int SvnPrivate::openRepository(const QString& pathToRepository)
{
    svn_repos_t* repos;
    QString path = pathToRepository;
    
    while (path.endsWith('/')) // no trailing slash allowed
    {
        path = path.mid(0, path.length() - 1);
    }
    
    SVN_INT_ERR(svn_repos_open3(&repos, QFile::encodeName(path), NULL, global_pool, scratch_pool));
    fs = svn_repos_fs(repos);

    return EXIT_SUCCESS;
}

int SvnPrivate::exportRevision(int revnum)
{
    SvnRevision rev(revnum, fs, global_pool);
    rev.allMatchRules = allMatchRules;
    rev.repositories = repositories;
    rev.identities = identities;
    rev.userdomain = userdomain;

    // open this revision:
    printf("Exporting revision %d ", revnum);
    fflush(stdout);

    if (rev.open() == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    if (rev.prepareTransactions() == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    if (!rev.needCommit) 
    {
        printf(" nothing to do\n");
        return EXIT_SUCCESS;    // no changes?
    }

    if (rev.commit() == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    printf(" done\n");
    return EXIT_SUCCESS;
}
