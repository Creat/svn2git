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

#ifndef FAST_IMPORT_GIT_REPOSITORY_H
#define FAST_IMPORT_GIT_REPOSITORY_H

#include <QVector>
#include <QProcess>

#include "GitRepository.h"
#include "logging/LoggingQProcess.h"

class GitRepositoryTransaction;
class FastImportGitRepositoryTransaction;

static const unsigned long long maxMark = ULONG_MAX;

class FastImportGitRepository : public GitRepository
{
    
public:
    
    struct Branch
    {
        int created;
        QVector<int> commits;
        QVector<int> marks;
        QByteArray note;
    };
    
    FastImportGitRepository(const RuleRepository &rule);
    ~FastImportGitRepository();
    
    int setupIncremental(int &cutoff);
    void restoreLog();
    void reloadBranches();
    int createBranch(const QString &branch, int revnum, const QString &branchFrom, int revFrom);
    int deleteBranch(const QString &branch, int revnum);
    GitRepositoryTransaction* newTransaction(const QString &branch, const QString &svnprefix, int revnum);
    void createAnnotatedTag(const QString &name, const QString &svnprefix, int revnum, const QByteArray &author, uint dt, const QByteArray &log);
    void finalizeTags();
    void commit();
    bool branchExists(const QString& branch) const;
    const QByteArray branchNote(const QString& branch) const;
    void setBranchNote(const QString& branch, const QByteArray& noteText);
    bool hasPrefix() const;
    const QString& getName() const;
    GitRepository *getEffectiveRepository();
    
private:
    
    struct AnnotatedTag
    {
        QString supportingRef;
        QByteArray svnprefix;
        QByteArray author;
        QByteArray log;
        uint dt;
        int revnum;
    };
    
    static QString marksFileName(QString name);
    static QString logFileName(QString name);
    static unsigned long long lastValidMark(QString name);

    void startFastImport();
    void closeFastImport();

    // called when a transaction is deleted
    void forgetTransaction(FastImportGitRepositoryTransaction *t);

    int resetBranch(const QString &branch, int revnum, unsigned long long mark, const QByteArray &resetTo, const QByteArray &comment);
    long long markFrom(const QString &branchFrom, int branchRevNum, QByteArray &desc);


    QHash<QString, Branch> branches;
    QHash<QString, AnnotatedTag> annotatedTags;
    QString name;
    QString prefix;
    LoggingQProcess fastImport;
    int commitCount;
    int outstandingTransactions;
    QByteArray deletedBranches;
    QByteArray resetBranches;

    /* Optional filter to fix up log messages */
    QProcess filterMsg;
    QByteArray msgFilter(QByteArray);

    /* starts at 0, and counts up.  */
    unsigned long long last_commit_mark;

    /* starts at maxMark and counts down. Reset after each SVN revision */
    unsigned long long next_file_mark;

    bool processHasStarted;

    friend class GitProcessCache;
    friend class FastImportGitRepositoryTransaction;
    Q_DISABLE_COPY(FastImportGitRepository)
};

#endif
