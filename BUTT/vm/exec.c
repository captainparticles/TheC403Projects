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
    
    exec.c
    
 Abstract:
   
    This module implements the instruction execution routines.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/24/15        Initial Creation

**/

#include "exec.h"
#include "error.h"
#include "memory_inl.h"
#include "program.h"
#include <windows.h>

#ifdef COMPILE_VERBOSE
#define PRINT_OUT       stdout
#else
extern FILE* _NUL;
#define PRINT_OUT       _NUL
#endif

extern PPROGRAM GProgram;
extern void VmFatal(char* Error);

ULONG GCodePointerBias;
ULONG GDataPointerBias;
ULONG GStackPointerBias;

extern
inline
VOID
MemRegisterValue (
    PCHAR Stack,
    PREGISTER_SET RegisterSet, 
    ULONG Register,
    LONG RegisterOffset,
    PLONG Value
    );

BOOL
ExecArithmeticInstruction (
    PTHREAD_EXECUTION_DATA ExecData,
    PINSTRUCTION Instruction
    )

/*

 Routine description:
 
    This routine executes an arithmetic instruction.
    
 Arguments:
 
    ExecData - The thread execution data for the calling thread.
    
    Instruction - The instruction to execute.
    
 Return value:
 
    TRUE if we should continue executing instructions. FALSE otherwise.

*/
    
{
    LONG L;
    LONG R;
    LONG D;
    ULONG Rl;
    ULONG Rr;
    ULONG Rd;
    LONG Rlo;
    LONG Rro;
    LONG Rdo;
    LONG StackOffset;
    
    Rl = Instruction->Arith.LtRegister;
    Rr = Instruction->Arith.RtRegister;
    Rd = Instruction->Arith.DtRegister;
    Rlo = Instruction->Arith.LtRegisterOffset;
    Rro = Instruction->Arith.RtRegisterOffset;
    Rdo = Instruction->Arith.DtRegisterOffset;
    
    MemRegisterValue(ExecData->ThreadStack,
                     ExecData->ActiveRegisterSet,
                     Rl,
                     Rlo,
                     &L);
                     
    MemRegisterValue(ExecData->ThreadStack,
                     ExecData->ActiveRegisterSet,
                     Rr,
                     Rro,
                     &R);
    
    switch(Instruction->Opcode) {
        case OPC_ADDI:
        case OPC_ADDF:
            D = L + R;
            break;
            
        case OPC_SUBI:
        case OPC_SUBF:
            D = L - R;
            break;
            
        case OPC_MULI:
        case OPC_MULF:
            D = L * R;
            break;
            
        case OPC_DIVI:
        case OPC_DIVF:
            D = L / R;
            break;
            
        case OPC_XOR:
            D = L ^ R;
            break;
            
        case OPC_OR:
            D = L | R;
            break;
            
        case OPC_AND:
            D = L & R;
            break;
            
        case OPC_NOT:
            assert(!"Yeah... didn't think this NOT thing through.");
            
        case OPC_LOR:
            D = L || R;
            break;
            
        case OPC_LAND:
            D = L && R;
            break;
            
        case OPC_EQ:
            D = L == R;
            break;
            
        case OPC_NEQ:
            D = L != R;
            break;
            
        case OPC_LT:
            D = L < R;
            break;
            
        case OPC_GT:
            D = L > R;
            break;
            
        case OPC_LTE:
            D = L <= R;
            break;
            
        case OPC_GTE:
            D = L >= R;
            break;
            
        default:
            VmFatal(ERR_STR_INVALIDINSTR);
    }
    
    fprintf(PRINT_OUT, "Arithmetic: Storing %ld OP %ld = %ld into %s + %ld\n",
           L, R, D, _REGISTER_NAMES[Rd], Rdo);
    
    if(Rd == REG_RGD) {
        memcpy(GProgram->GlobalData+Rro, &D, GProgram->Header.StackAlignment);
    } if(IS_REGISTER_INDEX(Rd)) {
        StackOffset = ExecData->ActiveRegisterSet->Register[Rd];
        StackOffset = StackOffset - GStackPointerBias;
        StackOffset = StackOffset + Rdo;
        memcpy(ExecData->ThreadStack+StackOffset,
               &D,
               GProgram->Header.StackAlignment);
    } else {
        ExecData->ActiveRegisterSet->Register[Rd] = D;
    }
    
    ExecData->ActiveRegisterSet->Register[REG_RIP] =
        ExecData->ActiveRegisterSet->Register[REG_RIP] + sizeof(INSTRUCTION);
    
    return TRUE;
}

