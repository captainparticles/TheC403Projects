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
    
    symdef.h
    
 Abstract:
    
    This module defines the instructions for the translator and VM.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/19/15        Initial Creation

**/

#ifndef __INSTRDEF_H__
#define __INSTRDEF_H__

#include <assert.h>
#include <inttypes.h>

#define JUMP_TYPE_UNCONDITIONAL 0  
#define JUMP_TYPE_CONDITIONAL   1

#define INDIRECT_OFFSET_TYPE_REGISTER   0
#define INDIRECT_OFFSET_TYPE_CONSTANT   1

//
// 64 bit instructions.
//

typedef struct _INSTRUCTION {
    union {
        
        struct {
            uint64_t Opcode                : 6;
            uint64_t                       : 58;
        };
        
        //
        // Arithmetic operations
        //
        
        struct {
            uint64_t Opcode                : 6;
            uint64_t LtRegister            : 5;
            uint64_t RtRegister            : 5;
            uint64_t DtRegister            : 5;
            int64_t  LtRegisterOffset      : 14;
            int64_t  RtRegisterOffset      : 14;
            int64_t  DtRegisterOffset      : 14;
            uint64_t                       : 1;
        } Arith;
        
        //
        // Indirect memory addressing
        //
        
        struct {
            uint64_t Opcode                : 6;
            uint64_t LtRegister            : 5;
            uint64_t DtRegister            : 5;
            uint64_t LtOffsetType          : 1;
            int64_t  LtOffset              : 32;
            uint64_t                       : 15;
        } Indirect;
        
        //
        // Store 
        //
        
        struct {
            uint64_t Opcode                : 6;    
            uint64_t RtRegister            : 5;
            uint64_t DtRegister            : 5;
            uint64_t AtomicStore           : 1;
            int64_t  RtRegisterOffset      : 23;
            int64_t  DtRegisterOffset      : 23;
            uint64_t                       : 1;
        } Store;
        
        //
        // Jumps & Branches
        //
        
        struct {  
            uint64_t Opcode                 : 6;
            uint64_t JumpType               : 1;
            uint64_t Register               : 5;
            int64_t  RegisterOffset         : 32;
            uint64_t ZeroRegister           : 5;
            uint64_t                        : 15;
        } Jump;
        
        //
        // Return
        //
        
        struct {
            uint64_t Opcode                 : 6;
            uint64_t StackCleanup           : 32;
            uint64_t                        : 26;
        } Return;
        
        //
        // Stack push & pop
        //
        
        struct {
            uint64_t Opcode                 : 6;
            uint64_t Register               : 5;
            int64_t  RegisterOffset         : 32;
            uint64_t                        : 21;
        } Stack;
        
        //
        // I/O
        //
        
        struct {
            uint64_t Opcode                 : 6;
            uint64_t PopCount               : 32;
            uint64_t                        : 26;
        } Io;
    };   
} INSTRUCTION, *PINSTRUCTION;

static_assert(sizeof(INSTRUCTION) == 8, "sizeof(INSTRUCTION) isn't 8.");

#endif // __INSTRDEF_H__