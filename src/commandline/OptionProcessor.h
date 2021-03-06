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

#ifndef OPTIONPROCESSOR_H
#define OPTIONPROCESSOR_H

#include "ParsedOption.h"

class CommandLineParserPrivate;

class OptionProcessor 
{

public:

    OptionProcessor(CommandLineParserPrivate* d);

    void next(ParsedOption& option);

private:
    
    CommandLineParserPrivate* clp;
};
    
#endif
