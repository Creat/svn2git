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

#ifndef GIT_REPOSITORY_TRANSACTION_H
#define GIT_REPOSITORY_TRANSACTION_H

#include <QByteArray>

class QString;
class QIODevice;

class GitRepositoryTransaction
{
    Q_DISABLE_COPY(GitRepositoryTransaction)
        
public:
        
    virtual ~GitRepositoryTransaction();
    virtual void commit() = 0;

    virtual void setAuthor(const QByteArray& author) = 0;
    virtual void setDateTime(uint dt) = 0;
    virtual void setLog(const QByteArray& log) = 0;

    virtual void noteCopyFromBranch (const QString &prevbranch, int revFrom) = 0;

    virtual void deleteFile(const QString& path) = 0;
    virtual QIODevice* addFile(const QString& path, int mode, qint64 length) = 0;

    virtual void commitNote(const QByteArray& noteText, bool append, const QByteArray& commit = QByteArray()) = 0;
        
protected:
       
    GitRepositoryTransaction();

};

#endif
