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

#ifndef RULE_REPOSITORY_H
#define RULE_REPOSITORY_H

#include <QList>
#include <QString>

#include "Rule.h"

class QString;

class RuleRepository : public Rule
{
    
public:
    
    RuleRepository();
    
    struct Branch
    {
        QString name;
    };
    
    const QString info() const;

    void addBranch(const Branch& branch);
    void setDescription(const QString& newDescription);
    void setForwardTo(const QString& newForwardTo);
    void setPrefix(const QString& newPrefix);
    void setName(const QString& newName);
    
    const QString& getName() const;
    const QList<Branch>& getBranches() const;
    const QString& getDescription() const;
    const QString& getForwardTo() const;
    const QString& getPrefix() const;
    
protected:

    QString name;
    QList<Branch> branches;
    QString description;

    QString forwardTo;
    QString prefix;
};

#endif
