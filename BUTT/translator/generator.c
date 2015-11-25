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
    
    generator.c
    
 Abstract:
    
    This module implements the routines used to generate code for the various 
    language constructs. These routines generally interface with instruction.h
    to generate the appropriate code.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation
    11/25/15        Documented functions

**/

#include "generator.h"
#include "objtypes.h"
#include "instruction.h"
#include "errors.h"
#include "register.h"
#include "debug.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef COMPILE_VERBOSE
#define PRINT_OUT       stdout
#else
extern FILE* _NUL;
#define PRINT_OUT       _NUL
#endif

extern int yyerror(char* err);

int
CheckOperatorIsInOperatorArray (
    OPR_TYPE Operator,
    POPR_TYPE OperatorArray,
    size_t OperatorArraySize
    )
    
/*

 Routine description:
 
    This routine checks if a given parameter is in a parameter array. It is
    quite literally a linear array search.
    
 Arguments:
 
    Operator - Operator to search for in the array.
    
    OperatorArray - The array to search in.
    
    OperatorArraySize - Number of elements for OperatorArray
    
 Return value:
 
    1 if found, 0 otherwise.

*/
    
{
    unsigned i;
    
    for(i=0; i<OperatorArraySize; ++i) {
        if(Operator == OperatorArray[i]) {
            return 1;
        }
    }
    
    return 0;
}

void
GenerateProgramStartBlockStage0 (
    PSSTACK PendingInstructionStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is the first stage in generating the program starting block. 
    This block is in charge of loading the initial values for RIP, RSB, and RST 
    and finally jumping to the main function. Stage 0 does the first three steps
    and inserts a pending instruction as we currently do not know mains address.
    
 Arguments:
 
    PendingInstructionStack - Pointer to the pending instruction stack.
    
    InstructionQueue - Pointer to the global instruction queue for the program.
    
    Context - Pointer to the programs global context.
    
 Return value:
 
    void.

*/
    
{
    
    assert(Context == Context->GlobalContext);
    
    //
    // Our stack and global data sections are of variable length... or at least
    // they have the capability of such. Thus, we have to load the values into
    // the RST, RSB, and RGD registers. Finally, we have to introduce a JMP 
    // instruction that will jump to main.
    //
    
    PINSTRUCTION InstructionStep1;
    PINSTRUCTION InstructionStep2;
    PINSTRUCTION InstructionStep3;
    PINSTRUCTION InstructionStep4;
    PIDENTIFIER_OBJECT RegisterRst;
    PIDENTIFIER_OBJECT RegisterRsb;
    PIDENTIFIER_OBJECT RegisterRgd;
    PIDENTIFIER_OBJECT RegisterRct;
    
    RegisterRst = RegisterSpecialRegister(REG_RST);
    RegisterRsb = RegisterSpecialRegister(REG_RSB);
    RegisterRgd = RegisterSpecialRegister(REG_RGD);
    RegisterRct = RegisterIdentifierAsIntegerConstant(PROGRAM_STACK_TOP, Context);
    
    InstructionStep1 = InstrMakeIndirectDirect(OPC_RCOPYD, RegisterRct, NULL, RegisterRst);
    InstructionStep2 = InstrMakeIndirectDirect(OPC_RCOPYD, RegisterRct, NULL, RegisterRsb);
    RegisterRct->RelOffset = PROGRAM_DATA_START;
    InstructionStep3 = InstrMakeIndirectDirect(OPC_RCOPYD, RegisterRct, NULL, RegisterRgd);
    InstructionStep4 = InstrMakeJump(OPC_JMP, NULL);
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionStep1);
    DebugPrettyPrintInstruction(InstructionStep2);
    DebugPrettyPrintInstruction(InstructionStep3);
    DebugPrettyPrintInstruction(InstructionStep4);
#endif
    
    Context->CodePointer = Context->CodePointer + 4*PROGRAM_CODE_ALIGNMENT;
    SStackPush(PendingInstructionStack, InstructionStep4);
    SQueuePush(InstructionQueue, InstructionStep1);
    SQueuePush(InstructionQueue, InstructionStep2);
    SQueuePush(InstructionQueue, InstructionStep3);
    SQueuePush(InstructionQueue, InstructionStep4);
    
    DestroyIdentifier(RegisterRst);
    DestroyIdentifier(RegisterRsb);
    DestroyIdentifier(RegisterRgd);
    DestroyIdentifier(RegisterRct);
}