BOOL
ExecDirectIndirectInstruction (
    PTHREAD_EXECUTION_DATA ExecData,
    PINSTRUCTION Instruction
    )
    
/*

 Routine description:
 
    This routine executes an indirect/direct instruction.
    
 Arguments:
 
    ExecData - The thread execution data for the calling thread.
    
    Instruction - The instruction to execute.
    
 Return value:
 
    TRUE if we should continue executing instructions. FALSE otherwise.

*/
    
{
    LONG L;
    LONG Offset;
    LONG D;
    ULONG Rl;
    
    if(Instruction->Opcode != OPC_RCOPYD) {
        VmFatal(ERR_STR_ONLYRCOPYD);
    }
    
    Rl = Instruction->Indirect.LtRegister;
    L = ExecData->ActiveRegisterSet->Register[Rl];
    if(Instruction->Indirect.LtOffsetType == INDIRECT_OFFSET_TYPE_CONSTANT) {
        Offset = Instruction->Indirect.LtOffset;
    } else {
        Offset = ExecData->ActiveRegisterSet->Register[Instruction->Indirect.LtOffset];
    }
    
    D = L + Offset;
    ExecData->ActiveRegisterSet->Register[Instruction->Indirect.DtRegister] = D;
    
    fprintf(PRINT_OUT, 
            "Copying value 0x%X into %s\n", 
            (int)D, 
            _REGISTER_NAMES[Instruction->Indirect.DtRegister]);
    
    ExecData->ActiveRegisterSet->Register[REG_RIP] =
        ExecData->ActiveRegisterSet->Register[REG_RIP] + sizeof(INSTRUCTION);
    
    return TRUE;
}

BOOL
ExecStoreInstruction (
    PTHREAD_EXECUTION_DATA ExecData,
    PINSTRUCTION Instruction
    )
    
/*

 Routine description:
 
    This routine executes a store instruction.
    
 Arguments:
 
    ExecData - The thread execution data for the calling thread.
    
    Instruction - The instruction to execute.
    
 Return value:
 
    TRUE if we should continue executing instructions. FALSE otherwise.
    
 Remarks:
    
    This routine masks and sign extends the data to be stored according to the
    store type.

*/
    
