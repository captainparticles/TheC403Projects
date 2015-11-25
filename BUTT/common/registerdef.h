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
    
    registerdef.h
    
 Abstract:
    
    This module defines the registers for the translator and VM.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#ifndef __REGISTERDEF_H__
#define __REGISTERDEF_H__

#define WORKING_REGISTER_COUNT  8
#define INDEX_REGISTER_COUNT    4

#define IS_REGISTER_SPECIAL(R)   \
    ((R) == REG_RIP ||           \
     (R) == REG_RGD ||           \
     (R) == REG_RST ||           \
     (R) == REG_RSB ||           \
     (R) == REG_RRV ||           \
     (R) == REG_RFN ||           \
     (R) == REG_INV)

#define IS_REGISTER_INDEX(R)     \
    ((R) == REG_RGD ||           \
     (R) == REG_RST ||           \
     (R) == REG_RSB ||           \
     (R) == REG_IX0 ||           \
     (R) == REG_IX1 ||           \
     (R) == REG_IX2 ||           \
     (R) == REG_IX3)
    
#define IS_REGISTER_INDEX_IX(R)  \
    ((R) == REG_IX0 ||           \
     (R) == REG_IX1 ||           \
     (R) == REG_IX2 ||           \
     (R) == REG_IX3)
    
#define IS_REGISTER_WORKING(R)   \
    ((R) == REG_RT0 ||           \
     (R) == REG_RT1 ||           \
     (R) == REG_RT2 ||           \
     (R) == REG_RT3 ||           \
     (R) == REG_RT4 ||           \
     (R) == REG_RT5 ||           \
     (R) == REG_RT6 ||           \
     (R) == REG_RT7)

typedef enum _REGISTER {
    REG_RIP = 0,        // Instruction Pointer
    REG_RGD = 1,        // Global Data Pointer      [INDEX]
    REG_RST = 2,        // Stack Top Pointer        [INDEX]
    REG_RSB = 3,        // Stack Bottom Pointer     [INDEX]
    REG_RRV = 4,        // Return value
    REG_RCT = 5,        // Constant Register
    
    REG_IX0 = 6,        // General Index register 0 [INDEX]
    REG_IX1 = 7,        // General Index register 1 [INDEX]
    REG_IX2 = 8,        // General Index register 2 [INDEX]
    REG_IX3 = 9,        // General Index register 3 [INDEX]
    
    REG_RT0 = 10,       // Scrap register 0
    REG_RT1 = 11,       // Scrap register 1
    REG_RT2 = 12,       // Scrap register 2
    REG_RT3 = 13,       // Scrap register 3
    REG_RT4 = 14,       // Scrap register 4
    REG_RT5 = 15,       // Scrap register 5
    REG_RT6 = 16,       // Scrap register 6
    REG_RT7 = 17,       // Scrap register 7  
    
    REG_MAX = 18,       // Error register above equal to this

    REG_RFN = 19,       // Pseudo-Register for function detection
    REG_INV = 20,       // Pseudo-Register for invalid register
} REGISTER;

extern const char * const _REGISTER_NAMES[];

#endif // __REGISTERDEF_H__