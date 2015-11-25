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
    
    exec.h
    
 Abstract:
   
    This module defines the instruction execution routines.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/24/15        Initial Creation

**/

#ifndef __EXEC_H__
#define __EXEC_H__

#include <windows.h>
#include "../Common/def.h"
#include "../../utils/inc/shashmap.h"
#include "../../utils/inc/sstack.h"
#include "../../utils/inc/squeue.h"

typedef struct _REGISTER_SET {
    ULONG Register[REG_MAX-1];
} REGISTER_SET, *PREGISTER_SET;

typedef struct _THREAD_EXECUTION_DATA {
    PSSTACK RegisterSetStack;
    PREGISTER_SET ActiveRegisterSet;
    PCHAR ThreadStack;
} THREAD_EXECUTION_DATA, *PTHREAD_EXECUTION_DATA;

typedef struct _THREAD_CREATION_DATA {
    PREGISTER_SET RegisterSet;
    PCHAR MiniStack;
    ULONG MiniStackSize;
    ULONG JumpAddress;
} THREAD_CREATION_DATA, *PTHREAD_CREATION_DATA;

VOID
ExecPrimeProgram (
    VOID
    );

#endif // __EXEC_H__
