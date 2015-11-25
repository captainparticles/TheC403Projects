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
    
    instruction.c
    
 Abstract:
   
    This module implements the instruction creation routines for the translator.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#include "instruction.h"
#include "register.h"
#include "../Common/registerdef.h"
#include "errors.h"
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

extern int yyerror(char* err);

PINSTRUCTION
InstrMakeArithmetic (
    OPCODES Opcode, 
    PIDENTIFIER_OBJECT OperandL, 
    PIDENTIFIER_OBJECT OperandR,
    PIDENTIFIER_OBJECT Destination
    )
{
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));
    NewInstruction->Opcode = Opcode;
    NewInstruction->Arith.LtRegister = OperandL->Register;
    NewInstruction->Arith.RtRegister = OperandR->Register;
    NewInstruction->Arith.DtRegister = Destination->Register; 
    NewInstruction->Arith.LtRegisterOffset = OperandL->RelOffset;
    NewInstruction->Arith.RtRegisterOffset = OperandR->RelOffset;
    NewInstruction->Arith.DtRegisterOffset = Destination->RelOffset;

    return NewInstruction;   
}

PINSTRUCTION
InstrMakeIndirectDirect (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Source,
    PIDENTIFIER_OBJECT Offset,
    PIDENTIFIER_OBJECT Destination
    )
{
    assert(Opcode == OPC_MOVE || Opcode == OPC_RCOPYD);
    
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));
    NewInstruction->Opcode = Opcode;
    NewInstruction->Indirect.LtRegister = Source->Register;
    if(Source->Register == REG_RCT) {
        
        assert(Offset == NULL);
        
        NewInstruction->Indirect.LtOffsetType = INDIRECT_OFFSET_TYPE_CONSTANT;
        NewInstruction->Indirect.LtOffset = Source->RelOffset;
    } else if(Offset == NULL) {
        NewInstruction->Indirect.LtOffsetType = INDIRECT_OFFSET_TYPE_CONSTANT;
        NewInstruction->Indirect.LtOffset = 0;
    } else if(Offset->Register == REG_RCT) {
        NewInstruction->Indirect.LtOffsetType = INDIRECT_OFFSET_TYPE_CONSTANT;
        NewInstruction->Indirect.LtOffset = Offset->RelOffset;
    } else {
        NewInstruction->Indirect.LtOffsetType = INDIRECT_OFFSET_TYPE_REGISTER;
        NewInstruction->Indirect.LtOffset = Offset->Register;
    }
    
    NewInstruction->Indirect.DtRegister = Destination->Register;
    
    return NewInstruction;
}

PINSTRUCTION
InstrPatchIndirectDirectAddSourceOffset (
    PINSTRUCTION Instruction,
    PIDENTIFIER_OBJECT Offset
    )
{
    assert(Instruction->Opcode == OPC_RCOPYD);
    assert(Instruction->Indirect.LtOffsetType == INDIRECT_OFFSET_TYPE_CONSTANT);
    assert(Offset->Register == REG_RCT);
    
    Instruction->Indirect.LtOffset = Offset->RelOffset;

#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(Instruction);
#endif
    
    return Instruction;
}

PINSTRUCTION
InstrMakeStore (
    OPCODES Opcode, 
    PIDENTIFIER_OBJECT Operand, 
    PIDENTIFIER_OBJECT Destination 
    )
{
    PINSTRUCTION NewInstruction;
    
    if(Destination->Register == REG_RCT) {
        yyerror(ERR_STR_LVALUECONSTANT);
    }
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));
    NewInstruction->Opcode = Opcode;
    NewInstruction->Store.RtRegister = Operand->Register;
    NewInstruction->Store.DtRegister = Destination->Register;
    NewInstruction->Store.AtomicStore = !!(Destination->IsAtomic);
    NewInstruction->Store.RtRegisterOffset = Operand->RelOffset;
    NewInstruction->Store.DtRegisterOffset = Destination->RelOffset;
    
    return NewInstruction;
}

PINSTRUCTION
InstrMakeJump ( 
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Target
    )
{
    (void)Opcode;
    assert(Opcode == OPC_JMP);
    
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));    
    NewInstruction->Opcode = OPC_JMP;
    NewInstruction->Jump.JumpType = JUMP_TYPE_UNCONDITIONAL;
    if(Target != NULL) {
        
        //
        // This instruction is usually generated in two stages, the first one
        // passes NULL since it doesn't know the target. It will be patched later.
        //
        
        DereferenceRegister(Target);
        NewInstruction->Jump.Register = Target->Register;
        NewInstruction->Jump.RegisterOffset = Target->AbsOffset;
    } else {
        NewInstruction->Jump.Register = REG_INV;
        NewInstruction->Jump.RegisterOffset = 0;
    }

#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(NewInstruction);
#endif
    
    return NewInstruction;
}

PINSTRUCTION
InstrPatchJumpTargetRelative (
    PINSTRUCTION Instruction,
    PIDENTIFIER_OBJECT Target
    )
{
    assert(Instruction->Opcode == OPC_JMP);
    
    DereferenceRegister(Target);
    Instruction->Jump.Register = REG_RIP;
    Instruction->Jump.RegisterOffset = Target->RelOffset;
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(Instruction);
#endif
    
    return Instruction;
}

PINSTRUCTION
InstrPatchJumpTargetAbsolute (
    PINSTRUCTION Instruction,
    PIDENTIFIER_OBJECT Target
    )
{
    assert(Instruction->Opcode == OPC_JMP);
    
    DereferenceRegister(Target);
    Instruction->Jump.Register = REG_RCT;
    Instruction->Jump.RegisterOffset = Target->AbsOffset;
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(Instruction);
#endif
    
    return Instruction;
}

