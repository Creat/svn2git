#include "FastImportGitRepository.h"

#include <QDir>
#include <QDebug>

#include "GitProcessCache.h"
#include "rules/RuleRepository.h"
#include "commandline/CommandLineParser.h"
#include "FastImportGitRepositoryTransaction.h"

FastImportGitRepository::FastImportGitRepository(const RuleRepository &rule) :
    name(rule.getName()),
    prefix(rule.getForwardTo()),
    fastImport(name),
    commitCount(0),
    outstandingTransactions(0),
    last_commit_mark(0),
    next_file_mark(maxMark),
    processHasStarted(false)
{
    foreach (RuleRepository::Branch branchRule, rule.getBranches()) 
    {
        Branch branch;
        branch.created = 1;

        branches.insert(branchRule.name, branch);
    }

    // create the default branch
    branches["master"].created = 1;

    fastImport.setWorkingDirectory(name);
    
    if (!CommandLineParser::instance()->contains("dry-run") && !CommandLineParser::instance()->contains("create-dump")) 
    {
        if (!QDir(name).exists()) 
        { 
            // repo doesn't exist yet.
            qDebug() << "Creating new repository" << name;
            QDir::current().mkpath(name);
            QProcess init;
            init.setWorkingDirectory(name);
            init.start("git", QStringList() << "--bare" << "init");
            init.waitForFinished(-1);
            
            // Write description
            if (!rule.getDescription().isEmpty()) 
            {
                QFile fDesc(QDir(name).filePath("description"));
                if (fDesc.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) 
                {
                    fDesc.write(rule.getDescription().toUtf8());
                    fDesc.putChar('\n');
                    fDesc.close();
                }
            }
            
            QFile marks(name + "/" + marksFileName(name));
            marks.open(QIODevice::WriteOnly);
            marks.close();
        }
    }
}

FastImportGitRepository::~FastImportGitRepository()
{
    Q_ASSERT(outstandingTransactions == 0);
}

QString FastImportGitRepository::marksFileName(QString name)
{
    name.replace('/', '_');
    name.prepend("marks-");
    return name;
}

QString FastImportGitRepository::logFileName(QString name)
{
    name.replace('/', '_');
    
    if (CommandLineParser::instance()->contains("create-dump"))
    {
        name.append(".fi");
    }
    else
    {
        name.prepend("log-");
    }
    
    return name;
}

unsigned long long FastImportGitRepository::lastValidMark(QString name)
{
    QFile marksfile(name + "/" + marksFileName(name));
    
    if (!marksfile.open(QIODevice::ReadOnly))
    {
        return 0;
    }

    qDebug()  << "marksfile " << marksfile.fileName();
    unsigned long long prev_mark = 0;

    int lineno = 0;
    while (!marksfile.atEnd()) 
    {
        QString line = marksfile.readLine();
        ++lineno;
        
        if (line.isEmpty())
        {
            continue;
        }

        unsigned long long mark = 0;
        if (line[0] == ':') 
        {
            int sp = line.indexOf(' ');
            
            if (sp != -1) 
            {
                QString m = line.mid(1, sp-1);
                mark = m.toULongLong();
            }
        }

        if (!mark) 
        {
            qCritical() << marksfile.fileName() << "line" << lineno << "marks file corrupt?" << "mark " << mark;
            
            return 0;
        }

        if (mark == prev_mark) 
        {
            qCritical() << marksfile.fileName() << "line" << lineno << "marks file has duplicates";
            
            return 0;
        }

        if (mark < prev_mark) 
        {
            qCritical() << marksfile.fileName() << "line" << lineno << "marks file not sorted";
            
            return 0;
        }

        if (mark > prev_mark + 1)
        {
            break;
        }

        prev_mark = mark;
    }

    return prev_mark;
}

