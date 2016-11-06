#include "Rules.h"

#include <QDebug>
#include <QFile>
#include <QStringList>

#include "RuleStats.h"
#include "RuleMatchAction.h"

Rules::Rules(const QString &fn) : 
    filename(fn)
{
}

Rules::~Rules()
{
}

const QList<RuleRepository> Rules::getRepositories() const
{
    return repositories;
}

const QList<RuleMatch> Rules::getMatchRules() const
{
    return matchRules;
}

RuleMatchSubstitution Rules::parseSubstitution(const QString &string)
{
    if (string.at(0) != 's' || string.length() < 5)
    {
        return RuleMatchSubstitution();
    }

    const QChar sep = string.at(1);

    if (string.at(string.length() - 1) != sep)
    {
        return RuleMatchSubstitution();
    }

    int i = 2, end = 0;
    RuleMatchSubstitution subst;

    // Separator might have been escaped with a backslash
    while (i > end) 
    {
        int backslashCount = 0;
        if ((end = string.indexOf(sep, i)) > -1) 
        {
            for (i = end - 1; i >= 2; i--) 
            {
                if (string.at(i) == '\\')
                {
                    backslashCount++;
                }
                else
                {
                    break;
                }
            }
        } 
        else 
        {
            return RuleMatchSubstitution(); // error
        }

        if (backslashCount % 2 != 0) 
        {
            // Separator was escaped. Search for another one
            i = end + 1;
        }
    }

    // Found the end of the pattern
    subst.pattern = QRegExp(string.mid(2, end - 2));
    if (!subst.pattern.isValid())
    {
        return RuleMatchSubstitution(); // error
    }
    
    subst.replacement = string.mid(end + 1, string.length() - 1 - end - 1);

    return subst;
}

void Rules::load()
{
    load(filename);
}

