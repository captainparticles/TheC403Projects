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
    
    debug.h
    
 Abstract:
    
    This module implements the debugging routines for the translator.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _ABS(X) ((X) < 0 ? -(X) : (X))
#define _SGN(X) ((X) < 0 ? '-' : '+')

void
DebugPrettyPrintInstructionArithmetic (
    PINSTRUCTION Instruction
    )
{
    char OpcodeString[16];
    
    switch(Instruction->Opcode) {
    case OPC_ADDI:
        sprintf(OpcodeString, "%-8s", "ADDI");
        break;
    case OPC_ADDF:
        sprintf(OpcodeString, "%-8s", "ADDF");
        break;
    case OPC_SUBI:
        sprintf(OpcodeString, "%-8s", "SUBI");
        break;
    case OPC_SUBF:
        sprintf(OpcodeString, "%-8s", "SUBI");
        break;
    case OPC_MULI:
        sprintf(OpcodeString, "%-8s", "MULI");
        break;
    case OPC_MULF:
        sprintf(OpcodeString, "%-8s", "MULF");
        break;
    case OPC_DIVI:
        sprintf(OpcodeString, "%-8s", "DIVI");
        break;
    case OPC_DIVF:
        sprintf(OpcodeString, "%-8s", "DIVF");
        break;
    case OPC_XOR:
        sprintf(OpcodeString, "%-8s", "XOR");
        break;
    case OPC_OR:
        sprintf(OpcodeString, "%-8s", "OR");
        break;
    case OPC_AND:
        sprintf(OpcodeString, "%-8s", "AND");
        break;
    case OPC_NOT:
        sprintf(OpcodeString, "%-8s", "NOT");
        break;
    case OPC_LOR:
        sprintf(OpcodeString, "%-8s", "LOR");
        break;
    case OPC_LAND:
        sprintf(OpcodeString, "%-8s", "LAND");
        break;
    case OPC_EQ:
        sprintf(OpcodeString, "%-8s", "EQ");
        break;
    case OPC_NEQ:
        sprintf(OpcodeString, "%-8s", "NEQ");
        break;
    case OPC_LT:
        sprintf(OpcodeString, "%-8s", "LT");
        break;
    case OPC_GT:
        sprintf(OpcodeString, "%-8s", "GT");
        break;
    case OPC_LTE:
        sprintf(OpcodeString, "%-8s", "LTE");
        break;
    case OPC_GTE:
        sprintf(OpcodeString, "%-8s", "GTE");
        break;
    }
    
    printf("%s %s%s%+d%s %s%s%+d%s %s%s%+d%s\n",
           OpcodeString,                                                    // %s
           
           IS_REGISTER_INDEX(Instruction->Arith.LtRegister) ? "[" : "",     // %s
           _REGISTER_NAMES[Instruction->Arith.LtRegister],                  // %s
           (signed)Instruction->Arith.LtRegisterOffset,                     // %d
           IS_REGISTER_INDEX(Instruction->Arith.LtRegister) ? "]" : "",     // %s
           
           IS_REGISTER_INDEX(Instruction->Arith.RtRegister) ? "[" : "",     // %s
           _REGISTER_NAMES[Instruction->Arith.RtRegister],                  // %s
           (signed)Instruction->Arith.RtRegisterOffset,                     // %d
           IS_REGISTER_INDEX(Instruction->Arith.RtRegister) ? "]" : "",     // %s
           
           IS_REGISTER_INDEX(Instruction->Arith.DtRegister) ? "[" : "",     // %s
           _REGISTER_NAMES[Instruction->Arith.DtRegister],                  // %s
           (signed)Instruction->Arith.DtRegisterOffset,                     // %d
           IS_REGISTER_INDEX(Instruction->Arith.DtRegister) ? "]" : "");    // %s
}

void
DebugPrettyPrintInstructionMove (
    PINSTRUCTION Instruction
    )
{
    char OpcodeString[16];
    unsigned Offset;
    char OffsetSign;
    
    switch(Instruction->Opcode) {
    case OPC_MOVE:
        sprintf(OpcodeString, "%-8s", "MOVE");
        break;
    case OPC_RCOPYD:
        sprintf(OpcodeString, "%-8s", "RCOPYD");
    }
    
    if(Instruction->Indirect.LtOffsetType == INDIRECT_OFFSET_TYPE_REGISTER) {
        printf("%s %s%s+%s%s %s\n",
               OpcodeString,
               
               Instruction->Opcode == OPC_MOVE ? "[" : "",
               _REGISTER_NAMES[Instruction->Indirect.LtRegister],
               _REGISTER_NAMES[Instruction->Indirect.LtOffset],
               Instruction->Opcode == OPC_MOVE ? "]" : "",
               
               _REGISTER_NAMES[Instruction->Indirect.DtRegister]);
    } else {
        Offset = _ABS((signed)Instruction->Indirect.LtOffset);
        OffsetSign = _SGN((signed)Instruction->Indirect.LtOffset);
        printf("%s %s%s%c0x%X%s %s\n",
               OpcodeString,
               
               Instruction->Opcode == OPC_MOVE ? "[" : "",
               _REGISTER_NAMES[Instruction->Indirect.LtRegister],
               OffsetSign,
               Offset,
               Instruction->Opcode == OPC_MOVE ? "]" : "",
               
               _REGISTER_NAMES[Instruction->Indirect.DtRegister]);
    }
}

