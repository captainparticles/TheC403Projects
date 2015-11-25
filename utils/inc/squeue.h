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
    
    squeue.h
    
 Abstract:
   
    This module defines a simple queue.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      10/10/15

 Revision:
 
    10/10/15        Initial Creation

**/

#ifndef __SQUEUE_H__
#define __SQUEUE_H__

#include <stdlib.h>

typedef void (*SQueueIterator)(void*, unsigned);

typedef struct _SQUEUE SQUEUE, *PSQUEUE;

void SQueueInitialize(PSQUEUE *Head);
void SQueuePush(PSQUEUE Head, void* Data);
void* SQueuePop(PSQUEUE Head);
void* SQueueTop(PSQUEUE Head);
void* SQueueTopNode(PSQUEUE Head);
void* SQueueDataFromNode(void* Node);
void* SQueueNextFromNode(void* Node);
void SQueueIterate(PSQUEUE Head, SQueueIterator Iter);
size_t SQueueSize(PSQUEUE Head); 

#endif // __SQUEUE_H__