int FastImportGitRepository::setupIncremental(int &cutoff)
{
    QFile logfile(logFileName(name));
    
    if (!logfile.exists())
    {
        return 1;
    }

    logfile.open(QIODevice::ReadWrite);

    QRegExp progress("progress SVN r(\\d+) branch (.*) = :(\\d+)");

    unsigned long long last_valid_mark = lastValidMark(name);

    int last_revnum = 0;
    qint64 pos = 0;
    int retval = 0;
    QString bkup = logfile.fileName() + ".old";

    while (!logfile.atEnd()) 
    {
        pos = logfile.pos();
        QByteArray line = logfile.readLine();
        int hash = line.indexOf('#');
        
        if (hash != -1)
        {
            line.truncate(hash);
        }
        
        line = line.trimmed();
        
        if (line.isEmpty())
        {
            continue;
        }
        
        if (!progress.exactMatch(line))
        {
            continue;
        }

        int revnum = progress.cap(1).toInt();
        QString branch = progress.cap(2);
        unsigned long long mark = progress.cap(3).toULongLong();

        if (revnum >= cutoff)
        {
            goto beyond_cutoff;
        }

        if (revnum < last_revnum)
        {
            qWarning() << "WARN:" << name << "revision numbers are not monotonic: got" << QString::number(last_revnum) << "and then" << QString::number(revnum);
        }

        if (mark > last_valid_mark) 
        {
            qWarning() << "WARN:" << name << "unknown commit mark found: rewinding -- did you hit Ctrl-C?";
            cutoff = revnum;
            goto beyond_cutoff;
        }

        last_revnum = revnum;

        if (last_commit_mark < mark)
            last_commit_mark = mark;

        Branch &br = branches[branch];
        if (!br.created || !mark || br.marks.isEmpty() || !br.marks.last())
            br.created = revnum;
        br.commits.append(revnum);
        br.marks.append(mark);
    }

    retval = last_revnum + 1;
    if (retval == cutoff)
    {
        /*
        * If a stale backup file exists already, remove it, so that
        * we don't confuse ourselves in 'restoreLog()'
        */
        QFile::remove(bkup);
    }

    return retval;

beyond_cutoff:

    // backup file, since we'll truncate
    QFile::remove(bkup);
    logfile.copy(bkup);

    // truncate, so that we ignore the rest of the revisions
    qDebug() << name << "truncating history to revision" << cutoff;
    
    logfile.resize(pos);
    
    return cutoff;
}

void FastImportGitRepository::restoreLog()
{
    QString file = logFileName(name);
    QString bkup = file + ".old";
    
    if (!QFile::exists(bkup))
    {
        return;
    }
    
    QFile::remove(file);
    QFile::rename(bkup, file);
}

void FastImportGitRepository::doCheckpoint()
{
    qDebug() << "checkpoint!, marks file trunkated" << endl;
    fastImport.write("checkpoint\n");
    fastImport.waitForBytesWritten(-1);
    
    fastImport.write("progress complete\n");
    fastImport.waitForBytesWritten(-1);
    
    fastImport.waitForReadyRead(-1);
    const QByteArray result = fastImport.readAll();
    
    if (!result.contains("complete"))
    {
        qDebug() << "checkpoint not completed properly!" << endl;
    }
    
    qDebug() << "checkpoint done!" << endl;
}

void FastImportGitRepository::closeFastImport()
{
    if (fastImport.state() != QProcess::NotRunning) 
    {
        doCheckpoint();
        
        // Give the signal to close up shop.
        fastImport.write("done\n");
        fastImport.closeWriteChannel();
        
        fastImport.write("progress complete\n");
        fastImport.waitForBytesWritten(-1);
    
        fastImport.waitForReadyRead(-1);
        const QByteArray result = fastImport.readAll();
    
        if (!result.contains("complete"))
        {
            qDebug() << "Data stream not properly closed!" << endl;
        }
        
        // Wait at least one more minute for closure.
        if (!fastImport.waitForFinished(60000))
        {
            fastImport.terminate();
            
            if (!fastImport.waitForFinished(30000))
            {
                qWarning() << "WARN: git-fast-import for repository" << name << "did not die";
            }
        }
    }
    
    processHasStarted = false;
    processCache.remove(this);
}

