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

#ifndef RULE_STATS_PRIVATE_H
#define RULE_STATS_PRIVATE_H

#include <QMap>

class RuleMatch;

class RuleStatsPrivate
{
    
public:
    
    RuleStatsPrivate();

    void printStats() const;
    void ruleMatched(const RuleMatch& rule, const int rev);
    void addRule(const RuleMatch& rule);
    
private:
    
    QMap<RuleMatch,int> m_usedRules;
};

#endif