{
    LONG R;
    LONG D;
    ULONG Rr;
    ULONG Rd;
    LONG Rro;
    LONG Rdo;
    LONG StackOffset;
    unsigned StoreMask;
    unsigned SignBit;
    unsigned SignExtend;
    
    Rr = Instruction->Store.RtRegister;
    Rd = Instruction->Store.DtRegister;
    Rro = Instruction->Store.RtRegisterOffset;
    Rdo = Instruction->Store.DtRegisterOffset;
    
    switch(Instruction->Opcode) {
        case OPC_STRI8:
        case OPC_STRU8:
            StoreMask = 0xFF;
            SignExtend = 0xFFFFFF00;
            SignBit = 0x80;
            break;
            
        case OPC_STRI16:
        case OPC_STRU16:
            StoreMask = 0xFFFF;
            SignExtend = 0xFFFF0000;
            SignBit = 0x8000;
            break;
            
        case OPC_STRI32:
        case OPC_STRU32:
        case OPC_STRF:
        case OPC_STRTH:
            StoreMask = 0xFFFFFFFF;
            SignExtend = 0x00000000;
            SignBit = 0x80000000;
            break;
    }
    
    MemRegisterValue(ExecData->ThreadStack,
                     ExecData->ActiveRegisterSet,
                     Rr,
                     Rro,
                     &R);
    
    D = R & StoreMask;
    if(D & SignBit) {
        D = D | SignExtend;
    }
    
    if(Rd == REG_RGD) {
        memcpy(GProgram->GlobalData+Rro, &D, GProgram->Header.StackAlignment);
    } if(IS_REGISTER_INDEX(Rd)) {
        StackOffset = ExecData->ActiveRegisterSet->Register[Rd];
        StackOffset = StackOffset - GStackPointerBias;
        StackOffset = StackOffset + Rdo;
        memcpy(ExecData->ThreadStack+StackOffset,
               &D,
               GProgram->Header.StackAlignment);
    } else {
        ExecData->ActiveRegisterSet->Register[Rd] = D;
    }
    
    fprintf(PRINT_OUT, 
            "Store: Storing %ld into %s + %ld\n", 
            D, 
            _REGISTER_NAMES[Rd], 
            Rdo);
    
    ExecData->ActiveRegisterSet->Register[REG_RIP] =
        ExecData->ActiveRegisterSet->Register[REG_RIP] + sizeof(INSTRUCTION);
    
    return TRUE;
}

BOOL
ExecJumpInstruction (
    PTHREAD_EXECUTION_DATA ExecData,
    PINSTRUCTION Instruction
    )
    
/*

 Routine description:
 
    This routine executes a jump or call instruction.
    
 Arguments:
 
    ExecData - The thread execution data for the calling thread.
    
    Instruction - The instruction to execute.
    
 Return value:
 
    TRUE if we should continue executing instructions. FALSE otherwise.

*/
    
{
    ULONG Rj;
    ULONG Rz;
    signed Rjo;
    ULONG Target;
    ULONG ReturnAddress;
    signed StackOffset;
    PREGISTER_SET NewRegisterSet;
    
    Rj = Instruction->Jump.Register;
    Rz = Instruction->Jump.ZeroRegister;
    Rjo = Instruction->Jump.RegisterOffset;
    
    if(Rj == REG_RIP) {
        Target = ExecData->ActiveRegisterSet->Register[Rj] + Rjo;
        fprintf(PRINT_OUT, 
                "Conditional Target %s 0x%X\n", 
                _REGISTER_NAMES[Rj], 
                (unsigned int)Target);
                
    } else if(Rj == REG_RCT) {
        Target = Rjo;
        fprintf(PRINT_OUT, 
                "Target %s 0x%X\n", 
                _REGISTER_NAMES[Rj], 
                (unsigned int)Target);
        
    } else {
        assert(!"Stop! Jump is using invalid (currently) register");
    }
    
    if(Instruction->Jump.JumpType == JUMP_TYPE_CONDITIONAL) {
        
        assert(!IS_REGISTER_INDEX(Rz));
        
        if(ExecData->ActiveRegisterSet->Register[Rz] == 0) {
            ExecData->ActiveRegisterSet->Register[REG_RIP] = Target;
            return TRUE;
        } 
    } else {
        switch(Instruction->Opcode) {
            case OPC_JMP:
                ExecData->ActiveRegisterSet->Register[REG_RIP] = Target;
                return TRUE;
                
            case OPC_CALLNORM:
            
                //
                // Save the return address. 
                //
                
                ReturnAddress = ExecData->ActiveRegisterSet->Register[REG_RIP] +
                                sizeof(INSTRUCTION);
                 
                ExecData->ActiveRegisterSet->Register[REG_RSB] = 
                    ExecData->ActiveRegisterSet->Register[REG_RSB] - 
                    GProgram->Header.StackAlignment;
                
                StackOffset = ExecData->ActiveRegisterSet->Register[REG_RSB];
                StackOffset = StackOffset - GStackPointerBias;
                memcpy(ExecData->ThreadStack+StackOffset, 
                       &ReturnAddress, 
                       GProgram->Header.StackAlignment);
                 
                fprintf(PRINT_OUT, 
                        "Call: Pushing RIP+0x8: 0x%X at 0x%p RSB: 0x%X\n",
                       (int)ReturnAddress,
                       ExecData->ThreadStack+StackOffset,
                       (int)ExecData->ActiveRegisterSet->Register[REG_RSB]);
                    
                //
                // We save the registers. All of them. Even if we don't need to.
                // Because that's just how we roll. 
                //
                
                SStackPush(ExecData->RegisterSetStack,
                           ExecData->ActiveRegisterSet);
                
                NewRegisterSet = malloc(sizeof(REGISTER_SET));
                if(ExecData->ActiveRegisterSet == NULL) {
                    VmFatal(ERR_STR_NOMEM);
                }
                
                memset(NewRegisterSet, 0, sizeof(REGISTER_SET));
                NewRegisterSet->Register[REG_RIP] = Target;
                NewRegisterSet->Register[REG_RST] = 
                    ExecData->ActiveRegisterSet->Register[REG_RST];
                NewRegisterSet->Register[REG_RSB] = 
                    ExecData->ActiveRegisterSet->Register[REG_RSB];
                    
                ExecData->ActiveRegisterSet = NewRegisterSet;
                return TRUE;
                
            case OPC_CALLPLLA:
            case OPC_CALLPLLS:
                assert(!"Stop! Calls not yet supported.");
        }
    }
    
    ExecData->ActiveRegisterSet->Register[REG_RIP] =
        ExecData->ActiveRegisterSet->Register[REG_RIP] + sizeof(INSTRUCTION);
    
    return TRUE;
}