void
GenerateProgramStartBlockStage1 (
    PSSTACK PendingInstructionStack,
    PSQUEUE InstructionQueue,
    unsigned long MainAddress,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is the second and final stage in generating the program 
    starting block. This stage patches in the jump to main.
    
 Arguments:
 
    PendingInstructionStack - Pointer to the pending instruction stack.
    
    InstructionQueue - Pointer to the global instruction queue for the program.
    
    MainAddress - Absolute address to the main function.
    
    Context - Pointer to the programs global context.
    
 Return value:
 
    void.

*/
    
{
    (void)InstructionQueue;
    assert(SStackSize(PendingInstructionStack) == 1);
    
    PINSTRUCTION InstructionJumpMain;
    PIDENTIFIER_OBJECT RegisterRct;
    
    RegisterRct = RegisterIdentifierAsIntegerConstant(MainAddress, Context);
    InstructionJumpMain = SStackPop(PendingInstructionStack);
    InstrPatchJumpTargetAbsolute(InstructionJumpMain, RegisterRct);
    DestroyIdentifier(RegisterRct);
}

PINSTRUCTION
GenerateExpressionInstruction (
    PIDENTIFIER_OBJECT OperandL,
    PIDENTIFIER_OBJECT OperandR,
    POPERATOR_OBJECT Operator,
    PIDENTIFIER_OBJECT *OperandOut
    )
    
/*

 Routine description:
 
    This routine is in charge of generating an arithmetic instruction given two
    operands and an operator.
    
 Arguments:
 
    OperandL - A pointer to the left operand.
    
    OperandR - A pointer to the right operand.
    
    Operator - A pointer to the operator.
    
    OperandOut - A pointer to the identifier to be pushed back into the operand
                 stack. We do this so we can operate on temporary variables.
    
 Return value:
 
    A pointer to a dynamically allocated instruction.

*/
    
{
    PIDENTIFIER_OBJECT DestinationRegister;
    OPCODES Opcode;
    IDN_TYPE DestinationType;
    PINSTRUCTION Instruction;
    
    *OperandOut = NULL;
    
    //
    // Ok, very very first thing we HAVE to do is to check if either OperandL
    // or OperandR are registers of type REG_RTn because if they are we need
    // to dereference them.
    //
    
    if(IS_REGISTER_WORKING(OperandR->Register) || 
       IS_REGISTER_INDEX_IX(OperandR->Register)) {
        DereferenceRegister(OperandR);
    }
    
    if(IS_REGISTER_WORKING(OperandL->Register) || 
       IS_REGISTER_INDEX_IX(OperandL->Register)) {
        DereferenceRegister(OperandL);
    }
    
    //
    // Generate the opcode for this operation.
    //
    
    Opcode = GenerateOpcode(OperandL->DataType, 
                            OperandR->DataType, 
                            Operator->Type);
    
    if(Opcode == OPC_ERR) {
        yyerror(ERR_STR_INVALIDINSTR);
        return NULL;
    }
    
    //
    // Determine the new register data type.
    //
    
    DestinationType = GenerateResultingDataType(OperandL->DataType, 
                                                OperandR->DataType, 
                                                Operator->Type);
    
    if(DestinationType == IDN_TYPE_ERR) {
        yyerror(ERR_STR_INVALIDINSTR);
        return NULL;
    }
    
    if(Operator->Type == OPR_TYPE_STR) {
        Instruction = InstrMakeStore(Opcode, OperandR, OperandL);
        *OperandOut = OperandL;
    } else {
    
        //
        // Get the next available register to work with.
        //
        
        DestinationRegister = NextAvailableRegister( );
        if(DestinationRegister == NULL) {
            yyerror(ERR_STR_NOREGISTERS);
            return NULL;
        }
        
        DestinationRegister->DataType = DestinationType;
        Instruction = InstrMakeArithmetic(Opcode, 
                                          OperandL, 
                                          OperandR, 
                                          DestinationRegister);
    
        *OperandOut = DestinationRegister;
    }
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(Instruction);
#endif
    
    return Instruction;
}

void
GenerateExpressionInstructions (
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    POPR_TYPE OperatorCheck,
    size_t OperatorCheckSize,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of generating arithmetic instructions while a 
    given set of operators lay at the top of the operator stack.
    
 Arguments:
 
    OperandStack - A pointer to the operand stack.
    
    OperatorStack - A pointer to the operator stack.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    OperatorCheck - A pointer to the array holding the operators we are going to
                    check against here. We generate instructions as long as the
                    top operator exists in this array.
                 
    OperatorCheckSize - The number of elements in OperatorCheck.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    POPERATOR_OBJECT Operator;
    PIDENTIFIER_OBJECT OperandL;
    PIDENTIFIER_OBJECT OperandR;
    PIDENTIFIER_OBJECT PushBackIdentifier;
    PINSTRUCTION Instruction;
    
    if(SStackSize(OperatorStack) == 0) { 
        return;
    }
    
    Operator = SStackTop(OperatorStack);
    while(CheckOperatorIsInOperatorArray(Operator->Type, 
                                         OperatorCheck, 
                                         OperatorCheckSize)) {
        
        Operator = SStackPop(OperatorStack);
        
        assert(SStackSize(OperandStack) >= 2);
        
        OperandR = SStackPop(OperandStack);
        OperandL = SStackPop(OperandStack);
        Instruction = GenerateExpressionInstruction(OperandL, 
                                                    OperandR, 
                                                    Operator, 
                                                    &PushBackIdentifier);
                                          
        assert(Instruction != NULL);
        assert(PushBackIdentifier != NULL);
        
        if(Instruction == NULL) {
            yyerror(ERR_STR_INVALIDINSTR);
        }
        
        Context->CodePointer = Context->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
        SQueuePush(InstructionQueue, Instruction);
        SStackPush(OperandStack, PushBackIdentifier);
        if(SStackSize(OperatorStack) == 0) {
            break;
        } else {
            Operator = SStackTop(OperatorStack);
        }
    }
}

void
GenerateExpressionInstructionsEmptyStacks (
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine generates arithmetic expressions until it exhausts the operand
    and operator stacks, and then pushes the final result back into the operand
    stack.
    
 Arguments:
 
    OperandStack - A pointer to the operand stack.
    
    OperatorStack - A pointer to the operator stack.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    POPERATOR_OBJECT Operator;
    PIDENTIFIER_OBJECT OperandL;
    PIDENTIFIER_OBJECT OperandR;
    PIDENTIFIER_OBJECT PushBackIdentifier;
    PINSTRUCTION Instruction;
    
    while(SStackSize(OperatorStack) != 0) {
        
        assert(SStackSize(OperandStack) >= 2);
        
        Operator = SStackPop(OperatorStack);
        OperandR = SStackPop(OperandStack);
        OperandL = SStackPop(OperandStack);
        Instruction = GenerateExpressionInstruction(OperandL, 
                                                    OperandR, 
                                                    Operator, 
                                                    &PushBackIdentifier);
        
        assert(PushBackIdentifier != NULL);
        
        if(Instruction == NULL) {
            yyerror(ERR_STR_INVALIDINSTR);
        }
        
        Context->CodePointer = Context->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
        SQueuePush(InstructionQueue, Instruction);
        SStackPush(OperandStack, PushBackIdentifier);
        DestroyOperator(Operator);
        Operator = SStackPop(OperatorStack);
    }
    
    assert(SStackSize(OperandStack) == 1);
    assert(SStackSize(OperatorStack) == 0);
}

void
GenerateExpressionInstructionsUntilMatch (
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    OPR_TYPE OperatorMatch,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine generates arithmetic expressions until the top of the operator
    stack matches a provided parameter. 
    
 Arguments:
 
    OperandStack - A pointer to the operand stack.
    
    OperatorStack - A pointer to the operator stack.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    OperatorMatch - Operator to be matched against to stop generation.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    POPERATOR_OBJECT Operator;
    PIDENTIFIER_OBJECT OperandL;
    PIDENTIFIER_OBJECT OperandR;
    PIDENTIFIER_OBJECT PushBackIdentifier;
    PINSTRUCTION Instruction;
    
    Operator = SStackPop(OperatorStack);
    
    assert(Operator != NULL);
    
    while(Operator->Type != OperatorMatch) {
        OperandR = SStackPop(OperandStack);
        OperandL = SStackPop(OperandStack);
        Instruction = GenerateExpressionInstruction(OperandL, 
                                                    OperandR, 
                                                    Operator, 
                                                    &PushBackIdentifier);
        
        assert(PushBackIdentifier != NULL);
        
        if(Instruction == NULL) {
            yyerror(ERR_STR_INVALIDINSTR);
        }
        
        Context->CodePointer = Context->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
        SQueuePush(InstructionQueue, Instruction);
        SStackPush(OperandStack, PushBackIdentifier);
        DestroyOperator(Operator);
        Operator = SStackPop(OperatorStack);
    }
}

void
GenerateArrayInstructions (
    PSSTACK OperandStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of generating access to an array-type variable. 
    Returning from this function an index type register will lay at the top of 
    the operand stack. This index register holds the address of the element 
    being accessed.
    
 Arguments:
 
    OperandStack - A pointer to the operand stack.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    PIDENTIFIER_OBJECT AccessOffset;
    PIDENTIFIER_OBJECT ArrayBase;
    PINSTRUCTION MultiplyArrayOffset;
    PINSTRUCTION LoadArrayBase;
    PINSTRUCTION MoveArrayOffset;
    IDENTIFIER_OBJECT ArrayOffsetIntoStack;
    IDENTIFIER_OBJECT ConstantStackAlignment;
    IDENTIFIER_OBJECT RstRegister;
    PIDENTIFIER_OBJECT OffsetRegister;
    PIDENTIFIER_OBJECT AccessIndexRegister;
    
    AccessOffset = SStackPop(OperandStack);
    ArrayBase = SStackPop(OperandStack);
    
    //
    // The offset into the stack is a constant value. Also, currently we do
    // not support passing arrays as parameters, which means the relative offset
    // into RST will always be negative if its not a global array.
    //
    
    assert((ArrayBase->Register == REG_RST && ArrayBase->RelOffset < 0) ||
           (ArrayBase->Register == REG_RGD && ArrayBase->RelOffset >= 0));
    
    memset(&ArrayOffsetIntoStack, 0, sizeof(IDENTIFIER_OBJECT));
    ArrayOffsetIntoStack.Register = REG_RCT;
    ArrayOffsetIntoStack.RelOffset = ArrayBase->RelOffset;
    
    memset(&ConstantStackAlignment, 0, sizeof(IDENTIFIER_OBJECT));
    ConstantStackAlignment.Register = REG_RCT;
    if(ArrayBase->Register == REG_RST) {
        ConstantStackAlignment.RelOffset = -PROGRAM_STACK_ALIGNMENT;
    } else {
        ConstantStackAlignment.RelOffset = +PROGRAM_STACK_ALIGNMENT;
    }
    
    memset(&RstRegister, 0, sizeof(IDENTIFIER_OBJECT));
    if(ArrayBase->Register == REG_RST) {
        RstRegister.Register = REG_RST;
    } else {
        RstRegister.Register = REG_RGD;
    }
    
    //
    // Grab the next available register for the array first. We will get this 
    // with a reference count 1, keep it that way as we use it on both sides of
    // the instruction.
    //
    // Unless the access offset is a register, in which case its reference count
    // must be one, and we use it.
    //
    
    if(IS_REGISTER_INDEX_IX(AccessOffset->Register)) {
        DereferenceRegister(AccessOffset);
    }
    
    if(IS_REGISTER_WORKING(AccessOffset->Register)) {
        
        assert(AccessOffset->RegisterReferenceCount >= 1);
        
        OffsetRegister = AccessOffset;
    } else {
        OffsetRegister = NextAvailableRegister( );
        if(OffsetRegister == NULL) {
            yyerror(ERR_STR_NOREGISTERS);
        }
    }
    
    AccessIndexRegister = NextAvailableRegisterIndex( );
    
    MultiplyArrayOffset = InstrMakeArithmetic(OPC_MULI,
                                              AccessOffset,
                                              &ConstantStackAlignment,
                                              OffsetRegister);
    
    LoadArrayBase = InstrMakeArithmetic(OPC_ADDI,
                                        OffsetRegister, 
                                        &ArrayOffsetIntoStack, 
                                        OffsetRegister);
    
    //
    // Store the address in an index register. It will get dereferenced in any
    // store/arithmetic operation. That's just how we roll.
    //
    
    MoveArrayOffset = InstrMakeIndirectDirect(OPC_RCOPYD, 
                                              &RstRegister, 
                                              OffsetRegister, 
                                              AccessIndexRegister);
                                              
    DereferenceRegister(OffsetRegister);
    Context->CodePointer = Context->CodePointer + 3*PROGRAM_CODE_ALIGNMENT;    
    SQueuePush(InstructionQueue, MultiplyArrayOffset);
    SQueuePush(InstructionQueue, LoadArrayBase);
    SQueuePush(InstructionQueue, MoveArrayOffset);
    SStackPush(OperandStack, AccessIndexRegister);
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(MultiplyArrayOffset);
    DebugPrettyPrintInstruction(LoadArrayBase);
    DebugPrettyPrintInstruction(MoveArrayOffset);
#endif
}

void
GenerateIoAddReadIdentifier (
    PIO_OBJECT IoObject,
    PIDENTIFIER_OBJECT Identifier,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of generating a push instruction to load the 
    address of an identifier onto the stack for it to be read. 
    
 Arguments:
 
    IoObject - A pointer to IO Object.
    
    Identifier - A pointer to the identifier for the variable to be read to.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    assert(Context != Context->GlobalContext);
    assert(IS_REGISTER_INDEX(Identifier->Register));
    
    PINSTRUCTION InstructionPush;
    PINSTRUCTION InstructionCopy;
    PIDENTIFIER_OBJECT RegisterRt0;
    PIDENTIFIER_OBJECT RegisterRct;
    
    RegisterRct = RegisterIdentifierAsIntegerConstant(Identifier->RelOffset, Context);
    RegisterRt0 = NextAvailableRegister( );
    
    assert(RegisterRt0->Register == REG_RT0);
    
    InstructionCopy = InstrMakeIndirectDirect(OPC_RCOPYD, 
                                              Identifier, 
                                              RegisterRct, 
                                              RegisterRt0);
                                              
    InstructionPush = InstrMakeStackPush(OPC_PUSH, RegisterRt0);
    Context->CodePointer = Context->CodePointer + 2*PROGRAM_CODE_ALIGNMENT;
    SQueuePush(InstructionQueue, InstructionCopy);
    SQueuePush(InstructionQueue, InstructionPush);
    IoObject->IoIdentifierCount = IoObject->IoIdentifierCount + 1;
    DereferenceRegister(RegisterRt0);
    DestroyIdentifier(RegisterRct);

#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionCopy);
    DebugPrettyPrintInstruction(InstructionPush);
#endif
}

void
GenerateIoAddPrintIdentifier (
    PIO_OBJECT IoObject,
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine parses an expression and pushes the resulting identifier for it
    to be printed.
    
 Arguments:
 
    IoObject - A pointer to IO Object.
    
    OperandStack - A pointer to the operand stack.
    
    OperatorStack - A pointer to the operator stack.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    assert(Context != Context->GlobalContext);
    
    PIDENTIFIER_OBJECT PrintIdentifier;
    PINSTRUCTION InstructionPush;
    
    GenerateExpressionInstructionsEmptyStacks(OperandStack,
                                              OperatorStack,
                                              InstructionQueue,
                                              Context);
    
    PrintIdentifier = SStackPop(OperandStack);
    InstructionPush = InstrMakeStackPush(OPC_PUSH, PrintIdentifier);
    Context->CodePointer = Context->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
    SQueuePush(InstructionQueue, InstructionPush);
    IoObject->IoIdentifierCount = IoObject->IoIdentifierCount + 1;
    DereferenceRegister(PrintIdentifier);
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionPush);
#endif
}
    
