/**

 Copyright 2015 Omar Carey.
 
 This file is part of The C403 Projects.

 The C403 Projects is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 The C403 Projects is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with The C403 Projects.  If not, see <http://www.gnu.org/licenses/>.
 
 Translation Unit:
    
    sstack.h
    
 Abstract:
   
    This module defines a simple stack.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      10/10/15

 Revision:
 
    10/10/15        Initial Creation

**/

#ifndef _SSTACK_H_
#define _SSTACK_H_

#include <stdlib.h>

typedef struct _SSTACK SSTACK, *PSSTACK;

void SStackInitialize(PSSTACK *Head);
void SStackPush(PSSTACK Head, void* Data);
void* SStackPop(PSSTACK Head);
void* SStackTop(PSSTACK Head);
size_t SStackSize(PSSTACK Head); 

#endif // _SSTACK_H_