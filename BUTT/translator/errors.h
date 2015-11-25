/**

 Copyright 2015 Omar Carey.
 
 This file is part of BUTT.

 BUTT is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 BUTT is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with BUTT.  If not, see <http://www.gnu.org/licenses/>.
 
 Translation Unit:
    
    debug.h
    
 Abstract:
    
    This module defines the error codes and strings used upon encountering a
    fault during the translation process.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#ifndef __ERRORS_H__
#define __ERRORS_H__

#define ERR_STR_FILEINOPEN      "Unable to open input file."
#define ERR_STR_FILEOUTOPEN     "Unable to open output file."
#define ERR_STR_NULOPENFAIL         "Unable to open NUL stream."
#define ERR_STR_NOMEM           "Out of memory :("
#define ERR_STR_REDECLARED      "Identifier redeclared."
#define ERR_STR_UNDECLARED      "Identifier undeclared."
#define ERR_STR_FLOATUSED       "Float support not enabled."
#define ERR_STR_NOATOMICARR     "Atomic arrays are not supported."
#define ERR_STR_PROGRAMINIT     "Error initializing translator."
#define ERR_STR_INVALIDINSTR    "Invalid instruction."
#define ERR_STR_NOREGISTERS     "Out of registers. This is uh.. bad, like, really bad."
#define ERR_STR_NOTARRAY        "Identifier is not an array."
#define ERR_STR_NOTPOSARRAY     "Array size must be greater than 0."
#define ERR_STR_NOTFUNC         "Identifier is not a function."
#define ERR_STR_LVALUECONSTANT  "lvalue is a constant."
#define ERR_STR_INCORRECTMAIN   "illegal main declaration. signature must be int main(int)."
#define ERR_STR_NOMAINFUNCTION  "int main(int) was not defined."
#define ERR_STR_PARAMMISMATCH   "Parameter mismatch."
#define ERR_STR_PARAMTYPEERR    "Parameter type mismatch."
#define ERR_STR_EXCESSPARAM     "Parameter count for function has been exceeded."

#endif // __ERRORS_H__