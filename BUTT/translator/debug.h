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
    
    This module defines the debugging routines for the translator.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "../Common/progdef.h"
#include "instruction.h"
#include "opcodes.h"
#include "objtypes.h"
#include "register.h"
#include "../../utils/inc/squeue.h"
#include "../../utils/inc/sstack.h"
#include "../../utils/inc/shashmap.h"

void
DebugPrettyPrintInstruction (
    PINSTRUCTION Instruction
    );
    
void
DebugPrintProgram (
    PSQUEUE Instructions
    );
    
void
DebugPrettyPrintProgramHeader (
    PPROGRAM_HEADER Header
    );

#endif // __DEBUG_H__
