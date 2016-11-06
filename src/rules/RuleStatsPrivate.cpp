#include "RuleStatsPrivate.h"

#include <QDebug>

#include "RuleMatch.h"

RuleStatsPrivate::RuleStatsPrivate()
{
}

void RuleStatsPrivate::printStats() const
{
    printf("\nRule stats\n");
    foreach(const RuleMatch rule, m_usedRules.keys()) 
    {
        printf("%s was matched %i times\n", qPrintable(rule.info()), m_usedRules[rule]);
    }
}

void RuleStatsPrivate::ruleMatched(const RuleMatch &rule, const int rev)
{
    Q_UNUSED(rev);
    if(!m_usedRules.contains(rule)) 
    {
        m_usedRules.insert(rule, 1);
        qWarning() << "WARN: New match rule" << rule.info() << ", should have been added when created.";
    } 
    else 
    {
        m_usedRules[rule]++;
    }
}

void RuleStatsPrivate::addRule( const RuleMatch &rule)
{
    if(m_usedRules.contains(rule))
    {
        qWarning() << "WARN: Rule" << rule.info() << "was added multiple times.";
    }
    
    m_usedRules.insert(rule, 0);
}