BOOL
ExecReturnInstruction (
    PTHREAD_EXECUTION_DATA ExecData,
    PINSTRUCTION Instruction
    )
    
/*

 Routine description:
 
    This routine executes a return instruction.
    
 Arguments:
 
    ExecData - The thread execution data for the calling thread.
    
    Instruction - The instruction to execute.
    
 Return value:
 
    TRUE if we should continue executing instructions. FALSE otherwise.

*/
    
{
    
    PREGISTER_SET TopRegisterSet;
    ULONG StackCleanup;
    ULONG ReturnAddress;
    ULONG Rsb = 0;
    signed StackOffset;
    
    if(SStackSize(ExecData->RegisterSetStack) == 0) {
        return FALSE;
    }
    
    StackCleanup = Instruction->Return.StackCleanup + 
                   GProgram->Header.StackAlignment;
    
    TopRegisterSet = SStackPop(ExecData->RegisterSetStack);
    StackOffset = ExecData->ActiveRegisterSet->Register[REG_RSB];
    StackOffset = StackOffset - GStackPointerBias;
    memcpy(&ReturnAddress, 
           ExecData->ThreadStack+StackOffset, 
           GProgram->Header.StackAlignment);
           
    assert((StackCleanup % GProgram->Header.StackAlignment) == 0);
           
    ExecData->ActiveRegisterSet->Register[REG_RSB] = 
        ExecData->ActiveRegisterSet->Register[REG_RSB] + 
        StackCleanup;
           
    fprintf(PRINT_OUT, "RETURN: RSB: 0x%X: Returning to 0x%X: Cleaning up 0x%X bytes\n", 
           (int)ExecData->ActiveRegisterSet->Register[REG_RSB],
           (int)ReturnAddress,
           (int)StackCleanup);
           
    fprintf(PRINT_OUT, "Stack RSB: 0x%X, Active RSB: 0x%X\n",
           (int)Rsb,
           (int)ExecData->ActiveRegisterSet->Register[REG_RSB]);
    
    TopRegisterSet->Register[REG_RIP] = ReturnAddress;
    TopRegisterSet->Register[REG_RRV] = ExecData->ActiveRegisterSet->Register[REG_RRV];
    TopRegisterSet->Register[REG_RST] = ExecData->ActiveRegisterSet->Register[REG_RST];
    TopRegisterSet->Register[REG_RSB] = ExecData->ActiveRegisterSet->Register[REG_RSB];
    free(ExecData->ActiveRegisterSet);
    ExecData->ActiveRegisterSet = TopRegisterSet;
    
    return TRUE;
}

