#include "FastImportGitRepositoryTransaction.h"

#include <QDebug>

#include "commandline/CommandLineParser.h"

FastImportGitRepositoryTransaction::~FastImportGitRepositoryTransaction()
{
    repository->forgetTransaction(this);
}

void FastImportGitRepositoryTransaction::setAuthor(const QByteArray &a)
{
    author = a;
}

void FastImportGitRepositoryTransaction::setDateTime(uint dt)
{
    datetime = dt;
}

void FastImportGitRepositoryTransaction::setLog(const QByteArray &l)
{
    log = l;
}

void FastImportGitRepositoryTransaction::noteCopyFromBranch(const QString &branchFrom, int branchRevNum)
{
    if(branch == branchFrom) 
    {
        qWarning() << "WARN: Cannot merge inside a branch";
        
        return;
    }
    
    static QByteArray dummy;
    long long mark = repository->markFrom(branchFrom, branchRevNum, dummy);
    Q_ASSERT(dummy.isEmpty());

    if (mark == -1) 
    {
        qWarning() << "WARN:" << branch << "is copying from branch" << branchFrom << "but the latter doesn't exist.  Continuing, assuming the files exist.";
    } 
    else if (mark == 0) 
    {
        qWarning() << "WARN: Unknown revision r" << QByteArray::number(branchRevNum) << ".  Continuing, assuming the files exist.";
    } 
    else 
    {
        qWarning() << "WARN: repository " + repository->name + " branch " + branch + " has some files copied from " + branchFrom + "@" + QByteArray::number(branchRevNum);

        if (!merges.contains(mark)) 
        {
            merges.append(mark);
            qDebug() << "adding" << branchFrom + "@" + QByteArray::number(branchRevNum) << ":" << mark << "as a merge point";
        } 
        else 
        {
            qDebug() << "merge point already recorded";
        }
    }
}

void FastImportGitRepositoryTransaction::deleteFile(const QString &path)
{
    QString pathNoSlash = repository->prefix + path;
    
    if(pathNoSlash.endsWith('/'))
    {
        pathNoSlash.chop(1);
    }
    
    deletedFiles.append(pathNoSlash);
}

QIODevice *FastImportGitRepositoryTransaction::addFile(const QString &path, int mode, qint64 length)
{
    unsigned long long mark = repository->next_file_mark--;

    // in case the two mark allocations meet, we might as well just abort
    Q_ASSERT(mark > repository->last_commit_mark + 1);

    if (modifiedFiles.capacity() == 0)
    {
        modifiedFiles.reserve(2048);
    }
    
    modifiedFiles.append("M ");
    modifiedFiles.append(QByteArray::number(mode, 8));
    modifiedFiles.append(" :");
    modifiedFiles.append(QByteArray::number(mark));
    modifiedFiles.append(' ');
    modifiedFiles.append(repository->prefix + path.toUtf8());
    modifiedFiles.append("\n");

    if (!CommandLineParser::instance()->contains("dry-run")) 
    {
        repository->startFastImport();
        repository->fastImport.writeNoLog("blob\nmark :");
        repository->fastImport.writeNoLog(QByteArray::number(mark));
        repository->fastImport.writeNoLog("\ndata ");
        repository->fastImport.writeNoLog(QByteArray::number(length));
        repository->fastImport.writeNoLog("\n", 1);
    }

    return &repository->fastImport;
}

void FastImportGitRepositoryTransaction::commitNote(const QByteArray &noteText, bool append, const QByteArray &commit = QByteArray())
{
    QByteArray branchRef = branch;
    
    if (!branchRef.startsWith("refs/"))
    {
        branchRef.prepend("refs/heads/");
    }
    
    const QByteArray &commitRef = commit.isNull() ? branchRef : commit;
    QByteArray message = "Adding Git note for current " + commitRef + "\n";
    QByteArray text = noteText;

    if (append && commit.isNull() && repository->branchExists(branch) && !repository->branchNote(branch).isEmpty())
    {
        text = repository->branchNote(branch) + text;
        message = "Appending Git note for current " + commitRef + "\n";
    }

    QByteArray s("");
    s.append("commit refs/notes/commits\n");
    s.append("mark :" + QByteArray::number(maxMark + 1) + "\n");
    s.append("committer " + author + " " + QString::number(datetime) + " +0000" + "\n");
    s.append("data " + QString::number(message.length()) + "\n");
    s.append(message + "\n");
    s.append("N inline " + commitRef + "\n");
    s.append("data " + QString::number(text.length()) + "\n");
    s.append(text + "\n");
    repository->fastImport.write(s);

    if (commit.isNull()) 
    {
        repository->setBranchNote(QString::fromUtf8(branch), text);
    }
}

