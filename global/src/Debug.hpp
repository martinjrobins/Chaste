/*

Copyright (C) University of Oxford, 2005-2011

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef DEBUG_HPP_
#define DEBUG_HPP_

#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include "PetscTools.hpp"

/**
 * @file
 * A bunch of useful macros for debugging.  These all output information to stdout as
 * lines starting "DEBUG: ".  When running in parallel the process number is also
 * displayed.  Each line is flushed as soon as it is written.
 *
 * @note Use of these should be removed from source code when committing.
 */

/** Print a 'header' for a debug output line */
std::string FormDebugHead();

/**
 * Print a debug message.
 * @param stuff  what to print (can be a variable, or e.g. a << " " << b)
 */
#define TRACE(stuff) std::cout << FormDebugHead() << stuff << std::endl << std::flush;

/** Print some trace containing the file name and line number. */
#define MARK std::cout << FormDebugHead() <<  __FILE__ << " at line " <<__LINE__ << std::endl << std::flush;

/** Print the name and value of the given variable.
 * @param var */
#define PRINT_VARIABLE(var) std::cout << FormDebugHead() << #var " = " << var << std::endl << std::flush;

/** Print the name and value of the given variables.
 * @param var1
 * @param var2
 */
#define PRINT_2_VARIABLES(var1,var2) std::cout << FormDebugHead() << #var1 " = " << var1 << ", " \
    #var2 " = " << var2 << std::endl << std::flush;

/** Print the name and value of the given variables.
 * @param var1
 * @param var2
 * @param var3
 */
#define PRINT_3_VARIABLES(var1,var2,var3) std::cout << FormDebugHead() << #var1 " = " << var1 << ", " \
    #var2 " = " << var2 << ", " #var3 " = " << var3 << std::endl << std::flush;

/** Print the name and value of the given variables.
 * @param var1
 * @param var2
 * @param var3
 * @param var4
 */
#define PRINT_4_VARIABLES(var1,var2,var3,var4) std::cout << FormDebugHead() << #var1 " = " << var1 << ", " \
    #var2 " = " << var2 << ", " #var3 " = " << var3 << ", " \
    #var4 " = " << var4 << std::endl << std::flush;

/** Print the name and value of the given variables.
 * @param var1
 * @param var2
 * @param var3
 * @param var4
 * @param var5
 */
#define PRINT_5_VARIABLES(var1,var2,var3,var4,var5) std::cout << FormDebugHead() << #var1 " = " << var1 << ", " \
    #var2 " = " << var2 << ", " #var3 " = " << var3 << ", " \
    #var4 " = " << var4 << ", " #var5 " = " << var5 <<std::endl << std::flush;

/** Quit (assert(0)) on the n-th time this line is reached, for the given n.
 * @param n */
#define QUIT_AFTER_N_VISITS(n) { static unsigned counter=0; if (++counter==(n)) {TRACE("User-forced quit."); assert(0);} }

/** Print how many times this line has been reached, everytime it is reached.
 * @param message  message to include in brackets */
#define HOW_MANY_TIMES_HERE(message) { \
    static unsigned counter=1; \
    std::cout << FormDebugHead() << "Num times here (" << message << "): " << counter++ << std::endl << std::flush; }

/** Prints the given message, but only from the n-th time that line is reached, for the given n.
 * @param stuff  what to print
 * @param n */
#define TRACE_FROM_NTH_VISIT(stuff,n) { \
    static unsigned counter=0; \
    if (++counter>=(n)) {TRACE(stuff<<" (visit "<<counter<<")");} }

/** Display a std::vector.
 * @param v */
#define PRINT_VECTOR(v) \
    { std::cout << FormDebugHead() << #v " = {"; \
      for (unsigned _i=0; _i<v.size(); _i++) { \
          std::cout << (_i==0?"":",") << v[_i]; } \
      std::cout << "}" << std::endl << std::flush; }

#endif /*DEBUG_HPP_*/