void
DebugPrettyPrintInstructionStore (
    PINSTRUCTION Instruction
    )
{
    char OpcodeString[16];
    
    switch(Instruction->Opcode) {
    case OPC_STRI8:
        sprintf(OpcodeString, "%s", "STRI8");
        break;
    case OPC_STRU8:
        sprintf(OpcodeString, "%s", "STRU8");
        break;
    case OPC_STRI16:
        sprintf(OpcodeString, "%s", "STRI16");
        break;
    case OPC_STRU16:
        sprintf(OpcodeString, "%s", "STRU16");
        break;
    case OPC_STRI32:
        sprintf(OpcodeString, "%s", "STRI32");
        break;
    case OPC_STRU32:
        sprintf(OpcodeString, "%s", "STRU32");
        break;
    case OPC_STRF:
        sprintf(OpcodeString, "%s", "STRF");
        break;
    case OPC_STRTH:
        sprintf(OpcodeString, "%s", "STRTH");
        break;        
    }
    
    sprintf(OpcodeString, "%s%s", OpcodeString, Instruction->Store.AtomicStore ? "A" : "");
    sprintf(OpcodeString, "%-8s", OpcodeString);
    
    printf("%s %s%s%+d%s %s%s%+d%s\n",
           OpcodeString,                                                    // %s            
           
           IS_REGISTER_INDEX(Instruction->Store.RtRegister) ? "[" : "",     // %s
           _REGISTER_NAMES[Instruction->Store.RtRegister],                  // %s
           (signed)Instruction->Store.RtRegisterOffset,                     // %d
           IS_REGISTER_INDEX(Instruction->Store.RtRegister) ? "]" : "",     // %s
           
           IS_REGISTER_INDEX(Instruction->Store.DtRegister) ? "[" : "",     // %s
           _REGISTER_NAMES[Instruction->Store.DtRegister],                  // %s
           (signed)Instruction->Store.DtRegisterOffset,                     // %d
           IS_REGISTER_INDEX(Instruction->Store.DtRegister) ? "]" : "");    // %s
}

void
DebugPrettyPrintInstructionJump (
    PINSTRUCTION Instruction
    )
{
    char OpcodeString[16];
    unsigned JumpAddr;
    char JumpSign;
    
    switch(Instruction->Opcode) {
    case OPC_JMP:
        sprintf(OpcodeString, "%-8s", "JMP");
        break;
    case OPC_JMPZ:
        sprintf(OpcodeString, "%-8s", "JMPZ");
        break;
    case OPC_CALLNORM:
        sprintf(OpcodeString, "%-8s", "CALLNORM");
        break;
    case OPC_CALLPLLS:
        sprintf(OpcodeString, "%-8s", "CALLPLLS");
        break;
    case OPC_CALLPLLA:
        sprintf(OpcodeString, "%-8s", "CALLPLLA");
        break;
    }
    
    JumpAddr = _ABS((signed)Instruction->Jump.RegisterOffset);
    JumpSign = _SGN((signed)Instruction->Jump.RegisterOffset);
    
    if(Instruction->Opcode == OPC_JMPZ) {
        printf("%s %s%s%c0x%X%s %s\n",
               OpcodeString,                                                        // %s
               IS_REGISTER_INDEX(Instruction->Jump.Register) ? "[" : "",            // %s
               _REGISTER_NAMES[Instruction->Jump.Register],                         // %s
               JumpSign,                                                            // %c
               JumpAddr,                                                            // %X
               IS_REGISTER_INDEX(Instruction->Jump.Register) ? "[" : "",            // %s
               _REGISTER_NAMES[Instruction->Jump.ZeroRegister]);                    // %s
    } else {
        printf("%s %s%s%c0x%X%s\n",
               OpcodeString,                                                        // %s
               IS_REGISTER_INDEX(Instruction->Jump.Register) ? "[" : "",            // %s
               _REGISTER_NAMES[Instruction->Jump.Register],                         // %s
               JumpSign,                                                            // %c
               JumpAddr,                                                            // %X
               IS_REGISTER_INDEX(Instruction->Jump.Register) ? "[" : "");           // %s
    }
}

void
DebugPrettyPrintInstructionReturn (
    PINSTRUCTION Instruction
    )
{
    assert(Instruction->Opcode == OPC_RETURN);
    
    printf("%-8s 0x%X\n", "RETURN", (int)Instruction->Return.StackCleanup);
}

void
DebugPrettyPrintInstructionStack (
    PINSTRUCTION Instruction
    )
{
    char OpcodeString[16];
    
    switch(Instruction->Opcode) {
    case OPC_PUSH:
        sprintf(OpcodeString, "%-8s", "PUSH");
        break;
    case OPC_POP:
        sprintf(OpcodeString, "%-8s", "POP");
        break;
    }
    
    printf("%s %s%s%+d%s\n",
           OpcodeString,                                                    // %s
           IS_REGISTER_INDEX(Instruction->Stack.Register) ? "[" : "",       // %s
           _REGISTER_NAMES[Instruction->Stack.Register],                    // %s
           (signed)Instruction->Stack.RegisterOffset,                       // %d
           IS_REGISTER_INDEX(Instruction->Stack.Register) ? "]" : "");      // %s
}

