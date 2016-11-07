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

#ifndef APR_AUTO_POOL_H
#define APR_AUTO_POOL_H

#include <svn_pools.h>

class AprAutoPool
{
   
public:
    
    inline AprAutoPool(apr_pool_t* parent = 0)
    {
        pool = svn_pool_create(parent);
    }
    
    inline ~AprAutoPool()
    {
        svn_pool_destroy(pool);    
    }
    
    inline void clear()
    {
        svn_pool_clear(pool);
    }
    
    inline apr_pool_t* data() const
    {
        return pool; 
    }
    
    inline operator apr_pool_t* () const
    {
        return pool;
    }
    
private:
    
    apr_pool_t *pool;
    AprAutoPool(const AprAutoPool&);
    AprAutoPool &operator=(const AprAutoPool&);
};

#endif