void FastImportGitRepository::reloadBranches()
{
    bool reset_notes = false;
    foreach (QString branch, branches.keys()) 
    {
        Branch &br = branches[branch];

        if (br.marks.isEmpty() || !br.marks.last())
        {
            continue;
        }

	reset_notes = true;

        QByteArray branchRef = branch.toUtf8();
        
        if (!branchRef.startsWith("refs/"))
        {
            branchRef.prepend("refs/heads/");
        }

        fastImport.write("reset " + branchRef + "\nfrom :" + QByteArray::number(br.marks.last()) + "\n\nprogress Branch " + branchRef + " reloaded\n");
    }

    if (reset_notes && CommandLineParser::instance()->contains("add-metadata-notes")) 
    {
        fastImport.write("reset refs/notes/commits\nfrom :" + QByteArray::number(maxMark + 1) + "\n");
    }
}

long long FastImportGitRepository::markFrom(const QString &branchFrom, int branchRevNum, QByteArray &branchFromDesc)
{
    Branch &brFrom = branches[branchFrom];
    
    if (!brFrom.created)
    {
        return -1;
    }

    if (brFrom.commits.isEmpty()) 
    {
        return -1;
    }
    
    if (branchRevNum == brFrom.commits.last()) 
    {
        return brFrom.marks.last();
    }

    QVector<int>::const_iterator it = qUpperBound(brFrom.commits, branchRevNum);
    
    if (it == brFrom.commits.begin()) 
    {
        return 0;
    }

    int closestCommit = *--it;

    if (!branchFromDesc.isEmpty()) 
    {
        branchFromDesc += " at r" + QByteArray::number(branchRevNum);
        
        if (closestCommit != branchRevNum) 
        {
            branchFromDesc += " => r" + QByteArray::number(closestCommit);
        }
    }

    return brFrom.marks[it - brFrom.commits.begin()];
}

int FastImportGitRepository::createBranch(const QString &branch, int revnum, const QString &branchFrom, int branchRevNum)
{
    QByteArray branchFromDesc = "from branch " + branchFrom.toUtf8();
    long long mark = markFrom(branchFrom, branchRevNum, branchFromDesc);

    if (mark == -1) 
    {
        qCritical() << branch << "in repository" << name << "is branching from branch" << branchFrom << "but the latter doesn't exist. Can't continue.";
        return EXIT_FAILURE;
    }

    QByteArray branchFromRef = ":" + QByteArray::number(mark);
    
    if (!mark) 
    {
        qWarning() << "WARN:" << branch << "in repository" << name << "is branching but no exported commits exist in repository creating an empty branch.";
        branchFromRef = branchFrom.toUtf8();
        
        if (!branchFromRef.startsWith("refs/"))
        {
            branchFromRef.prepend("refs/heads/");
        }
        
        branchFromDesc += ", deleted/unknown";
    }

    qDebug() << "Creating branch:" << branch << "from" << branchFrom << "(" << branchRevNum << branchFromDesc << ")";

    // Preserve note
    branches[branch].note = branches.value(branchFrom).note;

    return resetBranch(branch, revnum, mark, branchFromRef, branchFromDesc);
}

int FastImportGitRepository::deleteBranch(const QString &branch, int revnum)
{
    static QByteArray null_sha(40, '0');
    return resetBranch(branch, revnum, 0, null_sha, "delete");
}

int FastImportGitRepository::resetBranch(const QString &branch, int revnum, unsigned long long mark, const QByteArray &resetTo, const QByteArray &comment)
{
    QByteArray branchRef = branch.toUtf8();
    
    if (!branchRef.startsWith("refs/"))
    {
        branchRef.prepend("refs/heads/");
    }

    Branch &br = branches[branch];
    QByteArray backupCmd;
    
    if (br.created && br.created != revnum && !br.marks.isEmpty() && br.marks.last()) 
    {
        QByteArray backupBranch;
        if ((comment == "delete") && branchRef.startsWith("refs/heads/"))
        {
            backupBranch = "refs/tags/backups/" + branchRef.mid(11) + "@" + QByteArray::number(revnum);
        }
        else
        {
            backupBranch = "refs/backups/r" + QByteArray::number(revnum) + branchRef.mid(4);
        }
        
        qWarning() << "WARN: backing up branch" << branch << "to" << backupBranch;

        backupCmd = "reset " + backupBranch + "\nfrom " + branchRef + "\n\n";
    }

    br.created = revnum;
    br.commits.append(revnum);
    br.marks.append(mark);

    QByteArray cmd = "reset " + branchRef + "\nfrom " + resetTo + "\n\nprogress SVN r" + QByteArray::number(revnum) + " branch " + branch.toUtf8() + " = :" + QByteArray::number(mark) + " # " + comment + "\n\n";
    
    if(comment == "delete")
    {
        deletedBranches.append(backupCmd).append(cmd);
    }
    else
    {
        resetBranches.append(backupCmd).append(cmd);
    }

    return EXIT_SUCCESS;
}

