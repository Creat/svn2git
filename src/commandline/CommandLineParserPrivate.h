/*
 * This file is part of the vng project
 * Copyright (C) 2008 Thomas Zander <tzander@trolltech.com>
 * Copyright (C) 2016 Daniel Dewald <daniel.dewald@innogame.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMANDLINEPARSER_PRIVATE_H
#define COMMANDLINEPARSER_PRIVATE_H

#include <QHash>
#include <QList>
#include <QString>

#include "ParsedOption.h"
#include "OptionDefinition.h"
#include "CommandLineOption.h"

class CommandLineParserPrivate
{
    
public:
    
    CommandLineParserPrivate(int argc, char** argv);

    // functions
    void addDefinitions(const CommandLineOption* options);
    void setArgumentDefinition(const char* definition);
    void parse();

    // variables;
    const int argumentCount;
    char** const argumentStrings;
    bool dirty;
    int requiredArguments;
    QString argumentDefinition;

    QList<OptionDefinition> definitions;
    QHash<QString, ParsedOption> options;
    QList<QString> arguments;
    QList<QString> undefinedOptions;
    QList<QString> errors;
};

#endif
