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

#include "OptionProcessor.h"

#include "OptionDefinition.h"
#include "CommandLineParserPrivate.h"

OptionProcessor::OptionProcessor(CommandLineParserPrivate* d) : 
    clp(d) 
{ 
}

void OptionProcessor::next(ParsedOption& option) 
{
    if (! option.option.isEmpty()) 
    {
        // find the definition to match.
        OptionDefinition def;
        
        foreach (OptionDefinition definition, clp->definitions) 
        {
            if (definition.name == option.option) 
            {
                def = definition;
                break;
            }
        }
                
        if (! def.name.isEmpty() && def.requiredParameters >= option.parameters.count() && def.requiredParameters + def.optionalParameters <= option.parameters.count())
        {
            clp->options.insert(option.option, option);
        }
        else if (!clp->undefinedOptions.contains(option.option))
        {
            clp->undefinedOptions << option.option;
        }
        else
        {
            clp->errors.append(QLatin1String("Not enough arguments passed for option `") + option.option +QLatin1Char('\''));
        }
    }
    
    option.option.clear();
    option.parameters.clear();
}