void Rules::load(const QString &filename)
{
    qDebug() << "Loading rules from" << filename;
    
    // initialize the regexps we will use
    QRegExp repoLine("create repository\\s+(\\S+)", Qt::CaseInsensitive);

    QString varRegex("[A-Za-z0-9_]+");

    QRegExp matchLine("match\\s+(.*)", Qt::CaseInsensitive);
    QRegExp matchActionLine("action\\s+(\\w+)", Qt::CaseInsensitive);
    QRegExp matchRepoLine("repository\\s+(\\S+)", Qt::CaseInsensitive);
    QRegExp matchDescLine("description\\s+(.+)$", Qt::CaseInsensitive);
    QRegExp matchRepoSubstLine("substitute repository\\s+(.+)$", Qt::CaseInsensitive);
    QRegExp matchBranchLine("branch\\s+(\\S+)", Qt::CaseInsensitive);
    QRegExp matchBranchSubstLine("substitute branch\\s+(.+)$", Qt::CaseInsensitive);
    QRegExp matchRevLine("(min|max) revision (\\d+)", Qt::CaseInsensitive);
    QRegExp matchAnnotateLine("annotated\\s+(\\S+)", Qt::CaseInsensitive);
    QRegExp matchPrefixLine("prefix\\s+(.*)$", Qt::CaseInsensitive);
    QRegExp declareLine("declare\\s+("+varRegex+")\\s*=\\s*(\\S+)", Qt::CaseInsensitive);
    QRegExp variableLine("\\$\\{("+varRegex+")(\\|[^}$]*)?\\}", Qt::CaseInsensitive);
    QRegExp includeLine("include\\s+(.*)", Qt::CaseInsensitive);

    enum { ReadingNone, ReadingRepository, ReadingMatch } state = ReadingNone;
    RuleRepository repo;
    RuleMatch match;
    int lineNumber = 0;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qFatal("Could not read the rules file: %s", qPrintable(filename));
    }

    QTextStream s(&file);
    QStringList lines = s.readAll().split('\n', QString::KeepEmptyParts);

    QStringList::iterator it;
    for(it = lines.begin(); it != lines.end(); ++it) 
    {
        ++lineNumber;
        QString origLine = *it;
        QString line = origLine;

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

        bool isIncludeRule = includeLine.exactMatch(line);
        
        if (isIncludeRule) 
        {
            int index = filename.lastIndexOf("/");
            QString includeFile = filename.left( index + 1) + includeLine.cap(1);
            load(includeFile);
        } 
        else 
        {
            while( variableLine.indexIn(line) != -1 ) 
            {
                QString replacement;
                
                if (variables.contains(variableLine.cap(1))) 
                {
                    replacement = variables[variableLine.cap(1)];
                } 
                else 
                {
                    if (variableLine.cap(2).startsWith('|')) 
                    {
                        replacement = variableLine.cap(2).mid(1);
                    } 
                    else 
                    {
                        qFatal("Undeclared variable: %s", qPrintable(variableLine.cap(1)));
                    }
                }
                
                line = line.replace(variableLine.cap(0), replacement);
            }
            
            if (state == ReadingRepository) 
            {
                if (matchBranchLine.exactMatch(line)) 
                {
                    RuleRepository::Branch branch;
                    branch.name = matchBranchLine.cap(1);

                    repo.addBranch(branch);
                    continue;
                } 
                else if (matchDescLine.exactMatch(line)) 
                {
                    repo.setDescription(matchDescLine.cap(1));
                    continue;
                } 
                else if (matchRepoLine.exactMatch(line)) 
                {
                    repo.setForwardTo(matchRepoLine.cap(1));
                    continue;
                } 
                else if (matchPrefixLine.exactMatch(line)) 
                {
                    repo.setPrefix(matchPrefixLine.cap(1));
                    continue;
                } 
                else if (line == "end repository") 
                {
                    if (!repo.getForwardTo().isEmpty() && !repo.getDescription().isEmpty()) 
                    {
                        qFatal("Specifing repository and description on repository is invalid on line %d", lineNumber);
                    }

                    if (!repo.getForwardTo().isEmpty() && !repo.getBranches().isEmpty()) 
                    {
                        qFatal("Specifing repository and branches on repository is invalid on line %d", lineNumber);
                    }

                    repositories += repo;

                    // clear out 'repo'
                    RuleRepository temp;
                    std::swap(repo, temp);

                    state = ReadingNone;
                    continue;
                }
            } 
            else if (state == ReadingMatch) 
            {
                if (matchRepoLine.exactMatch(line)) 
                {
                    match.repository = matchRepoLine.cap(1);
                    continue;
                } 
                else if (matchBranchLine.exactMatch(line)) 
                {
                    match.branch = matchBranchLine.cap(1);
                    continue;
                } 
                else if (matchRepoSubstLine.exactMatch(line)) 
                {
                    RuleMatchSubstitution subst = parseSubstitution(matchRepoSubstLine.cap(1));
                    
                    if (!subst.isValid()) 
                    {
                        qFatal("Malformed substitution in rules file: line %d: %s",
                            lineNumber, qPrintable(origLine));
                    }
                    
                    match.repo_substs += subst;
                    continue;
                } 
                else if (matchBranchSubstLine.exactMatch(line)) 
                {
                    RuleMatchSubstitution subst = parseSubstitution(matchBranchSubstLine.cap(1));
                    
                    if (!subst.isValid()) 
                    {
                        qFatal("Malformed substitution in rules file: line %d: %s",
                            lineNumber, qPrintable(origLine));
                    }
                    
                    match.branch_substs += subst;
                    continue;
                } 
                else if (matchRevLine.exactMatch(line)) 
                {
                    if (matchRevLine.cap(1) == "min")
                    {
                        match.minRevision = matchRevLine.cap(2).toInt();
                    }
                    else            // must be max
                    {
                        match.maxRevision = matchRevLine.cap(2).toInt();
                    }
                    
                    continue;
                } 
                else if (matchPrefixLine.exactMatch(line)) 
                {
                    match.prefix = matchPrefixLine.cap(1);
                    
                    if( match.prefix.startsWith('/'))
                    {
                        match.prefix = match.prefix.mid(1);
                    }
                    
                    continue;
                } 
                else if (matchActionLine.exactMatch(line)) 
                {
                    QString action = matchActionLine.cap(1);
                    
                    if (action == "export")
                    {
                        match.action = Export;
                    }
                    else if (action == "ignore")
                    {
                        match.action = Ignore;
                    }
                    else if (action == "recurse")
                    {
                        match.action = Recurse;
                    }
                    else
                    {
                        qFatal("Invalid action \"%s\" on line %d", qPrintable(action), lineNumber);
                    }
                    
                    continue;
                } 
                else if (matchAnnotateLine.exactMatch(line)) 
                {
                    match.annotate = matchAnnotateLine.cap(1) == "true";
                    continue;
                } 
                else if (line == "end match") 
                {
                    if (!match.repository.isEmpty())
                    {
                        match.action = Export;
                    }
                    
                    matchRules += match;
                    RuleStats::instance()->addRule(match);
                    state = ReadingNone;
                    continue;
                }
            }

            bool isRepositoryRule = repoLine.exactMatch(line);
            bool isMatchRule = matchLine.exactMatch(line);
            bool isVariableRule = declareLine.exactMatch(line);

            if (isRepositoryRule) 
            {
                // repository rule
                state = ReadingRepository;
                repo = RuleRepository(); // clear
                repo.setName(repoLine.cap(1));
                repo.setLineNumber(lineNumber);
                repo.setFilename(filename);
            } 
            else if (isMatchRule) 
            {
                // match rule
                state = ReadingMatch;
                match = RuleMatch();
                match.rx = QRegExp(matchLine.cap(1), Qt::CaseSensitive, QRegExp::RegExp2);
                
                if( !match.rx.isValid() )
                {
                    qFatal("Malformed regular expression '%s' in file:'%s':%d, Error: %s", qPrintable(matchLine.cap(1)), qPrintable(filename), lineNumber, qPrintable(match.rx.errorString()));
                }
                
                match.setLineNumber(lineNumber);
                match.setFilename(filename);
            } 
            else if (isVariableRule) 
            {
                QString variable = declareLine.cap(1);
                QString value = declareLine.cap(2);
                variables.insert(variable, value);
            } 
            else 
            {
                qFatal("Malformed line in rules file: line %d: %s", lineNumber, qPrintable(origLine));
            }
        }
    }
}

