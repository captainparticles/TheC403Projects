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
    
    typesdef.h
    
 Abstract:
   
    This module defines standard types.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/20/15        Initial Creation

**/

#ifndef __TYPESDEF_H__
#define __TYPESDEF_H__

#include <inttypes.h>

//
// typedefs for the LLP64 model
//

typedef char CHAR, *PCHAR;
typedef unsigned char UCHAR, *PUCHAR;

typedef int16_t SHORT, *PSHORT;
typedef uint16_t USHORT, *PUSHORT;

typedef int32_t INT, *PINT;
typedef INT LONG, *PLONG;
typedef uint32_t UINT, *PUINT;
typedef UINT ULONG, *PULONG;

typedef int64_t LONGLONG, *PLONGLONG;
typedef uint64_t ULONGLONG, *PULONGLONG;

typedef void VOID, *PVOID;

#endif // __TYPESDEF_H__
