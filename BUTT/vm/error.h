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
    
    error.h
    
 Abstract:
   
    This module defines the error management routines and messages.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/24/15        Initial Creation

**/

#ifndef __ERROR_H__
#define __ERROR_H__

#define ERR_STR_NOMEM               "Out of memory."
#define ERR_STR_NULOPENFAIL         "Unable to open NUL stream."
#define ERR_STR_NOINPUTFILE         "Opening input file."
#define ERR_STR_TLSALLOCFAIL        "Allocating TLS index."
#define ERR_STR_INVALIDINSTR        "Invalid instruction."
#define ERR_STR_ONLYRCOPYD          "Only RCOPYD is defined for Indirect type instruction."

void 
VmFatal (
    char* Error
    );

#endif // __ERROR_H__