void
GenerateIoFinishRead (
    PIO_OBJECT IoObject,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of generating a read instruction for the total 
    number of addresses to read to.
    
 Arguments:
 
    IoObject - A pointer to IO Object.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    assert(Context != Context->GlobalContext);
    assert(IoObject->IoIdentifierCount >= 1);
    
    PIDENTIFIER_OBJECT RegisterRct;
    PINSTRUCTION InstructionRead;
    
    RegisterRct = RegisterIdentifierAsIntegerConstant(IoObject->IoIdentifierCount,
                                                      Context);
                                                      
    InstructionRead = InstrMakeIoRead(OPC_READ, RegisterRct);
    Context->CodePointer = Context->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
    SQueuePush(InstructionQueue, InstructionRead);
    DestroyIdentifier(RegisterRct);
   
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionRead);
#endif
    
    //
    // Clean the IoObject
    //
    
    CleanIoObject(IoObject);
}

void
GenerateIoFinishPrint (
    PIO_OBJECT IoObject,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of generating a print instruction for the total 
    number of values to print.
    
 Arguments:
 
    IoObject - A pointer to IO Object.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    assert(Context != Context->GlobalContext);
    assert(IoObject->IoIdentifierCount >= 1);
    
    PIDENTIFIER_OBJECT RegisterRct;
    PINSTRUCTION InstructionPrint;
    
    RegisterRct = RegisterIdentifierAsIntegerConstant(IoObject->IoIdentifierCount,
                                                      Context);
                                                      
    InstructionPrint = InstrMakeIoPrint(OPC_PRINT, RegisterRct);
    Context->CodePointer = Context->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
    SQueuePush(InstructionQueue, InstructionPrint);
    DestroyIdentifier(RegisterRct);
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionPrint);
#endif
    
    //
    // Clean the IoObject
    //
    
    CleanIoObject(IoObject);
}