void FastImportGitRepository::commit()
{
    if (deletedBranches.isEmpty() && resetBranches.isEmpty()) 
    {
        return;
    }
    
    startFastImport();
    fastImport.write(deletedBranches);
    fastImport.write(resetBranches);
    deletedBranches.clear();
    resetBranches.clear();
}

GitRepositoryTransaction* FastImportGitRepository::newTransaction(const QString &branch, const QString &svnprefix, int revnum)
{
    if (!branches.contains(branch)) 
    {
        qWarning() << "WARN: Transaction:" << branch << "is not a known branch in repository" << name << endl << "Going to create it automatically";
    }

    FastImportGitRepositoryTransaction *txn = new FastImportGitRepositoryTransaction;
    txn->repository = this;
    txn->branch = branch.toUtf8();
    txn->svnprefix = svnprefix.toUtf8();
    txn->datetime = 0;
    txn->revnum = revnum;

    if ((++commitCount % CommandLineParser::instance()->optionArgument(QLatin1String("commit-interval"), QLatin1String("10000")).toInt()) == 0) 
    {
        startFastImport();
        
        // write everything to disk every 10000 commits
        doCheckpoint();
    }
    
    outstandingTransactions++;
    
    return txn;
}

void FastImportGitRepository::forgetTransaction(FastImportGitRepositoryTransaction *)
{
    if (!--outstandingTransactions)
    {
        next_file_mark = maxMark;
    }
}

void FastImportGitRepository::createAnnotatedTag(const QString &ref, const QString &svnprefix, int revnum, const QByteArray &author, uint dt, const QByteArray &log)
{
    QString tagName = ref;
    
    if (tagName.startsWith("refs/tags/"))
    {
        tagName.remove(0, 10);
    }

    if (!annotatedTags.contains(tagName))
    {
        printf("Creating annotated tag %s (%s)\n", qPrintable(tagName), qPrintable(ref));
    }
    else
    {
        printf("Re-creating annotated tag %s\n", qPrintable(tagName));
    }

    AnnotatedTag &tag = annotatedTags[tagName];
    tag.supportingRef = ref;
    tag.svnprefix = svnprefix.toUtf8();
    tag.revnum = revnum;
    tag.author = author;
    tag.log = log;
    tag.dt = dt;
}

void FastImportGitRepository::close()
{
	closeFastImport();
}

