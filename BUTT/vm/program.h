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
    
    program.h
    
 Abstract:
   
    This module implements the program read/de-serialization routines.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/24/15        Initial Creation

**/

#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "../Common/progdef.h"
#include "../Common/symdef.h"
#include "../Common/instrdef.h"
#include "../Common/opcodedef.h"
#include "../Common/registerdef.h"
#include <windows.h>
#include <stdio.h>

typedef struct _PROGRAM {
	PROGRAM_HEADER Header;
    //PSHASHMAP FunctionSymbols;
    PFUNCTION_SYMBOL FunctionSymbols;
    ULONG FunctionSymbolsSize;
    PCHAR GlobalData;
    PCHAR Code;
} PROGRAM, *PPROGRAM;

LONG
ProgramRead (
	FILE *ProgramFile,
	PPROGRAM *ProgramOut
	);

#endif // __PROGRAM_H__