void
GenerateFunctionHeaderStage0 (
    PSSTACK PendingInstructionStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of executing the first stage in the generation of
    function headers. Stage 0 consists of the following steps:
    
    [0] RCOPYD RST RT0
    [1] PUSH   RT0 
    [2] RCOPYD RSB RST
    [3] RCOPYD RSB+X RSB
    
    Step 3 is completed in stage 1, as it is in charge of reserving space on the
    stack for local variables, and we do not yet know this value.
    One may notice this is very similar to the __cdecl calling convention in C.
    
 Arguments:
 
    PendingInstructionStack - A pointer to the pending instruction stack.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    assert(Context != Context->GlobalContext);
    
    //
    // Since it makes parameter passing simpler when evaluating them on the 
    // function call, we have to invert the parameter list offsets here.
    //
    
    PINSTRUCTION InstructionStep0;
    PINSTRUCTION InstructionStep1;
    PINSTRUCTION InstructionStep2;
    PINSTRUCTION InstructionStep3;
    PIDENTIFIER_OBJECT FunctionIdentifier;
    PIDENTIFIER_OBJECT RegisterRt0;
    PIDENTIFIER_OBJECT RegisterRst;
    PIDENTIFIER_OBJECT RegisterRsb;
    PIDENTIFIER_OBJECT RegisterRct;
    unsigned ParameterCount;
    unsigned CurrentParameterOffset;
    void* CurrentParameterNode;
    PIDENTIFIER_OBJECT CurrentParameter;
    
    FunctionIdentifier = Context->Identifier;
    ParameterCount = SQueueSize(FunctionIdentifier->Parameters);
    CurrentParameterOffset = 4 + PROGRAM_STACK_ALIGNMENT*ParameterCount;
    CurrentParameterNode = SQueueTopNode(FunctionIdentifier->Parameters);
    while(CurrentParameterNode != NULL) {
        
        assert(CurrentParameterOffset >= 8);
        
        CurrentParameter = SQueueDataFromNode(CurrentParameterNode);
        CurrentParameter->RelOffset = CurrentParameterOffset;
        CurrentParameterOffset = CurrentParameterOffset - PROGRAM_STACK_ALIGNMENT;
        CurrentParameterNode = SQueueNextFromNode(CurrentParameterNode);
    }
    
    RegisterRt0 = NextAvailableRegister( );
    
    assert(RegisterRt0->Register == REG_RT0);
    
    RegisterRst = RegisterSpecialRegister(REG_RST);
    RegisterRsb = RegisterSpecialRegister(REG_RSB);
    RegisterRct = RegisterIdentifierAsIntegerConstant(1, Context);
    
    InstructionStep0 = InstrMakeIndirectDirect(OPC_RCOPYD, RegisterRst, NULL, RegisterRt0);
    InstructionStep1 = InstrMakeStackPush(OPC_PUSH, RegisterRt0);
    InstructionStep2 = InstrMakeIndirectDirect(OPC_RCOPYD, RegisterRsb, NULL, RegisterRst);
    InstructionStep3 = InstrMakeIndirectDirect(OPC_RCOPYD, RegisterRsb, RegisterRct, RegisterRsb);
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionStep0);
    DebugPrettyPrintInstruction(InstructionStep1);
    DebugPrettyPrintInstruction(InstructionStep2);
    DebugPrettyPrintInstruction(InstructionStep3);
