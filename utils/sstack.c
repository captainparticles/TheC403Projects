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
    
    sstack.c
    
 Abstract:
   
    This module implements a simple stack.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      10/10/15

 Revision:
 
    10/10/15        Initial Creation

**/

#include "inc/sstack.h"
#include <stdlib.h>

typedef struct _STACK_NODE {
    
    void *Data;
    struct _STACK_NODE *Next;
    struct _STACK_NODE *Prev;
    
} STACK_NODE, *PSTACK_NODE;

typedef struct _SSTACK {
    
    PSTACK_NODE Head;
    PSTACK_NODE Tail;
    size_t Size;
    
} SSTACK, *PSSTACK;

void 
SStackInitialize(PSSTACK *Head)
{
    PSSTACK NewStack;
    
    *Head = NULL;
    NewStack = malloc(sizeof(SSTACK));
    if(NewStack == NULL) {
        return;
    }
    
    NewStack->Head = NULL;
    NewStack->Tail = NULL;
    NewStack->Size = 0;
    *Head = NewStack;
}

void 
SStackPush(PSSTACK Head, void* Data)
{
    PSTACK_NODE NewNode;
    
    NewNode = malloc(sizeof(STACK_NODE));
    NewNode->Data = Data;
    NewNode->Next = Head->Head;
    NewNode->Prev = NULL;
    
    if (Head->Size == 0) {
        Head->Tail = NewNode;
    } else {
        Head->Head->Prev = NewNode;
    }
    
    Head->Head = NewNode;
    Head->Size = Head->Size + 1;    
}

void* 
SStackPop(PSSTACK Head)
{
    PSTACK_NODE CurrHeadNode;
    PSTACK_NODE NewHeadNode;
    void* Data;
    
    CurrHeadNode = Head->Head;
    Data = CurrHeadNode->Data;
    if(Head->Size == 1) {
        Head->Head = NULL;
        Head->Tail = NULL;
    } else {
        NewHeadNode = CurrHeadNode->Next;
        NewHeadNode->Prev = NULL;
        Head->Head = NewHeadNode;
    }
    
    Head->Size = Head->Size - 1;
    free(CurrHeadNode);
    return Data;
}

void* 
SStackTop(PSSTACK Head)
{
    return Head->Head->Data;
}

size_t 
SStackSize(PSSTACK Head)
{
    return Head->Size;
}
