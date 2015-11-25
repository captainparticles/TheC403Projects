/**

 Based off c_hashmap by Pete Warden. https://github.com/petewarden/c_hashmap
 As allowed in his license, I am distributing this modified version as GPLv2.
 
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
    
    shashmap.h
    
 Abstract:
   
    This module defines a simple hashmap, keyed with a char*
    
 Author:
    
    Omar Carey      Carey403@gmail.com      10/10/15

 Revision:
 
    10/10/15        Initial Creation

**/

#ifndef __SHASHMAP_H__
#define __SHASHMAP_H__

#include <stdlib.h>

typedef enum _SHASHMAP_RESULT {
    SHASHMAP_MISSING = -3,
    SHASHMAP_FULL = -2,
    SHASHMAP_OMEM = -1,
    SHASHMAP_OK = 0
} SHASHMAP_RESULT;

typedef int (*HashMapIteratorFunc)(void*, void*);
typedef struct _SHASHMAP SHASHMAP, *PSHASHMAP;

void 
SHashMapInitialize(PSHASHMAP *Map);

SHASHMAP_RESULT
SHashMapIterate(PSHASHMAP Map, HashMapIteratorFunc f, void* item);

SHASHMAP_RESULT
SHashMapInsert(PSHASHMAP Map, char* Key, void* Value);

SHASHMAP_RESULT
SHashMapGet(PSHASHMAP Map, char* Key, void** Value);

SHASHMAP_RESULT
SHashMapRemove(PSHASHMAP Map, char* Key);

void 
SHashMapDestroy(PSHASHMAP Map);

size_t 
SHashMapSize(PSHASHMAP Map);

#endif // __SHASHMAP_H__