#endif
    
    Context->CodePointer = Context->CodePointer + 4*PROGRAM_CODE_ALIGNMENT;
    SStackPush(PendingInstructionStack, InstructionStep3);
    SQueuePush(InstructionQueue, InstructionStep0);
    SQueuePush(InstructionQueue, InstructionStep1);
    SQueuePush(InstructionQueue, InstructionStep2);
    SQueuePush(InstructionQueue, InstructionStep3);
    
    DereferenceRegister(RegisterRt0);
    DestroyIdentifier(RegisterRst);
    DestroyIdentifier(RegisterRsb);
    DestroyIdentifier(RegisterRct);
}

void
GenerateFunctionHeaderStage1 (
    PSSTACK PendingInstructionStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of executing the last stage in the generation of
    function headers. Stage 1 simply patches in the number of bytes to reserve
    on the stack for the function.
    
 Arguments:
 
    PendingInstructionStack - A pointer to the pending instruction stack.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    //
    // All this function does is complete the function header by completing the
    // RCOPYD instruction reserving stack memory for local variables.
    //
    
    (void)InstructionQueue;
    PINSTRUCTION InstructionPatch;
    PIDENTIFIER_OBJECT RegisterRct;
    signed DataSize;
    
    DataSize = Context->DataPointer;
    InstructionPatch = SStackPop(PendingInstructionStack);
    
    assert(DataSize <= 0);
    
    RegisterRct = RegisterIdentifierAsIntegerConstant(DataSize, Context);
    InstrPatchIndirectDirectAddSourceOffset(InstructionPatch, RegisterRct);
}
    