void FastImportGitRepositoryTransaction::commit()
{
    repository->startFastImport();

    // We might be tempted to use the SVN revision number as the fast-import commit mark.
    // However, a single SVN revision can modify multple branches, and thus lead to multiple
    // commits in the same repo.  So, we need to maintain a separate commit mark counter.
    unsigned long long  mark = ++repository->last_commit_mark;

    // in case the two mark allocations meet, we might as well just abort
    Q_ASSERT(mark < repository->next_file_mark - 1);

    // create the commit message
    QByteArray message = log;
    if (!message.endsWith('\n'))
    {
        message += '\n';
    }
    
    if (CommandLineParser::instance()->contains("add-metadata"))
    {
        message += "\n" + GitRepository::formatMetadataMessage(svnprefix, revnum);
    }

    // Call external message filter if provided
    message = repository->msgFilter(message);

    unsigned long long parentmark = 0;
    FastImportGitRepository::Branch &br = repository->branches[branch];
    
    if (br.created && !br.marks.isEmpty() && br.marks.last()) 
    {
        parentmark = br.marks.last();
    } 
    else 
    {
        qWarning() << "WARN: Branch" << branch << "in repository" << repository->name << "doesn't exist at revision"
                   << revnum << "-- did you resume from the wrong revision?";
        br.created = revnum;
    }
    
    br.commits.append(revnum);
    br.marks.append(mark);

    QByteArray branchRef = branch;
    
    if (!branchRef.startsWith("refs/"))
    {
        branchRef.prepend("refs/heads/");
    }

    QByteArray s("");
    s.append("commit " + branchRef + "\n");
    s.append("mark :" + QByteArray::number(mark) + "\n");
    s.append("committer " + author + " " + QString::number(datetime).toUtf8() + " +0000" + "\n");
    s.append("data " + QString::number(message.length()) + "\n");
    s.append(message + "\n");
    repository->fastImport.write(s);

    // note some of the inferred merges
    QByteArray desc = "";
    unsigned long long i = !!parentmark;	// if parentmark != 0, there's at least one parent

    if(log.contains("This commit was manufactured by cvs2svn") && merges.count() > 1) 
    {
        qSort(merges);
        repository->fastImport.write("merge :" + QByteArray::number(merges.last()) + "\n");
        merges.pop_back();
        qWarning() << "WARN: Discarding all but the highest merge point as a workaround for cvs2svn created branch/tag Discarded marks:" << merges;
    } 
    else 
    {
        foreach (const unsigned long long merge, merges) 
        {
            if (merge == parentmark) 
            {
                qDebug() << "Skipping marking" << merge << "as a merge point as it matches the parent";
                continue;
            }

            if (++i > 16) 
            {
                // FIXME: options:
                //   (1) ignore the 16 parent limit
                //   (2) don't emit more than 16 parents
                //   (3) create another commit on branch to soak up additional parents
                // we've chosen option (2) for now, since only artificial commits
                // created by cvs2svn seem to have this issue
                qWarning() << "WARN: too many merge parents";
                break;
            }

            QByteArray m = " :" + QByteArray::number(merge);
            desc += m;
            repository->fastImport.write("merge" + m + "\n");
        }
    }
    
    // write the file deletions
    if (deletedFiles.contains(""))
    {
        repository->fastImport.write("deleteall\n");
    }
    else
    {
        foreach (QString df, deletedFiles)
        {
            repository->fastImport.write("D " + df.toUtf8() + "\n");
        }
    }

    // write the file modifications
    repository->fastImport.write(modifiedFiles);

    repository->fastImport.write("\nprogress SVN r" + QByteArray::number(revnum) + " branch " + branch + " = :" + QByteArray::number(mark) + (desc.isEmpty() ? "" : " # merge from") + desc + "\n\n");
    
    printf(" %d modifications from SVN %s to %s/%s", deletedFiles.count() + modifiedFiles.count('\n'), svnprefix.data(), qPrintable(repository->name), branch.data());

    // Commit metadata note if requested
    if (CommandLineParser::instance()->contains("add-metadata-notes"))
    {
        commitNote(GitRepository::formatMetadataMessage(svnprefix, revnum), false);
    }

    while (repository->fastImport.bytesToWrite())
    {
        if (!repository->fastImport.waitForBytesWritten(-1))
        {
            qFatal("Failed to write to process: %s for repository %s", qPrintable(repository->fastImport.errorString()), qPrintable(repository->name));
        }
    }
}