BOOL
ExecStackInstruction (
    PTHREAD_EXECUTION_DATA ExecData,
    PINSTRUCTION Instruction
    )
    
/*

 Routine description:
 
    This routine executes a stack push/pop instruction.
    
 Arguments:
 
    ExecData - The thread execution data for the calling thread.
    
    Instruction - The instruction to execute.
    
 Return value:
 
    TRUE if we should continue executing instructions. FALSE otherwise.

*/
    
{
    ULONG R;
    ULONG D;
    PCHAR Da;
    signed Ro;
    signed StackOffset;
    
    R = Instruction->Stack.Register;
    Ro = Instruction->Stack.RegisterOffset;
    
    if(R == REG_RGD) {
        memcpy(&D, GProgram->GlobalData+Ro, GProgram->Header.StackAlignment);
        Da = GProgram->GlobalData+Ro;
    } if(IS_REGISTER_INDEX(R)) {
        StackOffset = ExecData->ActiveRegisterSet->Register[R];
        StackOffset = StackOffset - GStackPointerBias;
        StackOffset = StackOffset + Ro;
        //StackOffset = StackOffset - GProgram->Header.StackAlignment;
        memcpy(&D, 
               ExecData->ThreadStack+StackOffset, 
               GProgram->Header.StackAlignment);
               
        fprintf(PRINT_OUT, 
                "Pushing register index %s offset %d address 0x%p value %d\n",
               _REGISTER_NAMES[R],
               Ro,
               ExecData->ThreadStack+StackOffset,
               (int)D);
               
        Da = ExecData->ThreadStack+StackOffset;
    } else if(R == REG_RCT) {
        
        assert(Instruction->Opcode == OPC_PUSH);
        
        D = Ro;
    } else {        
        D = ExecData->ActiveRegisterSet->Register[R];
        Da = (PCHAR)&ExecData->ActiveRegisterSet->Register[R];
    }
    
    switch(Instruction->Opcode) {
    case OPC_PUSH: 
        ExecData->ActiveRegisterSet->Register[REG_RSB] = 
            ExecData->ActiveRegisterSet->Register[REG_RSB] - 
            GProgram->Header.StackAlignment;
        
        StackOffset = ExecData->ActiveRegisterSet->Register[REG_RSB];
        StackOffset = StackOffset - GStackPointerBias;
        memcpy(ExecData->ThreadStack+StackOffset, 
               &D, 
               GProgram->Header.StackAlignment);
            
        fprintf(PRINT_OUT, 
                "Push: Pushing 0x%X at 0x%p RSB: 0x%X\n",
               (int)D,
               ExecData->ThreadStack+StackOffset,
               (int)ExecData->ActiveRegisterSet->Register[REG_RSB]);
        
        break;
        
    case OPC_POP:   
        StackOffset = ExecData->ActiveRegisterSet->Register[REG_RSB];
        StackOffset = StackOffset - GStackPointerBias;
        memcpy(Da,
               ExecData->ThreadStack+StackOffset,
               GProgram->Header.StackAlignment);
               
        fprintf(PRINT_OUT, 
                "Pop: Poping 0x%X at 0x%p RSB: 0x%X\n",
               (int)*(int*)Da,
               ExecData->ThreadStack+StackOffset,
               (int)ExecData->ActiveRegisterSet->Register[REG_RSB]);  
        
        ExecData->ActiveRegisterSet->Register[REG_RSB] = 
            ExecData->ActiveRegisterSet->Register[REG_RSB] +
            GProgram->Header.StackAlignment;
        
        break;
    }
    
    ExecData->ActiveRegisterSet->Register[REG_RIP] =
        ExecData->ActiveRegisterSet->Register[REG_RIP] + sizeof(INSTRUCTION);
    
    return TRUE;
}

