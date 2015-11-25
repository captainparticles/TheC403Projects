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
    
    squeue.c
    
 Abstract:
   
    This module implements a simple queue.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      10/10/15

 Revision:
 
    10/10/15        Initial Creation

**/

#include "inc/squeue.h"
#include <stdlib.h>

typedef struct _QUEUE_NODE {
    
    void **Data;
    struct _QUEUE_NODE *Next;
    struct _QUEUE_NODE *Prev;
    
} QUEUE_NODE, *PQUEUE_NODE;

typedef struct _SQUEUE {
    
    PQUEUE_NODE Head;
    PQUEUE_NODE Tail;
    size_t Size;
    
} SQUEUE, *PSQUEUE;

void 
SQueueInitialize(PSQUEUE *Head)
{
    PSQUEUE NewQueue;
    
    *Head = NULL;
    NewQueue = malloc(sizeof(SQUEUE));
    if(NewQueue == NULL) {
        return;
    }
    
    NewQueue->Head = NULL;
    NewQueue->Tail = NULL;
    NewQueue->Size = 0;
    *Head = NewQueue;
}

void
SQueuePush(PSQUEUE Head, void* Data)
{
    PQUEUE_NODE NewNode;
    
    NewNode = malloc(sizeof(QUEUE_NODE));
    NewNode->Data = Data;
    NewNode->Next = NULL;
    NewNode->Prev = Head->Tail;
    
    if (Head->Size == 0) {
        Head->Head = NewNode;
    } else {
        Head->Tail->Next = NewNode;
    }
    
    Head->Tail = NewNode;
    Head->Size = Head->Size + 1;    
}

void* 
SQueuePop(PSQUEUE Head)
{
    PQUEUE_NODE CurrHeadNode;
    PQUEUE_NODE NewHeadNode;
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
SQueueTop(PSQUEUE Head)
{
    return Head->Head->Data;
}

void* 
SQueueTopNode(PSQUEUE Head)
{
    return Head->Head;
}

void* 
SQueueDataFromNode(void* Node)
{
    return ((PQUEUE_NODE)Node)->Data;
}

void* 
SQueueNextFromNode(void* Node)
{
    return ((PQUEUE_NODE)Node)->Next;
}

void
SQueueIterate(PSQUEUE Head, SQueueIterator Iter)
{
    PQUEUE_NODE Node;
    unsigned CurrentLength;
    
    CurrentLength = 0;
    Node = Head->Head;
    while(Node != NULL) {
        Iter(Node->Data, CurrentLength);
        Node = Node->Next;
        CurrentLength = CurrentLength + 1;
    }
}

size_t 
SQueueSize(PSQUEUE Head)
{
    return Head->Size;
}
