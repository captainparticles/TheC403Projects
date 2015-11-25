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
    
    memory_inl.h
    
 Abstract:
   
    This module implements the inline memory/stack management routines.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/24/15        Initial Creation

**/

#ifndef __MEMORY_INL_H__
#define __MEMORY_INL_H__

#include <windows.h>
#include "program.h"

extern PPROGRAM GProgram;

extern ULONG GCodePointerBias;
extern ULONG GDataPointerBias;
extern ULONG GStackPointerBias;

inline
VOID
MemRegisterValue (
    PCHAR Stack,
    PREGISTER_SET RegisterSet, 
    ULONG Register,
    LONG RegisterOffset,
    PLONG Value
    )
    
/*

 Routine description:
 
    This inline routine obtains the value of a provided register according to
    the register type. This function may make a memory access in the case the
    register turns out be an index.
    
 Arguments:
 
    Stack - A pointer to the execution threads stack.
    
    RegisterSet - A pointer to the active register set.
    
    Register - The register whose value is to be obtained
    
    RegisterOffset - The offset into the register.
    
    Value - A pointer receiving the register value.
    
 Return value:
 
    VOID.

*/
    
{
    LONG StackOffset;
    
    if(Register == REG_RGD) {
        memcpy(Value, 
               GProgram->GlobalData+RegisterOffset, 
               GProgram->Header.StackAlignment);
        
    } else if(IS_REGISTER_INDEX(Register)) {
        StackOffset = RegisterSet->Register[Register];
        StackOffset = StackOffset - GStackPointerBias;
        StackOffset = StackOffset + RegisterOffset;
        memcpy(Value, 
               Stack+StackOffset, 
               GProgram->Header.StackAlignment);
    
    } else if(Register == REG_RCT) {
        *Value = RegisterOffset;
    } else {
        *Value = RegisterSet->Register[Register];
    }
}

inline
VOID
MemRegisterSet (
    PCHAR Stack,
    ULONG Register,
    LONG Value
    )
{
    
}

#endif // __MEMORY_INL_H__