void
DebugPrettyPrintInstructionIo (
    PINSTRUCTION Instruction
    )
{
    char OpcodeString[16];
    
    switch(Instruction->Opcode) {
    case OPC_PRINT:
        sprintf(OpcodeString, "%-8s", "PRINT");
        break;
    case OPC_READ:
        sprintf(OpcodeString, "%-8s", "READ");
        break;
    }
    
    printf("%s %+d\n",
           OpcodeString,
           Instruction->Io.PopCount);
}

void
DebugPrettyPrintInstruction (
    PINSTRUCTION Instruction
    )
{   
    switch(Instruction->Opcode) {
        case OPC_ADDI:
        case OPC_ADDF:
        case OPC_SUBI:
        case OPC_SUBF:
        case OPC_MULI:
        case OPC_MULF:
        case OPC_DIVI:
        case OPC_DIVF:
        case OPC_XOR:
        case OPC_OR:
        case OPC_AND:
        case OPC_NOT:
        case OPC_LOR:
        case OPC_LAND:
        case OPC_EQ:
        case OPC_NEQ:
        case OPC_LT:
        case OPC_GT:
        case OPC_LTE:
        case OPC_GTE:
            DebugPrettyPrintInstructionArithmetic(Instruction);
            break;
            
        case OPC_MOVE:
        case OPC_RCOPYD:
            DebugPrettyPrintInstructionMove(Instruction);
            break;
            
        case OPC_STRI8:
        case OPC_STRU8:
        case OPC_STRI16:
        case OPC_STRU16:
        case OPC_STRI32:
        case OPC_STRU32:
        case OPC_STRF:
        case OPC_STRTH:
            DebugPrettyPrintInstructionStore(Instruction);
            break;  
            
        case OPC_JMP:
        case OPC_JMPZ:
        case OPC_CALLNORM:
        case OPC_CALLPLLS:
        case OPC_CALLPLLA:
            DebugPrettyPrintInstructionJump(Instruction);
            break;
            
        case OPC_RETURN:
            DebugPrettyPrintInstructionReturn(Instruction);
            break;
            
        case OPC_PUSH:
        case OPC_POP:
            DebugPrettyPrintInstructionStack(Instruction);
            break;
            
        case OPC_PRINT:
        case OPC_READ:
            DebugPrettyPrintInstructionIo(Instruction);
            break;
            
        case OPC_ERR:
        default:
            assert(!"The fuck are you printing m8?");
            break;
    }
    
    return;
}

void
DebugPrintProgramIterator(void* Instr, unsigned ItemCount) 
{
    unsigned CurrentAddress;
    PINSTRUCTION Instruction;
    
    Instruction = Instr;
    CurrentAddress = PROGRAM_CODE_START + ItemCount * PROGRAM_CODE_ALIGNMENT;
    printf("0x%08X:\t0x%08X%08X\t", 
           CurrentAddress, 
           (int)((*(uint64_t*)Instruction) >> 32),
           (int)((*(uint64_t*)Instruction) & 0xFFFFFFFF));
    DebugPrettyPrintInstruction(Instruction);
}

void
DebugPrintProgram (
    PSQUEUE InstructionQueue
    )
{
    printf("####################### PROGRAM START #######################\n");
    SQueueIterate(InstructionQueue, DebugPrintProgramIterator);
    printf("#######################  PROGRAM END  #######################\n");
}

void
DebugPrettyPrintProgramHeader (
    PPROGRAM_HEADER Header
    )
{
    printf("##################### PROGRAM HDR START #####################\n");
    printf("Magic Number   : 0x%X\n", (unsigned int)Header->MagicNumber);
    printf("Version Major  : 0x%X\n", (unsigned int)Header->VersionMajor);
    printf("Version Minor  : 0x%X\n", (unsigned int)Header->VersionMinor);
    printf("Stack Alignment: 0x%X\n", (unsigned int)Header->StackAlignment);
    printf("Stack Top      : 0x%X\n", (unsigned int)Header->StackTop);
    printf("Data Start     : 0x%X\n", (unsigned int)Header->DataStart);
    printf("Code Start     : 0x%X\n", (unsigned int)Header->CodeStart);
    printf("Stack Size     : 0x%X\n", (unsigned int)Header->StackSize);
    printf("Data Size      : 0x%X\n", (unsigned int)Header->DataSize);
    printf("Code Size      : 0x%X\n", (unsigned int)Header->CodeSize);
    printf("Symbol Size    : 0x%X\n", (unsigned int)Header->SymbolSize);
    printf("Symbol Location: 0x%X\n", (unsigned int)Header->SymbolBinaryLocation);
    printf("Code Location  : 0x%X\n", (unsigned int)Header->CodeBinaryLocation);
    printf("###################### PROGRAM HDR END ######################\n");
}
