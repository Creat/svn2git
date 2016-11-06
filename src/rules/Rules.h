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

#ifndef RULES_H
#define RULES_H

#include <QString>
#include <QList>
#include <QMap>

#include "RuleMatch.h"
#include "RuleRepository.h"
#include "RuleMatchSubstitution.h"

class Rules
{
    
public:
    
    Rules(const QString &filename);
    ~Rules();

    const QList<RuleRepository> getRepositories() const;
    const QList<RuleMatch> getMatchRules() const;
    RuleMatchSubstitution parseSubstitution(const QString &string);
    void load();

private:
    
    void load(const QString &filename);
    QString filename;
    QList<RuleRepository> repositories;
    QList<RuleMatch> matchRules;
    QMap<QString,QString> variables;
};

#endif