void FastImportGitRepository::finalizeTags()
{
    if (annotatedTags.isEmpty())
    {
        return;
    }

    printf("Finalising tags for %s...", qPrintable(name));
    startFastImport();

    QHash<QString, AnnotatedTag>::ConstIterator it = annotatedTags.constBegin();
    for ( ; it != annotatedTags.constEnd(); ++it) 
    {
        const QString &tagName = it.key();
        const AnnotatedTag &tag = it.value();

        QByteArray message = tag.log;
        if (!message.endsWith('\n'))
        {
            message += '\n';
        }
        
        if (CommandLineParser::instance()->contains("add-metadata"))
        {
            message += "\n" + formatMetadataMessage(tag.svnprefix, tag.revnum, tagName.toUtf8());
        }

        QByteArray branchRef = tag.supportingRef.toUtf8();
        
        if (!branchRef.startsWith("refs/"))
        {
            branchRef.prepend("refs/heads/");
        }

        QByteArray s = "progress Creating annotated tag " + tagName.toUtf8() + " from ref " + branchRef + "\n" + "tag " + tagName.toUtf8() + "\n" + "from " + branchRef + "\n" + "tagger " + tag.author + ' ' + QByteArray::number(tag.dt) + " +0000" + "\n" + "data " + QByteArray::number( message.length() ) + "\n";
        
        fastImport.write(s);

        fastImport.write(message);
        fastImport.putChar('\n');
        
        if (!fastImport.waitForBytesWritten(-1))
        {
            qFatal("Failed to write to process: %s", qPrintable(fastImport.errorString()));
        }

        // Append note to the tip commit of the supporting ref. There is no
        // easy way to attach a note to the tag itself with fast-import.
        if (CommandLineParser::instance()->contains("add-metadata-notes")) 
        {
            GitRepositoryTransaction* txn = newTransaction(tag.supportingRef, tag.svnprefix, tag.revnum);
            txn->setAuthor(tag.author);
            txn->setDateTime(tag.dt);
            txn->commitNote(formatMetadataMessage(tag.svnprefix, tag.revnum, tagName.toUtf8()), true);
            delete txn;

            if (!fastImport.waitForBytesWritten(-1))
            {
                qFatal("Failed to write to process: %s", qPrintable(fastImport.errorString()));
            }
        }

        printf(" %s", qPrintable(tagName));
        
        fflush(stdout);
    }

    while (fastImport.bytesToWrite())
    {
        if (!fastImport.waitForBytesWritten(-1))
        {
            qFatal("Failed to write to process: %s", qPrintable(fastImport.errorString()));
        }
    }
    
    printf("\n");
}


QByteArray FastImportGitRepository::msgFilter(QByteArray msg)
{
    QByteArray output = msg;

    if (CommandLineParser::instance()->contains("msg-filter")) 
    {
	if (filterMsg.state() == QProcess::Running)
        {
	    qFatal("filter process already running?");
        }

	filterMsg.start(CommandLineParser::instance()->optionArgument("msg-filter"));

	if(!(filterMsg.waitForStarted(-1)))
        {
	    qFatal("Failed to Start Filter %d %s", __LINE__, qPrintable(filterMsg.errorString()));
        }

	filterMsg.write(msg);
	filterMsg.closeWriteChannel();
	filterMsg.waitForFinished();
	output = filterMsg.readAllStandardOutput();
    }
    
    return output;
}

void FastImportGitRepository::startFastImport()
{
    processCache.touch(this);

    if (fastImport.state() == QProcess::NotRunning) 
    {
        if (processHasStarted)
        {
            qFatal("git-fast-import has been started once and crashed?");
        }
        
        const QString maxPackSize = CommandLineParser::instance()->optionArgument(QLatin1String("max-packsize"), QLatin1String(""));
        
        processHasStarted = true;

        // start the process
        QString marksFile = marksFileName(name);
        QStringList marksOptions;
        marksOptions << "--import-marks=" + marksFile;
        marksOptions << "--export-marks=" + marksFile;
        marksOptions << "--force";
        
        if (!maxPackSize.isEmpty())
        {
            marksOptions << "--max-pack-size=" + maxPackSize;
        }

        fastImport.setStandardOutputFile(logFileName(name), QIODevice::Append);
        fastImport.setProcessChannelMode(QProcess::MergedChannels);

        if (!CommandLineParser::instance()->contains("dry-run") && !CommandLineParser::instance()->contains("create-dump")) 
        {
            fastImport.start("git", QStringList() << "fast-import" << marksOptions);
        } 
        else 
        {
            fastImport.start("/bin/cat", QStringList());
        }
        
        fastImport.waitForStarted(-1);

        reloadBranches();
    }
}

bool FastImportGitRepository::branchExists(const QString& branch) const
{
    return branches.contains(branch);
}

const QByteArray FastImportGitRepository::branchNote(const QString& branch) const
{
    return branches.value(branch).note;
}

void FastImportGitRepository::setBranchNote(const QString& branch, const QByteArray& noteText)
{
    if (branches.contains(branch))
        branches[branch].note = noteText;
}

bool FastImportGitRepository::hasPrefix() const
{
    return !prefix.isEmpty();
}

const QString& FastImportGitRepository::getName() const
{
    return name;
}

GitRepository* FastImportGitRepository::getEffectiveRepository()
{
    return this;
}