BOOL
ExecIoInstruction (
    PTHREAD_EXECUTION_DATA ExecData,
    PINSTRUCTION Instruction
    )
    
/*

 Routine description:
 
    This routine executes an IO read/print instruction.
    
 Arguments:
 
    ExecData - The thread execution data for the calling thread.
    
    Instruction - The instruction to execute.
    
 Return value:
 
    TRUE if we should continue executing instructions. FALSE otherwise.

*/
    
{
    ULONG i;
    ULONG PopCount;
    signed StackOffset;
    signed PrintVal;
    signed ReadAddress;
    signed RsbOffset;
    
    PopCount = Instruction->Io.PopCount;
    RsbOffset = ExecData->ActiveRegisterSet->Register[REG_RSB] + 
                PopCount * GProgram->Header.StackAlignment - 
                GProgram->Header.StackAlignment;
    
    switch(Instruction->Opcode) {
        case OPC_PRINT:
            for(i=0; i<PopCount; ++i) {    
                ExecData->ActiveRegisterSet->Register[REG_RSB] = 
                ExecData->ActiveRegisterSet->Register[REG_RSB] +
                GProgram->Header.StackAlignment;
                
                StackOffset = RsbOffset;
                StackOffset = StackOffset - GStackPointerBias;
                memcpy(&PrintVal,
                       ExecData->ThreadStack+StackOffset,
                       GProgram->Header.StackAlignment);
                
#ifdef COMPILE_VERBOSE
                fprintf(PRINT_OUT, 
                        "PRINT: 0x%p: RSB: 0x%X: RsbOffset: 0x%X: %d\n", 
                       ExecData->ThreadStack+StackOffset,
                       (int)ExecData->ActiveRegisterSet->Register[REG_RSB],
                       (int)RsbOffset,
                       PrintVal);
#else
                printf("PRINT: %d\n", PrintVal);
#endif
                RsbOffset = RsbOffset - GProgram->Header.StackAlignment;
            }
            
            break;
            
        case OPC_READ:
            for(i=0; i<PopCount; ++i) {      
                ExecData->ActiveRegisterSet->Register[REG_RSB] = 
                ExecData->ActiveRegisterSet->Register[REG_RSB] +
                GProgram->Header.StackAlignment;
                
                StackOffset = RsbOffset;
                StackOffset = StackOffset - GStackPointerBias;
                memcpy(&ReadAddress,
                       ExecData->ThreadStack+StackOffset,
                       GProgram->Header.StackAlignment);
                
                ReadAddress = ReadAddress - GStackPointerBias;
#ifdef COMPILE_VERBOSE
                fprintf(PRINT_OUT, 
                        "READ: RSB: 0x%X: RsbOffset: 0x%X: ReadAddr: 0x%X: ", 
                       (int)ExecData->ActiveRegisterSet->Register[REG_RSB], 
                       (int)RsbOffset,
                       (int)(ReadAddress + GStackPointerBias));
#else
                printf("READ: ");
#endif            
                scanf("%d", (int*)(ExecData->ThreadStack+ReadAddress));
                
                RsbOffset = RsbOffset - GProgram->Header.StackAlignment;
            }
            
            break;
    }
    
    ExecData->ActiveRegisterSet->Register[REG_RIP] =
        ExecData->ActiveRegisterSet->Register[REG_RIP] + sizeof(INSTRUCTION);
    
    return TRUE;
}

