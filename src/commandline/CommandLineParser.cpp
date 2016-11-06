/*
 * This file is part of the vng project
 * Copyright (C) 2008 Thomas Zander <tzander@trolltech.com>
 * Copyright (C) 2016 Daniel Dewald <daniel.dewald@innogames.com>
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

#include "CommandLineParser.h"

#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QList>
#include <QHash>

#include "CommandLineParserPrivate.h"

CommandLineParser *CommandLineParser::self = 0;

// static
void CommandLineParser::init(int argc, char** argv)
{
    if (self)
    {
        delete self;
    }
    
    self = new CommandLineParser(argc, argv);
}

// static
void CommandLineParser::addOptionDefinitions(const CommandLineOption* optionList)
{
    if (!self) 
    {
        qWarning() << "WARN: CommandLineParser:: Use init before addOptionDefinitions!";
        return;
    }
    
    self->d->addDefinitions(optionList);
}

// static
CommandLineParser* CommandLineParser::instance()
{
    return self;
}

// static
void CommandLineParser::setArgumentDefinition(const char* definition)
{
    if (!self) 
    {
        qWarning() << "WARN: CommandLineParser:: Use init before addOptionDefinitions!";
        return;
    }
    
    self->d->setArgumentDefinition(definition);
}


CommandLineParser::CommandLineParser(int argc, char **argv) : 
    d(new CommandLineParserPrivate(argc, argv))
{
}

CommandLineParser::~CommandLineParser()
{
    delete d;
}

void CommandLineParser::usage(const QString &name, const QString &argumentDescription)
{
    QTextStream cout(stdout, QIODevice::WriteOnly);
    cout << "Usage: " << d->argumentStrings[0];
    
    if (! name.isEmpty())
    {
        cout << " " << name;
    }
    
    if (d->definitions.count())
    {
        cout << " [OPTION]";
    }
    
    if (! argumentDescription.isEmpty())
    {
        cout << " " << argumentDescription;
    }
    
    cout << endl << endl;

    if (d->definitions.count() > 0)
    {
        cout << "Options:" << endl;
    }
    
    int commandLength = 0;
    foreach (OptionDefinition definition, d->definitions)
    {
        commandLength = qMax(definition.name.length(), commandLength);
    }

    foreach (OptionDefinition definition, d->definitions) 
    {
        cout << "    ";
        
        if (definition.shortName == 0)
        {
            cout << "   --";
        }
        else
        {
            cout << "-" << definition.shortName << " --";
        }
        
        cout << definition.name;
        
        for (int i = definition.name.length(); i <= commandLength; i++)
        {
            cout << ' ';
        }
        
        cout << definition.comment <<endl;
    }
}

QStringList CommandLineParser::options() const
{
    d->parse();
    return d->options.keys();
}

bool CommandLineParser::contains(const QString & key) const
{
    d->parse();
    return d->options.contains(key);
}

QStringList CommandLineParser::arguments() const
{
    d->parse();
    return d->arguments;
}

QStringList CommandLineParser::undefinedOptions() const
{
    d->parse();
    return d->undefinedOptions;
}

QString CommandLineParser::optionArgument(const QString &optionName, const QString &defaultValue) const
{
    QStringList answer = optionArguments(optionName);
    
    if (answer.isEmpty())
    {
        return defaultValue;
    }
    
    return answer.first();
}

QStringList CommandLineParser::optionArguments(const QString &optionName) const
{
    if (! contains(optionName))
    {
        return QStringList();
    }
    
    ParsedOption po = d->options[optionName];
    return po.parameters;
}

QStringList CommandLineParser::parseErrors() const
{
    d->parse();
    return d->errors;
}
