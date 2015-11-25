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
    
    opcodedef.h
    
 Abstract:
    
    This module defines the opcodes for the translator and VM.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/19/15        Initial Creation

**/

#ifndef __OPCODE_DEF__
#define __OPCODE_DEF__

//
// 6 bit opcodes. 64 max.
//

typedef enum _OPCODES {
    
    //
    // Arithmetic operations
    //
    
    OPC_ADDI        = 1,
    OPC_ADDF        = 2,
    OPC_SUBI        = 3,
    OPC_SUBF        = 4,
    OPC_MULI        = 5,
    OPC_MULF        = 6,
    OPC_DIVI        = 7,
    OPC_DIVF        = 8,
    
    //
    // Indirect & Direct addressing
    //
    
    OPC_MOVE        = 9,
    OPC_RCOPYD      = 10,
    
    //
    // Binary operations
    //
    
    OPC_XOR         = 11,
    OPC_OR          = 12,
    OPC_AND         = 13,
    OPC_NOT         = 14,
    
    //
    // Logical operations
    //
    
    OPC_LOR         = 15,
    OPC_LAND        = 16,
    
    //
    // Comparison operations
    //
    
    OPC_EQ          = 17,
    OPC_NEQ         = 18,
    OPC_LT          = 19,
    OPC_GT          = 20,
    OPC_LTE         = 21,
    OPC_GTE         = 22,
    
    //
    // Assignments
    //
    
    OPC_STRI8       = 23,
    OPC_STRU8       = 24,
    OPC_STRI16      = 25,
    OPC_STRU16      = 26,
    OPC_STRI32      = 27,
    OPC_STRU32      = 28,
    OPC_STRF        = 29,
    OPC_STRTH       = 30,
    
    //
    // Jumps & Branches & Calls
    //
    
    OPC_JMP         = 31,
    OPC_JMPZ        = 32,
    OPC_CALLNORM    = 33,
    OPC_CALLPLLS    = 34,
    OPC_CALLPLLA    = 35,
    OPC_RETURN      = 36,
    
    //
    // Register relative arithmetic
    //
    
    OPC_PUSH        = 37,
    OPC_POP         = 38,
    
    //
    // I/O
    //
    
    OPC_PRINT       = 39,
    OPC_READ        = 40,
    
    OPC_ERR         = 63
} OPCODES;

#endif // __OPCODE_DEF__