BOOL
ExecProcessInstruction (
    PTHREAD_EXECUTION_DATA ExecData,
    PINSTRUCTION Instruction
    )
    
/*

 Routine description:
 
    This routine executes an instruction.
    
 Arguments:
 
    ExecData - The thread execution data for the calling thread.
    
    Instruction - The instruction to execute.
    
 Return value:
 
    TRUE if we should continue executing instructions. FALSE otherwise.

*/
    
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
            return ExecArithmeticInstruction(ExecData, Instruction);
            
        case OPC_MOVE:
        case OPC_RCOPYD:
            return ExecDirectIndirectInstruction(ExecData, Instruction);
            
        case OPC_STRI8:
        case OPC_STRU8:
        case OPC_STRI16:
        case OPC_STRU16:
        case OPC_STRI32:
        case OPC_STRU32:
        case OPC_STRF:
        case OPC_STRTH:
            return ExecStoreInstruction(ExecData, Instruction);
            
        case OPC_JMP:
        case OPC_JMPZ:
        case OPC_CALLNORM:
        case OPC_CALLPLLS:
        case OPC_CALLPLLA:
            return ExecJumpInstruction(ExecData, Instruction);
            
        case OPC_RETURN:
            return ExecReturnInstruction(ExecData, Instruction);
            
        case OPC_PUSH:
        case OPC_POP:
            return ExecStackInstruction(ExecData, Instruction);
            
        case OPC_PRINT:
        case OPC_READ:
            return ExecIoInstruction(ExecData, Instruction);
            
        case OPC_ERR:
        default:
            VmFatal(ERR_STR_INVALIDINSTR);
            return FALSE; // Keep the compiler happy.
    }
}

VOID
ExecThreadExecute (
    PTHREAD_EXECUTION_DATA ExecData
    )
    
/*

 Routine description:
 
    This routine is the main execution loop for an execution thread.
    
 Arguments:
 
    ExecData - The thread execution data for the calling thread.
    
 Return value:
 
    VOID.

*/
    
{
    INSTRUCTION Instruction;
    ULONG InstructionIndex;
    BOOL ContinueProcessing;
    
    ContinueProcessing = TRUE;
    while(ContinueProcessing != FALSE) {
        InstructionIndex = ExecData->ActiveRegisterSet->Register[REG_RIP];
        InstructionIndex = InstructionIndex - GCodePointerBias;
        fprintf(PRINT_OUT, "Accessing instruction: 0x%X\n", (int)InstructionIndex);
        memcpy(&Instruction, &GProgram->Code[InstructionIndex], sizeof(INSTRUCTION));
        ContinueProcessing = ExecProcessInstruction(ExecData, &Instruction);
    }
    
    return;
}

DWORD
WINAPI
ThreadExecFunc (
    LPVOID Param
    )
    
/*

 Routine description:
 
    This routine initializes the thread execution data and starts executing 
    the instructions.
    
 Arguments:
 
    Param - The thread creation data for this thread
    
 Return value:
 
    0.

*/
    
