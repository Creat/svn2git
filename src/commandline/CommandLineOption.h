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

#ifndef COMMAND_LINE_OPTION_H
#define COMMAND_LINE_OPTION_H

struct CommandLineOption 
{
    /**
     * The specification of an option includes the name of the option the user must pass and optional arguments it has.
     * Example specifications are;
     * <ol>
     * <li>"-a, --all" </li>
     * <li>"--version" </li>
     * <li>"--type name" </li>
     * <li>"--list item[,item]" </li>
     * <li>"-f, --format name [suffix] [foo]" </li> </ol>
     * Number 1 allows the user to either type -a or --all (or /A on Windows) to activate this option.
     * Number 2 allows the user to type --version to activate this option.
     * Number 3 requires the user to type a single argument after the option.
     * Number 4 allows the user to use an option that takes a required argument and one or more optional ones
     * Number 5 Allows the user to either use -f or --format, which is followed by one required argument
     *          and optionally 2 more arguments.
     */
    const char* specification;
    /**
     * A textual description of the option that will be printed when the user asks for help.
     */
    const char* description;
};

#endif