void
GenerateFunctionTrailer (
    PSSTACK PendingInstructionStack,
    PSSTACK InstructionCountStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of generating the exit sequence for functions, and
    consists of the following steps:
    
    [0] RCOPYD RSB+X RSB
    [1] POP    RT0
    [2] RCOPYD RT0 RST
    [3] RETURN Y
    
    X refers to the bytes reserved for local variables, and Y refers to them
    number of bytes reserved by parameters to the function. The latter is used 
    to clean up the stack upon execution of the return instruction.
    
    Additionally, this function patches all the return instructions found in the
    function to make them jump to this exit sequence.
    
 Arguments:
 
    PendingInstructionStack - A pointer to the pending instruction stack.
    
    InstructionCountStack - A pointer to all pending return instructions.
    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{   
    assert(Context != Context->GlobalContext);
    assert(Context->Identifier->Register == REG_RFN);
    
    PINSTRUCTION InstructionStep0;
    PINSTRUCTION InstructionStep1;
    PINSTRUCTION InstructionStep2;
    PINSTRUCTION InstructionStep3;
    PINSTRUCTION InstructionPatchJump;
    PIDENTIFIER_OBJECT RegisterRt0;
    PIDENTIFIER_OBJECT RegisterRst;
    PIDENTIFIER_OBJECT RegisterRsb;
    PIDENTIFIER_OBJECT RegisterRct;
    PIDENTIFIER_OBJECT RelativeOffset;
    unsigned long ReturnCount;
    unsigned long CurrentReturnCount;
    signed Delta;
    unsigned StackCleanupBytes;
    
    //
    // Patch all the return jump instructions first
    //
    
    CurrentReturnCount = 0;
    ReturnCount = Context->Identifier->ReturnCount;
    RelativeOffset = RegisterIdentifierAsIntegerConstant(0, Context);
    while(CurrentReturnCount < ReturnCount) {
        InstructionPatchJump = SStackPop(PendingInstructionStack);
        Delta = SQueueSize(InstructionQueue) - 
                (size_t)SStackPop(InstructionCountStack);
        
        Delta = Delta * PROGRAM_CODE_ALIGNMENT;
        RelativeOffset->RelOffset = Delta;
        InstructionPatchJump = InstrPatchJumpTargetRelative(InstructionPatchJump,
                                                            RelativeOffset);
                                                            
        CurrentReturnCount = CurrentReturnCount + 1;
    }
    
    RegisterRt0 = NextAvailableRegister( );
    
    assert(RegisterRt0->Register == REG_RT0);
    
    RegisterRst = RegisterSpecialRegister(REG_RST);
    RegisterRsb = RegisterSpecialRegister(REG_RSB);
    RegisterRct = RegisterIdentifierAsIntegerConstant(Context->DataPointer*-1, Context);
    
    InstructionStep0 = InstrMakeIndirectDirect(OPC_RCOPYD, RegisterRsb, RegisterRct, RegisterRsb);
    InstructionStep1 = InstrMakeStackPop(OPC_POP, RegisterRt0);
    InstructionStep2 = InstrMakeIndirectDirect(OPC_RCOPYD, RegisterRt0, NULL, RegisterRst);
    
    if(((PIDENTIFIER_OBJECT)SQueueTop(Context->Identifier->Parameters))->DataType == IDN_TYPE_VOID) {
        StackCleanupBytes = 0;
    } else {
        StackCleanupBytes = SQueueSize(Context->Identifier->Parameters);
        StackCleanupBytes = StackCleanupBytes * PROGRAM_STACK_ALIGNMENT;
    }
    
    RegisterRct->RelOffset = StackCleanupBytes;
    InstructionStep3 = InstrMakeReturn(OPC_RETURN, RegisterRct);
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionStep0);
    DebugPrettyPrintInstruction(InstructionStep1);
    DebugPrettyPrintInstruction(InstructionStep2);
    DebugPrettyPrintInstruction(InstructionStep3);
#endif
    
    Context->CodePointer = Context->CodePointer + 4*PROGRAM_CODE_ALIGNMENT;
    SQueuePush(InstructionQueue, InstructionStep0);
    SQueuePush(InstructionQueue, InstructionStep1);
    SQueuePush(InstructionQueue, InstructionStep2);
    
    DestroyIdentifier(RegisterRsb);
    DestroyIdentifier(RegisterRct);
    
    SQueuePush(InstructionQueue, InstructionStep3);
    
    DereferenceRegister(RegisterRt0);
    DestroyIdentifier(RegisterRst);
    DestroyIdentifier(RegisterRsb);
    DestroyIdentifier(RegisterRct);
}    

