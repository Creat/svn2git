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

#ifndef GIT_PROCESS_CACHE_H
#define GIT_PROCESS_CACHE_H

#include "QLinkedList"

#include "FastImportGitRepository.h"

static const int maxSimultaneousProcesses = 100;

class GitProcessCache: QLinkedList<FastImportGitRepository*>
{
    
public:
    
    void touch(FastImportGitRepository* repo);
    
    inline void remove(FastImportGitRepository* repo)
    {
        #if QT_VERSION >= 0x040400
            removeOne(repo);
        #else
            removeAll(repo);
        #endif
    }
};

static GitProcessCache processCache;

#endif
