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

#ifndef RULE_MATCH_H
#define RULE_MATCH_H

#include <QList>

#include "Rule.h"
#include "RuleMatchSubstitution.h"
#include "RuleMatchAction.h"

class QString;
class QRegExp;

class RuleMatch : public Rule
{
    
public:

    RuleMatch();

    bool operator<(const RuleMatch& other) const;
    const QString info() const;
    
    QRegExp rx;
    QString repository;
    QList<RuleMatchSubstitution> repo_substs;
    QString branch;
    QList<RuleMatchSubstitution> branch_substs;
    QString prefix;
    int minRevision;
    int maxRevision;
    bool annotate;
    RuleMatchAction action;
};

#ifndef QT_NO_DEBUG_STREAM
class QDebug;
QDebug operator<<(QDebug, const RuleMatch&);
#endif


#endif
