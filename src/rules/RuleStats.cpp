#include "RuleStats.h"
#include "RuleStatsPrivate.h"

#include "commandline/CommandLineParser.h"

RuleStats *RuleStats::self = 0;

RuleStats::RuleStats() : 
    privateClass(new RuleStatsPrivate())
{
    use = CommandLineParser::instance()->contains("stats");
}

RuleStats::~RuleStats()
{
    delete privateClass;
}

void RuleStats::init()
{
    if(self)
    {
        delete self;
    }
    
    self = new RuleStats();
}

RuleStats* RuleStats::instance()
{
    return self;
}

void RuleStats::printStats() const
{
    if(use)
    {
        privateClass->printStats();
    }
}

void RuleStats::ruleMatched(const RuleMatch& rule, const int rev)
{
    if(use)
    {
        privateClass->ruleMatched(rule, rev);
    }
}

void RuleStats::addRule( const RuleMatch& rule)
{
    if(use)
    {
        privateClass->addRule(rule);
    }
}
