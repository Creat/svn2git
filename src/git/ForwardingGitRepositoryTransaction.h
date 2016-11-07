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

#ifndef FORWARDING_GIT_REPOSITORY_TRANSACTION_H
#define FORWARDING_GIT_REPOSITORY_TRANSACTION_H

#include "GitRepositoryTransaction.h"

#include <QString>
#include <QByteArray>

class ForwardingGitRepositoryTransaction : public GitRepositoryTransaction
{
    Q_DISABLE_COPY(ForwardingGitRepositoryTransaction)

public:
        
    ForwardingGitRepositoryTransaction(GitRepositoryTransaction* t, const QString& p);
    ~ForwardingGitRepositoryTransaction();
    
    void commit();
    void setAuthor(const QByteArray& author);
    void setDateTime(uint dt);
    void setLog(const QByteArray& log);
    void noteCopyFromBranch (const QString& prevbranch, int revFrom);
    void deleteFile(const QString& path);
    QIODevice* addFile(const QString& path, int mode, qint64 length);
    void commitNote(const QByteArray& noteText, bool append, const QByteArray& commit);

private:
    
    GitRepositoryTransaction* txn;
    QString prefix;
};

#endif
