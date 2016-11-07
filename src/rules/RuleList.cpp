#include "RuleList.h"

#include <QStringList>
#include <QString>
#include <QDebug>

RuleList::RuleList(const QString& filenames) : 
    filenames(filenames)
{
    
}

RuleList::~RuleList() 
{
    
}

void RuleList::load()
{
    foreach(const QString filename, this->filenames.split(',') ) 
    {
        qDebug() << "Loading rules from:" << filename;
        
        Rules *rules = new Rules(filename);
        this->rules.append(rules);
        
        rules->load();
        this->allrepositories.append(rules->getRepositories());
        
        QList<RuleMatch> matchRules = rules->getMatchRules();
        this->allMatchRules.append( QList<RuleMatch>(matchRules));
    }
}

const QList<RuleRepository> RuleList::getAllRepositories() const
{
    return allrepositories;
}

const QList<QList<RuleMatch> > RuleList::getAllMatchRules() const
{
    return allMatchRules;
}

const QList<Rules*> RuleList::getRules() const
{
    return rules;
}
