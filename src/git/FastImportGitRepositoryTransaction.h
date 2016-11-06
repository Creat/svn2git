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

#ifndef FAST_IMPORT_GIT_REPOSITORY_TRANSACTION_H
#define FAST_IMPORT_GIT_REPOSITORY_TRANSACTION_H

#include <QVector>
#include <QByteArray>
#include <QStringList>

#include "GitRepositoryTransaction.h"

#include "FastImportGitRepository.h"

class FastImportGitRepositoryTransaction : public GitRepositoryTransaction
{

public:
    
    Q_DISABLE_COPY(FastImportGitRepositoryTransaction)

    ~FastImportGitRepositoryTransaction();
    
    void commit();

    void setAuthor(const QByteArray &author);
    void setDateTime(uint dt);
    void setLog(const QByteArray &log);

    void noteCopyFromBranch (const QString &prevbranch, int revFrom);

    void deleteFile(const QString &path);
    QIODevice *addFile(const QString &path, int mode, qint64 length);

    void commitNote(const QByteArray &noteText, bool append, const QByteArray &commit);
    
private:
    
    FastImportGitRepository *repository;
    
    QByteArray branch;
    QByteArray svnprefix;
    QByteArray author;
    QByteArray log;
    uint datetime;
    int revnum;

    QVector<int> merges;

    QStringList deletedFiles;
    QByteArray modifiedFiles;
    
    inline FastImportGitRepositoryTransaction() {}
    
    friend class FastImportGitRepository;
};

#endif
