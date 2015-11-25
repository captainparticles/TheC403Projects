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
    
    instruction.h
    
 Abstract:
    
    This module implements the routine definitions for the instruction
    creation.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include "../Common/instrdef.h"
#include "objtypes.h"
#include "opcodes.h"
#include "../../utils/inc/squeue.h"

PINSTRUCTION
InstrMakeArithmetic (
    OPCODES Opcode, 
    PIDENTIFIER_OBJECT OperandL, 
    PIDENTIFIER_OBJECT OperandR,
    PIDENTIFIER_OBJECT Destination
    );
    
PINSTRUCTION
InstrMakeIndirectDirect (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Source,
    PIDENTIFIER_OBJECT Offset,
    PIDENTIFIER_OBJECT Destination
    );
    
PINSTRUCTION
InstrPatchIndirectDirectAddSourceOffset (
    PINSTRUCTION Instruction,
    PIDENTIFIER_OBJECT Offset
    );
    
PINSTRUCTION
InstrMakeStore (
    OPCODES Opcode, 
    PIDENTIFIER_OBJECT Operand, 
    PIDENTIFIER_OBJECT Destination 
    );

PINSTRUCTION
InstrMakeJump ( 
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Target
    );
    
PINSTRUCTION
InstrPatchJumpTargetRelative (
    PINSTRUCTION Instruction,
    PIDENTIFIER_OBJECT Target
    );
    
PINSTRUCTION
InstrPatchJumpTargetAbsolute (
    PINSTRUCTION Instruction,
    PIDENTIFIER_OBJECT Target
    );

PINSTRUCTION
InstrMakeJumpConditional (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Target,
    PIDENTIFIER_OBJECT Check
    );
    
PINSTRUCTION
InstrPatchJumpConditionalAddTargetRelative (
    PINSTRUCTION Instruction,
    PIDENTIFIER_OBJECT Target
    );

PINSTRUCTION
InstrMakeCall (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Function
    );
    
PINSTRUCTION
InstrPatchNormalCallToParallelSync (
    OPCODES Opcode,
    PINSTRUCTION Instruction
    );
    
PINSTRUCTION
InstrPatchNormalCallToParallelAsync (
    OPCODES Opcode,
    PINSTRUCTION Instruction
    );

PINSTRUCTION
InstrMakeCallParallelSync (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Function
    );

PINSTRUCTION
InstrMakeCallParallelAsync (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Function
    );
    
PINSTRUCTION
InstrMakeReturn (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT StackCleanup
    );

PINSTRUCTION
InstrMakeStackPush (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Value
    );

PINSTRUCTION
InstrMakeStackPop ( 
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Location
    );
    
PINSTRUCTION
InstrMakeIoRead (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT ReadCount
    );

PINSTRUCTION
InstrMakeIoPrint (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT ReadCount
    );
    
#endif // __INSTRUCTION_H__
