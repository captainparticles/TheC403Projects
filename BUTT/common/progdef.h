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
    
    progdef.h
    
 Abstract:
    
    This module defines the program header for the translator and VM.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/19/15        Initial Creation

**/

#ifndef __PROGDEF_H__
#define __PROGDEF_H__

#include <assert.h>
#include <inttypes.h>

#define HEADER_MAGIC_NUMBER     0xC403
#define COMPILER_VERSION_MAJOR  0x0001
#define COMPILER_VERSION_MINOR  0x0000
#define HEADER_SIZE_BYTES       0x40

typedef struct _PROGRAM_HEADER {
    uint16_t MagicNumber;                       // 0x02
    uint16_t VersionMajor;                      // 0x04
    uint16_t VersionMinor;                      // 0x06
    uint16_t StackAlignment;                    // 0x08
    uint16_t Reserved1;                         // 0x0A
    uint16_t Reserved2;                         // 0x0C
    uint32_t StackTop;                          // 0x10
    uint32_t DataStart;                         // 0x14
    uint32_t CodeStart;                         // 0x18
    uint32_t StackSize;                         // 0x1C
    uint32_t DataSize;                          // 0x20
    uint32_t CodeSize;                          // 0x24
    uint32_t SymbolSize;                        // 0x28
    uint32_t SymbolBinaryLocation;              // 0x2C
    uint32_t CodeBinaryLocation;                // 0x30
    uint32_t Reserved3;                         // 0x34
    uint32_t Reserved4;                         // 0x38
    uint32_t Reserved5;                         // 0x3C
    uint32_t Reserved6;                         // 0x40
} PROGRAM_HEADER, *PPROGRAM_HEADER;

static_assert(sizeof(PROGRAM_HEADER) == HEADER_SIZE_BYTES,
              "sizeof(PROGRAM_HEADER) exceeds HEADER_SIZE_BYTES");

#endif // __PROGDEF_H__