PINSTRUCTION
InstrMakeJumpConditional (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Target,
    PIDENTIFIER_OBJECT Check
    )
{
    (void)Opcode;
    assert(Opcode == OPC_JMPZ);
    
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));
    DereferenceRegister(Check);
    NewInstruction->Opcode = OPC_JMPZ;
    NewInstruction->Jump.JumpType = JUMP_TYPE_CONDITIONAL;
    NewInstruction->Jump.ZeroRegister = Check->Register;
    if(Target != NULL) {
        
        //
        // This instruction is usually generated in two stages, the first one
        // passes NULL since it doesn't know the target. It will be patched later.
        //
        
        DereferenceRegister(Target);
        NewInstruction->Jump.Register = Target->Register;
        NewInstruction->Jump.RegisterOffset = Target->RelOffset;
    } else {
        NewInstruction->Jump.Register = REG_INV;
        NewInstruction->Jump.RegisterOffset = 0;
    }
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(NewInstruction);
#endif
    
    return NewInstruction;
}

PINSTRUCTION
InstrPatchJumpConditionalAddTargetRelative (
    PINSTRUCTION Instruction,
    PIDENTIFIER_OBJECT Target
    )
{
    assert(Instruction->Opcode == OPC_JMPZ);
    
    DereferenceRegister(Target);
    Instruction->Jump.Register = REG_RIP;
    Instruction->Jump.RegisterOffset = Target->RelOffset;
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(Instruction);
#endif
    
    return Instruction;
}

PINSTRUCTION
InstrMakeCall (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Function
    )
{
    (void)Opcode;
    assert(Opcode == OPC_CALLNORM);
    
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));    
    NewInstruction->Opcode = OPC_CALLNORM;
    NewInstruction->Jump.JumpType = JUMP_TYPE_UNCONDITIONAL;
    NewInstruction->Jump.Register = REG_RCT;
    NewInstruction->Jump.RegisterOffset = Function->AbsOffset;
    
    return NewInstruction;
}

PINSTRUCTION
InstrPatchNormalCallToParallelSync (
    OPCODES Opcode,
    PINSTRUCTION Instruction
    )
{
    assert(Instruction->Opcode == OPC_CALLNORM);
    
    Instruction->Opcode = OPC_CALLPLLS;
    return Instruction;
}
    
PINSTRUCTION
InstrPatchNormalCallToParallelAsync (
    OPCODES Opcode,
    PINSTRUCTION Instruction
    )
{
    assert(Instruction->Opcode == OPC_CALLNORM);
    
    Instruction->Opcode = OPC_CALLPLLA;
    return Instruction;
}

PINSTRUCTION
InstrMakeCallParallelSync (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Function
    )
{
    (void)Opcode;
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));    
    NewInstruction->Opcode = OPC_CALLPLLS;
    NewInstruction->Jump.JumpType = JUMP_TYPE_UNCONDITIONAL;
    NewInstruction->Jump.Register = REG_RCT;
    NewInstruction->Jump.RegisterOffset = Function->AbsOffset;
    
    return NewInstruction;
}

PINSTRUCTION
InstrMakeCallParallelAsync(
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Function
    )
{
    (void)Opcode;
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));    
    NewInstruction->Opcode = OPC_CALLPLLA;
    NewInstruction->Jump.JumpType = JUMP_TYPE_UNCONDITIONAL;
    NewInstruction->Jump.Register = REG_RCT;
    NewInstruction->Jump.RegisterOffset = Function->AbsOffset;
    
    return NewInstruction;
}

PINSTRUCTION
InstrMakeReturn (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT StackCleanup
    )
{
    (void)Opcode;
    assert(Opcode == OPC_RETURN);
    assert(StackCleanup->Register == REG_RCT);
    
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));    
    NewInstruction->Opcode = OPC_RETURN;
    NewInstruction->Return.StackCleanup = StackCleanup->RelOffset;
    
    return NewInstruction;
}

PINSTRUCTION
InstrMakeStackPush(
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Value
    )
{
    (void)Opcode;
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));
    NewInstruction->Opcode = OPC_PUSH;
    NewInstruction->Stack.Register = Value->Register;
    NewInstruction->Stack.RegisterOffset = Value->RelOffset;
    
    return NewInstruction;
}

PINSTRUCTION
InstrMakeStackPop( 
    OPCODES Opcode,
    PIDENTIFIER_OBJECT Location
    )
{
    (void)Opcode;
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));
    NewInstruction->Opcode = OPC_POP;
    NewInstruction->Stack.Register = Location->Register;
    NewInstruction->Stack.RegisterOffset = 0;
    
    return NewInstruction;
}

PINSTRUCTION
InstrMakeIoRead (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT ReadCount
    )
{
    (void)Opcode;
    assert(Opcode == OPC_READ);
    assert(ReadCount->Register == REG_RCT);
    
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));    
    NewInstruction->Io.Opcode = OPC_READ;
    NewInstruction->Io.PopCount = ReadCount->AbsOffset;
    
    return NewInstruction;    
}

PINSTRUCTION
InstrMakeIoPrint (
    OPCODES Opcode,
    PIDENTIFIER_OBJECT WriteCount
    )
{
    (void)Opcode;
    assert(Opcode == OPC_PRINT);
    assert(WriteCount->Register == REG_RCT);
    
    PINSTRUCTION NewInstruction;
    
    NewInstruction = malloc(sizeof(INSTRUCTION));
    memset(NewInstruction, 0, sizeof(INSTRUCTION));    
    NewInstruction->Io.Opcode = OPC_PRINT;
    NewInstruction->Io.PopCount = WriteCount->AbsOffset;
    
    return NewInstruction;    
}