void
GenerateFunctionCallVoidParameter (
    PFUNCTIONCALL_OBJECT FunctionCall,
    PSSTACK OperatorStack,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of validating that the provided function takes a
    single void-type parameter. If this does not hold, the routine terminates 
    execution.
    
 Arguments:
 
    FunctionCall - A pointer to the current function call object.
    
    OperatorStack - A pointer to the operator stack. This is required as a left
                    parentheses is pushed in before each parameter, as in the 
                    case of parameters being expressions, we need to know when 
                    to stop parsing.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    (void)Context;
    assert(Context != Context->GlobalContext);
    
    PIDENTIFIER_OBJECT ParameterVoid;

    assert(FunctionCall->FunctionIdentifier->Register == REG_RFN);
    assert(SQueueSize(FunctionCall->FunctionIdentifier->Parameters) > 0);
    
    ParameterVoid = SQueueTop(FunctionCall->FunctionIdentifier->Parameters);
    if((SQueueSize(FunctionCall->FunctionIdentifier->Parameters) != 1) ||
       (ParameterVoid->DataType != IDN_TYPE_VOID)) {
           
        yyerror(ERR_STR_PARAMMISMATCH);
    }
    
    assert(SStackSize(OperatorStack) > 0);
    assert(((POPERATOR_OBJECT)SStackTop(OperatorStack))->Type == OPR_TYPE_LPAREN);
    
    SStackPop(OperatorStack); // Pop the LPAREN
}
    
void
GenerateFunctionCallPushParameter (
    PFUNCTIONCALL_OBJECT FunctionCall,
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of parsing an expression and pushing the result as
    a parameter to a provided function.
    
 Arguments:
 
    FunctionCall - A pointer to the current function call object.
    
    OperandStack - A pointer to the operand stack.
    
    OperatorStack - A pointer to the operator stack. This is required as a left
                    parentheses is pushed in before each parameter, as in the 
                    case of parameters being expressions, we need to know when 
                    to stop parsing.
                    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    assert(Context != Context->GlobalContext);
    
    PINSTRUCTION InstructionPush;
    PIDENTIFIER_OBJECT ParameterIdentifier;
    
    //
    // By now we already pushed an LPAREN into the operand stack before
    // evaluating the parameter expression, so go ahead and finish parsing it.
    //
    
    GenerateExpressionInstructionsUntilMatch(OperandStack,
                                             OperatorStack,
                                             InstructionQueue,
                                             OPR_TYPE_LPAREN,
                                             Context);
                                             
    ParameterIdentifier = SStackPop(OperandStack);
    FunctionCall->CurrentParameterCount = FunctionCall->CurrentParameterCount + 1;
    if(FunctionCall->CurrentParameterCount > 
       SQueueSize(FunctionCall->FunctionIdentifier->Parameters)) {
    
        yyerror(ERR_STR_EXCESSPARAM);
    }
    
    //
    // TODO: verify type matching
    //
    
    assert(IS_REGISTER_WORKING(ParameterIdentifier->Register)  ||
           IS_REGISTER_INDEX_IX(ParameterIdentifier->Register) ||
           ParameterIdentifier->Register == REG_RST            ||
           ParameterIdentifier->Register == REG_RCT            ||
           ParameterIdentifier->Register == REG_RGD);
    
    InstructionPush = InstrMakeStackPush(OPC_PUSH, ParameterIdentifier);
    SQueuePush(InstructionQueue, InstructionPush);
    Context->CodePointer = Context->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
    DereferenceRegister(ParameterIdentifier);
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionPush);
#endif
}
    
void
GenerateFunctionCallMakeCall (
    PFUNCTIONCALL_OBJECT FunctionCall,
    PSSTACK OperandStack,
    PSQUEUE InstructionQueue,
    PINSTRUCTION *LastCallInstruction,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of generating a call to a function. It is called 
    after all parameters to the function have been parsed and verified. This
    routine additionally copies the return value of the function call into a 
    RTn register for use by the expression parser.
    
 Arguments:
 
    FunctionCall - A pointer to the current function call object.
    
    OperandStack - A pointer to the operand stack.
                    
    InstructionQueue - A pointer to the global instruction queue.
    
    LastCallInstruction - A pointer receiving the generated call instruction.
                          The caller may want to use this in the case it needs
                          to be patched to a parallel type call.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    assert(Context != Context->GlobalContext);
    
    PINSTRUCTION InstructionCall;
    PINSTRUCTION InstructionReturnCopy;
    PIDENTIFIER_OBJECT RegisterInv;
    PIDENTIFIER_OBJECT RegisterRrv;
    PIDENTIFIER_OBJECT RegisterRtn;
    
    InstructionCall = InstrMakeCall(OPC_CALLNORM, FunctionCall->FunctionIdentifier);
    SQueuePush(InstructionQueue, InstructionCall);
    Context->CodePointer = Context->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionCall);