{
    PTHREAD_CREATION_DATA ThreadCreationData;
    PTHREAD_EXECUTION_DATA ThreadExecData;
    
    ThreadExecData = malloc(sizeof(THREAD_EXECUTION_DATA));
    if(ThreadExecData == NULL) {
        VmFatal(ERR_STR_NOMEM);
    }
    
    SStackInitialize(&ThreadExecData->RegisterSetStack);
    if(ThreadExecData->RegisterSetStack == NULL) {
        VmFatal(ERR_STR_NOMEM);
    }
    
    ThreadExecData->ActiveRegisterSet = malloc(sizeof(REGISTER_SET));
    if(ThreadExecData->ActiveRegisterSet == NULL) {
        VmFatal(ERR_STR_NOMEM);
    }
    
    //
    // The spawning thread looks up the target address and copies the stack 
    // elements into the creation data, as well as copying its register set for
    // us. This way the spawning thread can resume execution and ignore us while
    // we set up shop. This also means we don't need to hold any locks for
    // thread creation. Nice.
    //
    
    ThreadCreationData = Param;    
    memcpy(ThreadExecData->ActiveRegisterSet, 
           ThreadCreationData->RegisterSet,
           sizeof(REGISTER_SET));
    
    ThreadExecData->ActiveRegisterSet->Register[REG_RIP] = ThreadCreationData->JumpAddress;
    ThreadExecData->ActiveRegisterSet->Register[REG_RST] = GProgram->Header.StackTop;
    ThreadExecData->ActiveRegisterSet->Register[REG_RSB] = GProgram->Header.StackTop;
    
    ThreadExecData->ThreadStack = malloc(GProgram->Header.StackSize);
    if(ThreadExecData->ThreadStack == NULL) {
        VmFatal(ERR_STR_NOMEM);
    }
    
    //
    // Stack grows down, so the stack pointer needs to point to the top of the
    // stack. No funny business here.
    //
    
    ThreadExecData->ThreadStack = 
        ThreadExecData->ThreadStack + GProgram->Header.StackSize - 1;
    
    if(ThreadCreationData->MiniStack != NULL) {
    
        //
        // MiniStackSize is the size in bytes of MiniStack. It better be stack 
        // aligned.
        //
    
        assert((ThreadCreationData->MiniStackSize % 
                GProgram->Header.StackAlignment) == 0);
        
        memcpy(ThreadExecData->ThreadStack,
               ThreadCreationData->MiniStack,
               ThreadCreationData->MiniStackSize);
               
        ThreadExecData->ActiveRegisterSet->Register[REG_RSB] = 
            ThreadExecData->ActiveRegisterSet->Register[REG_RSB] - 
            (ThreadCreationData->MiniStackSize / GProgram->Header.StackAlignment);
        
        free(ThreadCreationData->MiniStack);
    }
    
    free(ThreadCreationData->RegisterSet);
    free(ThreadCreationData);
    ExecThreadExecute(ThreadExecData);
    
    return 0;
}

VOID
ExecPrimeProgram (
    VOID
    )
    
/*

 Routine description:
 
    Houston, we have liftoff.
    
 Arguments:
 
    VOID
    
 Return value:
 
    VOID.

*/
    
{
    PTHREAD_CREATION_DATA FirstThread;
    HANDLE FirstThreadHandle;
    
    //
    // The code is compiled with different offsets in mind, as its meant to 
    // simulate running in an environment with a single address space. We
    // counter this here with bias variables which we subtract with ever access
    // to certain registers
    //
    
    GCodePointerBias = GProgram->Header.CodeStart;
    GDataPointerBias = GProgram->Header.DataStart;
    GStackPointerBias = GProgram->Header.StackTop;
    
    FirstThread = malloc(sizeof(THREAD_CREATION_DATA));
    if(FirstThread == NULL) {
        VmFatal(ERR_STR_NOMEM);
    }
    
    memset(FirstThread, 0, sizeof(THREAD_CREATION_DATA));
    FirstThread->RegisterSet = malloc(sizeof(REGISTER_SET));
    if(FirstThread->RegisterSet == NULL) {
        VmFatal(ERR_STR_NOMEM);
    }
    
    memset(FirstThread->RegisterSet, 0, sizeof(REGISTER_SET));
    FirstThread->MiniStack = NULL;
    FirstThread->MiniStackSize = 0;
    FirstThread->JumpAddress = GProgram->Header.CodeStart;
    
    FirstThreadHandle = CreateThread(NULL,
                                     0,
                                     ThreadExecFunc,
                                     (LPVOID)FirstThread,
                                     0,
                                     NULL);
                                     
    WaitForSingleObject(FirstThreadHandle, INFINITE);
}
