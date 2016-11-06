#include "RuleRepository.h"

#include <QStringBuilder>

RuleRepository::RuleRepository() 
{ 
    
}
    
const QString RuleRepository::info() const 
{
    const QString info = filename % ":" % QByteArray::number(lineNumber);
    return info;
}

const QList<RuleRepository::Branch>& RuleRepository::getBranches() const
{
    return branches;
}

const QString& RuleRepository::getPrefix() const
{
    return prefix;
}

void RuleRepository::addBranch(const Branch& branch)
{
    branches += branch;
}

void RuleRepository::setDescription(const QString& newDescription)
{
    description = newDescription;
}

void RuleRepository::setForwardTo(const QString& newForwardTo)
{
    forwardTo = newForwardTo;
}

void RuleRepository::setPrefix(const QString& newPrefix)
{
    prefix = newPrefix;
}

void RuleRepository::setName(const QString& newName)
{
    name = newName;
}

const QString& RuleRepository::getDescription() const
{
    return description;
}

const QString& RuleRepository::getForwardTo() const
{
    return forwardTo;
}

const QString& RuleRepository::getName() const
{
    return name;
}

