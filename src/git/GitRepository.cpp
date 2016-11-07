#include "GitRepository.h"

#include <QDebug>

#include "FastImportGitRepository.h"
#include "ForwardingGitRepository.h"

GitRepository* GitRepository::createRepository(const RuleRepository& rule, const QHash<QString, GitRepository*>& repositories)
{
    if (rule.getForwardTo().isEmpty())
    {
        return new FastImportGitRepository(rule);
    }
    
    GitRepository *r = repositories[rule.getForwardTo()];
    
    if (!r) 
    {
        qCritical() << "no repository with name" << rule.getForwardTo() << "found at" << rule.info();
        return r;
    }
    
    return new ForwardingGitRepository(rule.getName(), r, rule.getPrefix());
}

const QByteArray GitRepository::formatMetadataMessage(const QByteArray &svnprefix, int revnum, const QByteArray &tag)
{
    QByteArray msg = "svn path=" + svnprefix + "; revision=" + QByteArray::number(revnum);
    
    if (!tag.isEmpty())
    {
        msg += "; tag=" + tag;
    }
    
    msg += "\n";
    
    return msg;
}