#endif
    
    if(FunctionCall->FunctionIdentifier->ReturnType == IDN_TYPE_VOID) {
        RegisterInv = RegisterSpecialRegister(REG_INV);
        RegisterInv->DataType = IDN_TYPE_ERR;
        SStackPush(OperandStack, RegisterInv);
    } else {
        RegisterRtn = NextAvailableRegister( );
        RegisterRrv = RegisterSpecialRegister(REG_RRV);
        RegisterRrv->RelOffset = 4;
        InstructionReturnCopy = InstrMakeIndirectDirect(OPC_RCOPYD,
                                                        RegisterRrv,
                                                        NULL,
                                                        RegisterRtn);
        
        SStackPush(OperandStack, RegisterRtn);
        SQueuePush(InstructionQueue, InstructionReturnCopy);
        Context->CodePointer = Context->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
        DestroyIdentifier(RegisterRrv);
        
#ifdef COMPILE_VERBOSE
        DebugPrettyPrintInstruction(InstructionReturnCopy);
#endif
    }
    
    *LastCallInstruction = InstructionCall;
}

void
GenerateFunctionReturn (
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSSTACK PendingInstructionStack,
    PSSTACK InstructionCountStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine is in charge of generating a return instruction as well as a 
    pending jump the function exit senquence, to be patched in the function exit
    sequence generation.
    
 Arguments:
    
    OperandStack - A pointer to the operand stack.
    
    OperatorStack - A pointer to the operator stack.
    
    PendingInstructionStack - A pointer to the pending instruction stack.
    
    InstructionCountStack - A pointer to the current instruction count stack. 
                            This to know how many instructions to patch into
                            the jump.
                    
    InstructionQueue - A pointer to the global instruction queue.
    
    Context - A pointer to the current scope context.
    
 Return value:
 
    void.

*/
    
{
    PINSTRUCTION InstructionCopyToRrv;
    PINSTRUCTION InstructionJump;
    PIDENTIFIER_OBJECT RegisterRrv;
    PIDENTIFIER_OBJECT RegisterRct;
    PIDENTIFIER_OBJECT ReturnIdentifier;
    
    //
    // TODO: Type matching & return for void-type functions
    //
    
    GenerateExpressionInstructionsEmptyStacks(OperandStack,
                                              OperatorStack,
                                              InstructionQueue,
                                              Context);
                                              
    ReturnIdentifier = SStackPop(OperandStack);
    RegisterRct = RegisterIdentifierAsIntegerConstant(0, Context);
    RegisterRrv = RegisterSpecialRegister(REG_RRV);
    InstructionCopyToRrv = InstrMakeArithmetic(OPC_ADDI, 
                                               ReturnIdentifier, 
                                               RegisterRct, 
                                               RegisterRrv);
                                               
    InstructionJump = InstrMakeJump(OPC_JMP, NULL);
    SQueuePush(InstructionQueue, InstructionCopyToRrv);
    SStackPush(InstructionCountStack, (void*)SQueueSize(InstructionQueue));
    SQueuePush(InstructionQueue, InstructionJump);
    SStackPush(PendingInstructionStack, InstructionJump);
    Context->CodePointer = Context->CodePointer + 2*PROGRAM_CODE_ALIGNMENT;
    Context->Identifier->ReturnCount = Context->Identifier->ReturnCount + 1;
    DereferenceRegister(ReturnIdentifier);
    DestroyIdentifier(RegisterRrv);
    DestroyIdentifier(RegisterRct);
    
#ifdef COMPILE_VERBOSE
    DebugPrettyPrintInstruction(InstructionCopyToRrv);
    DebugPrettyPrintInstruction(InstructionJump);
#endif
}

void
GenerateFunctionCallPatchParallelSync (
    PINSTRUCTION InstructionCall
    )
    
/*

 Routine description:
 
    This routine patches a call instruction into a parallel sync call 
    instruction.
    
 Arguments:
 
    InstructionCall - The instruction to be patched.
    
 Return value:
 
    void.

*/
    
{
    InstrPatchNormalCallToParallelSync(OPC_CALLPLLS, InstructionCall);
}
    
void
GenerateFunctionCallPatchParallelAsync (
    PINSTRUCTION InstructionCall
    )
    
/*

 Routine description:
 
    This routine patches a call instruction into a parallel async call 
    instruction.
    
 Arguments:
 
    InstructionCall - The instruction to be patched.
    
 Return value:
 
    void.

*/
    
{
    InstrPatchNormalCallToParallelAsync(OPC_CALLPLLA, InstructionCall);
}
