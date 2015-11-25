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
    
    translator.y
    
 Abstract:
    
    This module implements the grammar and semantics for the translator. It is 
    in charge of parsing the input language and generating the appropriate 
    VM code.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../../utils/inc/squeue.h"
#include "objtypes.h"
#include "../Common/registerdef.h"
#include "opcodes.h"
#include "instruction.h"
#include "register.h"
#include "generator.h"
#include "program.h"
#include "debug.h"
#include "errors.h"

#ifdef __cplusplus
extern "C" {
#endif

int yylex();
int yyparse();
extern FILE *yyin;
extern int yylineno;

#ifdef __cplusplus
}
#endif

#ifndef COMPILE_VERBOSE
FILE* _NUL;
#endif

extern int yyerror(char *err);

OPR_TYPE GOperatorStore[] = { 
    OPR_TYPE_STR 
    };
    
OPR_TYPE GOperatorLog1[] = { 
    OPR_TYPE_LOR, 
    OPR_TYPE_LAND 
    };
                             
OPR_TYPE GOperatorBit1[] = { 
    OPR_TYPE_XOR, 
    OPR_TYPE_OR, 
    OPR_TYPE_AND, 
    OPR_TYPE_NOT 
    };
                            
OPR_TYPE GOperatorLog2[] = { 
    OPR_TYPE_EQ,
     OPR_TYPE_NEQ,
     OPR_TYPE_LT,
     OPR_TYPE_GT,
     OPR_TYPE_LTE,
     OPR_TYPE_GTE 
     };
                             
OPR_TYPE GOperatorAri1[] = { 
    OPR_TYPE_PLUS,                         
    OPR_TYPE_MINUS
    };
    
OPR_TYPE GOperatorAri2[] = { 
    OPR_TYPE_TIMES,
    OPR_TYPE_DIV
    };

PSCOPE_CONTEXT GGlobalContext = NULL;
PSCOPE_CONTEXT GCurrentContext = NULL;

PSQUEUE GInstructionQueue = NULL;
PSSTACK GPendingInstructionStack = NULL;

//
// Uh, so I think I messed this up. My mental picture was wrong, and I'm pretty
// sure now we should only ever have ONE(!) value in this stack.
//

PSSTACK GCurrentIdentifierStack = NULL;

PSSTACK GCurrentExpressionOperandStack = NULL;
PSSTACK GCurrentExpressionOperatorStack = NULL;

//
// This global gets updated each time a new instruction is generated. We use it
// to calculate relative offsets for branches/jumps in non-linear control groups.
//

PSSTACK GCurrentInstructionCountStack = NULL;

//
// These two are used for return logic
//

PSSTACK GPendingReturnJumpsStack = NULL;
PSSTACK GPendingReturnInstructionCountStack = NULL;

PSSTACK GCurrentFunctionCallStack = NULL;
PINSTRUCTION GLastCallInstruction = NULL;

PIO_OBJECT GCurrentIoObject = NULL;

//
// Symbol to export
//

PSQUEUE GFunctionSymbolQueue = NULL;

//
// Address of the main function.
//

unsigned long GMainAddress = 0;

%}

%union {
    int Int;
    float Float;
    char *String;
    
    IDN_TYPE ConstantObjType;
}

%token<Int> TINT;
%token<Float> TFLOAT;
%token<String> TIDENTIFIER;

%type<ConstantObjType> DataType;

// KEYWORDS

/* Loops */
%token<String> TKFOR
%token<String> TKWHILE

/* Conditionals */
%token<String> TKIF
%token<String> TKELSE

/* Threading */
%token<String> TKTHREAD
%token<String> TKAS
%token<String> TKSYNC
%token<String> TKASYNC
%token<String> TKATOMIC

/* Data types */
%token<String> TKVOID
%token<String> TKINT8
%token<String> TKUINT8
%token<String> TKINT16
%token<String> TKUINT16
%token<String> TKINT32
%token<String> TKUINT32
%token<String> TKFLOAT
/* %token<String> TKTHREAD */

/* IO */
%token<String> TKPRINT
%token<String> TKREAD

/* Calls and stuff */
%token<String> TKRETURN

/* Logical two char tokens */
%token<String> TKLOR
%token<String> TKLAND
%token<String> TKEQ
%token<String> TKNEQ
%token<String> TKLEQ
%token<String> TKGEQ

%start Prg

%%

/* Data types */

DataType: TKINT8    { $$ = IDN_TYPE_INT8T; }
    | TKUINT8       { $$ = IDN_TYPE_UINT8T; }
    | TKINT16       { $$ = IDN_TYPE_INT16T; }
    | TKUINT16      { $$ = IDN_TYPE_UINT16T; }
    | TKINT32       { $$ = IDN_TYPE_INT32T; }
    | TKUINT32      { $$ = IDN_TYPE_UINT32T; }
    | TKFLOAT       { $$ = IDN_TYPE_FLOATT; }
    | TKTHREAD      { $$ = IDN_TYPE_THREADT; }
    ;

/* Program */

Prg: PrgSub1
    | VarDecl Prg
    ;

PrgSub1: FuncDecl PrgSub2
    ;
    
PrgSub2: /* empty */
    | PrgSub1
    ;
    
/* Function and variable declaration common header */

FuncVarDeclHdr: 
    DataType 
    TIDENTIFIER 
    {
        PIDENTIFIER_OBJECT Identifier;
        
        if(CheckIdentifierExists($2, GCurrentContext) != 0) {
            yyerror(ERR_STR_REDECLARED);
        }
        
        Identifier = RegisterIdentifier($2, $1, GCurrentContext);
        if(Identifier == NULL) {
            yyerror(ERR_STR_NOMEM);
        }
        
        SStackPush(GCurrentIdentifierStack, Identifier);
    }
    ;
    
/* Function Declaration */

FuncDecl: 
    FuncDeclTypeHdr 
    '(' 
    {
        PIDENTIFIER_OBJECT ThisIdentifier = SStackTop(GCurrentIdentifierStack);
        PSCOPE_CONTEXT ScopeContext = CreateScopeContext(ThisIdentifier, GGlobalContext);
        if(ScopeContext == NULL) {
            yyerror(ERR_STR_NOMEM);
        }
        
        GCurrentContext = ScopeContext;
        RegisterIdentifierAsFunction(SStackTop(GCurrentIdentifierStack), GGlobalContext);
    }
    FuncDeclSub1 
    ')'
    {
        PFUNCTION_SYMBOL FunctionSymbol;
        signed IsMainFunction;
                
        (void)SStackPop(GCurrentIdentifierStack);

        IsMainFunction = CheckFunctionIsMain(GCurrentContext);
        switch(IsMainFunction) {
        case FUNCTION_IS_MAIN_OK:
            GMainAddress = GGlobalContext->CodePointer;
            fprintf(_NUL,
                    "Found main function at start address 0x%p\n", 
                    (void*)(unsigned long long)GMainAddress);
            break;
        case FUNCTION_IS_MAIN_ERR:
            yyerror(ERR_STR_INCORRECTMAIN);
            break;
        case FUNCTION_IS_NOT_MAIN:
        default:
            break;
        }
        
        GenerateFunctionHeaderStage0(GPendingInstructionStack, 
                                     GInstructionQueue, 
                                     GCurrentContext);
        
        FunctionSymbol = RegisterFunctionSymbol(GCurrentContext);
        if(FunctionSymbol == NULL) {
            yyerror(ERR_STR_NOMEM);
        }
        
        SQueuePush(GFunctionSymbolQueue, FunctionSymbol);
    }
    Block
    {
        GenerateFunctionHeaderStage1(GPendingInstructionStack, 
                                     GInstructionQueue, 
                                     GCurrentContext);
                                     
        GenerateFunctionTrailer(GPendingReturnJumpsStack,
                                GPendingReturnInstructionCountStack,
                                GInstructionQueue,
                                GCurrentContext);
        
        
        //
        // In Context->CodePointer we hold the effective size of the function,
        // by which we have to increment the global context StackPointer so we 
        // can locate the next function at a suitable offset. We need to add an
        // additional PROGRAM_CODE_ALIGNMENT so the next function doesn't overlap
        // the last instruction.
        //
        // Nov/14: On second note, do not add PROGRAM_CODE_ALIGNMENT, it seems to
        // cause an off-by-one in instruction jumps.
        //
        
        GGlobalContext->CodePointer += 
            (GCurrentContext->CodePointer - GGlobalContext->CodePointer);// + PROGRAM_CODE_ALIGNMENT;
        DestroyScopeContext(GCurrentContext);
        GCurrentContext = GGlobalContext;
    }
    ;
    
FuncDeclTypeHdr: 
    FuncVarDeclHdr 
    { }
    | 
    TKVOID TIDENTIFIER
    {
        //
        // GCurrentContext should ALWAYS be GGlobalContext here.
        //
        
        if(CheckIdentifierExists($2, GCurrentContext) != 0) {
            yyerror(ERR_STR_REDECLARED);
        }
        
        SStackPush(GCurrentIdentifierStack, 
                    RegisterIdentifier($2, IDN_TYPE_VOID, GCurrentContext));
        if(SStackTop(GCurrentIdentifierStack) == NULL) {
            yyerror(ERR_STR_NOMEM);
        }
    }
    ;

FuncDeclSub1: 
    TKVOID
    {
        PIDENTIFIER_OBJECT Parameter = RegisterIdentifierAsParameter("", IDN_TYPE_VOID, GCurrentContext);
        RegisterParameterToIdentifier(SStackTop(GCurrentIdentifierStack), Parameter);
    }
    | 
    FuncDeclSub2
    { }
    ;

FuncDeclSub2: 
    DataType 
    TIDENTIFIER 
    {
        if(CheckIdentifierExists($2, GCurrentContext) != 0) {
            yyerror(ERR_STR_REDECLARED);
        }
        
        PIDENTIFIER_OBJECT Parameter = RegisterIdentifierAsParameter($2, $1, GCurrentContext);
        RegisterParameterToIdentifier(SStackTop(GCurrentIdentifierStack), Parameter);
    }
    FuncDeclSub3
    { }
    ;
    
FuncDeclSub3: 
    /* empty */   
    | ',' FuncDeclSub2  
    ;
    
/* Function Call */

FuncCall: 
    TIDENTIFIER 
    { }
    '('
    {
        PIDENTIFIER_OBJECT FuncIdentifier;
        PFUNCTIONCALL_OBJECT FunctionCall;
        
        //
        // So we can be here from an expression evaluation or from a PLL call,
        // we find out by checking if the operand stack is empty. If it's not,
        // then $1 should be equal to the tops name
        //
        
        if(!CheckIdentifierExists($1, GGlobalContext)) {
            yyerror(ERR_STR_UNDECLARED);
        }
        
        FuncIdentifier = GetDeclaredIdentifier($1, GGlobalContext);
        
        if(!CheckIdentifierIsFunction(FuncIdentifier)) {
            yyerror(ERR_STR_NOTFUNC);
        }
        
        FunctionCall = RegisterFunctionCall(FuncIdentifier);
        if(FunctionCall == NULL) {
            yyerror(ERR_STR_NOMEM);
        }
        
        SStackPush(GCurrentFunctionCallStack, FunctionCall);
        
        //
        // Function call parameters derive from the same operand and operator 
        // stacks as those for regular expressions. We use these stacks to 
        // determine the expression to evaluate for a given parameter, so we 
        // need to know when to stop evaluating, so we insert an LPAREN here as
        // a stopper.
        //
        
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_LPAREN));
    }
    FuncCallSub1 
    ')'
    {
        PFUNCTIONCALL_OBJECT FunctionCall;
       
        assert(SStackSize(GCurrentFunctionCallStack) >= 1);
        
        FunctionCall = SStackPop(GCurrentFunctionCallStack);
        GenerateFunctionCallMakeCall(FunctionCall,
                                     GCurrentExpressionOperandStack,
                                     GInstructionQueue, 
                                     &GLastCallInstruction,
                                     GCurrentContext);
        
        DestroyFunctionCall(FunctionCall);
    }
    ;
    
FuncCallSub1: 
    TKVOID
    {
        PFUNCTIONCALL_OBJECT FunctionCall;
        
        FunctionCall = SStackTop(GCurrentFunctionCallStack);
        GenerateFunctionCallVoidParameter(FunctionCall, 
                                          GCurrentExpressionOperatorStack,
                                          GCurrentContext);
    }
    | 
    FuncCallSub2
    ;
    
FuncCallSub2: 
    Exp
    {
        PFUNCTIONCALL_OBJECT FunctionCall;
        
        FunctionCall = SStackTop(GCurrentFunctionCallStack);
        GenerateFunctionCallPushParameter(FunctionCall,
                                          GCurrentExpressionOperandStack,
                                          GCurrentExpressionOperatorStack,
                                          GInstructionQueue,
                                          GCurrentContext);
    }
    FuncCallSub3
    ;
    
FuncCallSub3: /* empty */
    | 
    ','
    {
        //
        // Just like the first parameter, the next one will need a form of
        // stopper to know when to stop parsing, we again use RPAREN
        //
        
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_LPAREN));
    }
    FuncCallSub2
    ;
    
FuncCallPll: FuncCall FuncCallPllSub1
    ; 
    
FuncCallPllSub1: TKAS TKTHREAD FuncCallPllSub2
    ;
    
FuncCallPllSub2: /* empty */
    | 
    TKSYNC
    {
        GenerateFunctionCallPatchParallelSync(GLastCallInstruction);
    }
    | 
    TKASYNC
    {
        GenerateFunctionCallPatchParallelAsync(GLastCallInstruction);
    }
    ;
    
/* Variable Declaration */

VarDeclHdrAtomic:
    TKATOMIC
    DataType
    TIDENTIFIER
    {
        PIDENTIFIER_OBJECT Identifier;
        
        if(CheckIdentifierExists($3, GCurrentContext) != 0) {
            yyerror(ERR_STR_REDECLARED);
        }
        
        Identifier = RegisterIdentifier($3, $2, GCurrentContext);
        if(Identifier == NULL) {
            yyerror(ERR_STR_NOMEM);
        }
        
        Identifier = RegisterIdentifierAsVariable(Identifier, 1, GCurrentContext);
        SStackPush(GCurrentIdentifierStack, Identifier);
    }
    
VarDeclHdr:
    VarDeclHdrAtomic
    |
    FuncVarDeclHdr
    {
        PIDENTIFIER_OBJECT Identifier;
        
        Identifier = SStackTop(GCurrentIdentifierStack);
        RegisterIdentifierAsVariable(Identifier, 0, GCurrentContext);
    }

VarDecl:
    VarDeclHdr
    VarDeclSub1 
    ';'
    {
        assert(SStackSize(GCurrentIdentifierStack) == 1);
        
        SStackPop(GCurrentIdentifierStack);
    }
    ;
    
VarDeclSub1: 
    VarDeclSub2 
    VarDeclSub3
    ;

VarDeclSub2: /* empty */ 
    { }
    | 
    '[' TINT ']' 
    {
        PIDENTIFIER_OBJECT CurrentIdentifier;
        
        if($2 <= 0) {
            yyerror(ERR_STR_NOTPOSARRAY);
        }
        
        CurrentIdentifier = SStackTop(GCurrentIdentifierStack);
        if(CurrentIdentifier->IsAtomic) {
            yyerror(ERR_STR_NOATOMICARR);
        }
        
        RegisterArrayToIdentifier(CurrentIdentifier, $2, GCurrentContext);
    }
    ;
    
VarDeclSub3: /* empty */
    |
    ',' 
    TIDENTIFIER
    {        
        PIDENTIFIER_OBJECT CurrentIdentifier;
        PIDENTIFIER_OBJECT NewIdentifier;
        
        if(CheckIdentifierExists($2, GCurrentContext) != 0) {
            yyerror(ERR_STR_REDECLARED);
        }
        
        CurrentIdentifier = SStackPop(GCurrentIdentifierStack);
        NewIdentifier = RegisterIdentifier($2, 
                                           CurrentIdentifier->DataType, 
                                           GCurrentContext);
        
        NewIdentifier = RegisterIdentifierAsVariable(NewIdentifier,
                                                     CurrentIdentifier->IsAtomic,
                                                     GCurrentContext);
        
        SStackPush(GCurrentIdentifierStack, NewIdentifier);
    }
    VarDeclSub1
    { }
    ;
    
/* Block */

Block: '{' BlockSub1 '}'
    ;

BlockSub1: ';'
    | /* empty */
    | BlockBody BlockSub2
    ;

BlockSub2: /* empty */
    | BlockBody BlockSub1
    ;
    
BlockBody: VarDecl
    | 
    FuncCallPll ';' 
    {
        PIDENTIFIER_OBJECT ReturnIdentifier;
        
        assert(SStackSize(GCurrentExpressionOperandStack) == 1);
        assert(SStackSize(GCurrentExpressionOperatorStack) == 0);
        
        ReturnIdentifier = SStackPop(GCurrentExpressionOperandStack);
        DereferenceRegister(ReturnIdentifier);
    }
    |
    Exp 
    ';'
    {
        GenerateExpressionInstructionsEmptyStacks(GCurrentExpressionOperandStack,
                                                  GCurrentExpressionOperatorStack,
                                                  GInstructionQueue,
                                                  GCurrentContext);
        
        SStackPop(GCurrentExpressionOperandStack);
        
        //
        // I think at this point we should be able to clear all working
        // registers. Lets see if this causes a problem down the line?
        //
        
        FreeAllRegisters( );
    }
    | Cond
    | FLoop
    | WLoop
    | Return
    | IO ';'
    
/* I/O */

IO:
    IoPrint
    |
    IoRead

IoPrint:
    TKPRINT 
    '(' 
    Exp
    {
        GenerateIoAddPrintIdentifier(GCurrentIoObject,
                                     GCurrentExpressionOperandStack,
                                     GCurrentExpressionOperatorStack,
                                     GInstructionQueue,
                                     GCurrentContext);
    }
    IoPrintSub1 
    ')'
    {
        GenerateIoFinishPrint(GCurrentIoObject, 
                             GInstructionQueue, 
                             GCurrentContext);
    }

IoPrintSub1: /* empty */
    |
    ',' 
    Exp 
    {
        GenerateIoAddPrintIdentifier(GCurrentIoObject,
                                     GCurrentExpressionOperandStack,
                                     GCurrentExpressionOperatorStack,
                                     GInstructionQueue,
                                     GCurrentContext);
    }
    IoPrintSub1
    
IoRead:
    TKREAD 
    '(' 
    TIDENTIFIER
    {
        PIDENTIFIER_OBJECT Identifier;
        
        Identifier = GetDeclaredIdentifier($3, GCurrentContext);
        if(Identifier == NULL) {
            yyerror(ERR_STR_UNDECLARED);
        }
        
        GenerateIoAddReadIdentifier(GCurrentIoObject, 
                                    Identifier, 
                                    GInstructionQueue,
                                    GCurrentContext);
    }
    IoReadSub1 
    ')'
    {
        GenerateIoFinishRead(GCurrentIoObject, 
                             GInstructionQueue, 
                             GCurrentContext);
    }

IoReadSub1: /* empty */
    |
    ',' 
    TIDENTIFIER
    {
        PIDENTIFIER_OBJECT Identifier;
        
        Identifier = GetDeclaredIdentifier($2, GCurrentContext);
        if(Identifier == NULL) {
            yyerror(ERR_STR_UNDECLARED);
        }
        
        GenerateIoAddReadIdentifier(GCurrentIoObject, 
                                    Identifier, 
                                    GInstructionQueue,
                                    GCurrentContext);
    }
    IoReadSub1
    
/* Function return */
    
Return: 
    TKRETURN 
    Exp 
    ';'
    {
        GenerateFunctionReturn(GCurrentExpressionOperandStack,
                               GCurrentExpressionOperatorStack,
                               GPendingReturnJumpsStack,
                               GPendingReturnInstructionCountStack,
                               GInstructionQueue,
                               GCurrentContext)
    }
    
/* Loops */
    
FLoop: 
    TKFOR 
    '(' 
    FLoopSub1
    {
        //
        // Initializer
        //
        
        if(SStackSize(GCurrentExpressionOperandStack) > 0) {
            GenerateExpressionInstructionsEmptyStacks(GCurrentExpressionOperandStack,
                                                      GCurrentExpressionOperatorStack,
                                                      GInstructionQueue,
                                                      GCurrentContext);
            SStackPop(GCurrentExpressionOperandStack);
        }
        
        SStackPush(GCurrentInstructionCountStack, 
                   (void*)SQueueSize(GInstructionQueue));
    }
    ';' 
    FLoopSub1 
    {
        //
        // Condition
        //
        
        
        
        PINSTRUCTION Instruction;
        PIDENTIFIER_OBJECT ZeroCheckRegister;
        
        if(SStackSize(GCurrentExpressionOperandStack) > 0) {
            GenerateExpressionInstructionsEmptyStacks(GCurrentExpressionOperandStack,
                                                      GCurrentExpressionOperatorStack,
                                                      GInstructionQueue,
                                                      GCurrentContext);
            
            ZeroCheckRegister = SStackPop(GCurrentExpressionOperandStack);
        } else {
            
            //
            // RIP is never 0. Or I mean, I guess it can be but you would have 
            // bigger problems then.
            //
            
            ZeroCheckRegister = RegisterSpecialRegister(REG_RIP);
        }
        
        Instruction = InstrMakeJumpConditional(OPC_JMPZ,
                                               NULL,
                                               ZeroCheckRegister);

        SQueuePush(GInstructionQueue, Instruction);           
        GCurrentContext->CodePointer = GCurrentContext->CodePointer + 
                                       1*PROGRAM_CODE_ALIGNMENT;
                                       
        SStackPush(GPendingInstructionStack, Instruction);
        SStackPush(GCurrentInstructionCountStack, 
                   (void*)SQueueSize(GInstructionQueue));
    }
    ';' 
    FLoopSub1 
    {
        //
        // Modifier
        //
        
        if(SStackSize(GCurrentExpressionOperandStack) > 0) {
            GenerateExpressionInstructionsEmptyStacks(GCurrentExpressionOperandStack,
                                                      GCurrentExpressionOperatorStack,
                                                      GInstructionQueue,
                                                      GCurrentContext);
            SStackPop(GCurrentExpressionOperandStack);
        }
    }
    ')' 
    Block
    {
        PINSTRUCTION ForInstruction;
        PINSTRUCTION ExitInstruction;
        PIDENTIFIER_OBJECT RelativeOffset;
        signed ForDelta;
        signed ExitDelta;
        
        ExitInstruction = SStackPop(GPendingInstructionStack);
        ExitDelta = SQueueSize(GInstructionQueue) - 
                     (size_t)SStackPop(GCurrentInstructionCountStack) + 2;
                     
        ForDelta = SQueueSize(GInstructionQueue) - 
                     (size_t)SStackPop(GCurrentInstructionCountStack);
                     
        ExitDelta = ExitDelta * PROGRAM_CODE_ALIGNMENT;
        ForDelta = ForDelta * PROGRAM_CODE_ALIGNMENT;
          
        RelativeOffset = RegisterSpecialRegister(REG_RIP);
        RelativeOffset->RelOffset = ExitDelta;
                
        InstrPatchJumpConditionalAddTargetRelative(ExitInstruction, RelativeOffset);
        RelativeOffset->RelOffset = ForDelta*-1;
        ForInstruction = InstrMakeJump(OPC_JMP, RelativeOffset);
        SQueuePush(GInstructionQueue, ForInstruction);
        GCurrentContext->CodePointer = GCurrentContext->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
    }
    ;

FLoopSub1: /* empty */
    | Exp
    ;
    
WLoop: 
    TKWHILE 
    {
        SStackPush(GCurrentInstructionCountStack, 
                   (void*)SQueueSize(GInstructionQueue));
    }
    '('
    Exp 
    ')'
    {
        PINSTRUCTION Instruction;
        PIDENTIFIER_OBJECT ZeroCheckRegister;
        
        GenerateExpressionInstructionsEmptyStacks(GCurrentExpressionOperandStack,
                                                  GCurrentExpressionOperatorStack,
                                                  GInstructionQueue,
                                                  GCurrentContext);
        
        ZeroCheckRegister = SStackPop(GCurrentExpressionOperandStack);                    
        Instruction = InstrMakeJumpConditional(OPC_JMPZ,
                                               NULL,
                                               ZeroCheckRegister);
                                               
        SQueuePush(GInstructionQueue, Instruction);      
        GCurrentContext->CodePointer = GCurrentContext->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;             
        SStackPush(GPendingInstructionStack, Instruction);
        SStackPush(GCurrentInstructionCountStack, 
                   (void*)SQueueSize(GInstructionQueue));
    }
    Block
    {
        PINSTRUCTION WhileInstruction;
        PINSTRUCTION ExitInstruction;
        PIDENTIFIER_OBJECT RelativeOffset;
        signed WhileDelta;
        signed ExitDelta;
        
        ExitInstruction = SStackPop(GPendingInstructionStack);
        ExitDelta = SQueueSize(GInstructionQueue) - 
                     (size_t)SStackPop(GCurrentInstructionCountStack) + 2;
                     
        WhileDelta = SQueueSize(GInstructionQueue) - 
                     (size_t)SStackPop(GCurrentInstructionCountStack);
                     
        ExitDelta = ExitDelta * PROGRAM_CODE_ALIGNMENT;
        WhileDelta = WhileDelta * PROGRAM_CODE_ALIGNMENT;
                
        RelativeOffset = RegisterSpecialRegister(REG_RIP);
        RelativeOffset->RelOffset = ExitDelta;
                
        InstrPatchJumpConditionalAddTargetRelative(ExitInstruction, RelativeOffset);
        RelativeOffset->RelOffset = WhileDelta*-1;
        WhileInstruction = InstrMakeJump(OPC_JMP, RelativeOffset);
        SQueuePush(GInstructionQueue, WhileInstruction);
        GCurrentContext->CodePointer = GCurrentContext->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
    }
    ;
    
/* Conditionals */

Cond: 
    TKIF 
    '(' 
    Exp 
    ')'
    {
        PINSTRUCTION Instruction;
        PIDENTIFIER_OBJECT ZeroCheckRegister;
        
        //
        // Empty the expression stacks and push the current instruction count
        // into the count stack. We will later use this to patch the JMPZ 
        // instruction.
        //
        
        GenerateExpressionInstructionsEmptyStacks(GCurrentExpressionOperandStack,
                                                  GCurrentExpressionOperatorStack,
                                                  GInstructionQueue,
                                                  GCurrentContext);
                                                  
        ZeroCheckRegister = SStackPop(GCurrentExpressionOperandStack);                    
        Instruction = InstrMakeJumpConditional(OPC_JMPZ,
                                               NULL,
                                               ZeroCheckRegister);
        SQueuePush(GInstructionQueue, Instruction);
        GCurrentContext->CodePointer = GCurrentContext->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
        SStackPush(GCurrentInstructionCountStack, 
                   (void*)SQueueSize(GInstructionQueue));
                   
        SStackPush(GPendingInstructionStack, Instruction);
    }
    Block 
    CondSub1
    ;
    
CondSub1: /* empty */
    {
        PINSTRUCTION Instruction;
        PIDENTIFIER_OBJECT RelativeOffset;
        signed InstructionDelta;
                           
        Instruction = SStackPop(GPendingInstructionStack);
        InstructionDelta = SQueueSize(GInstructionQueue) - 
                           (size_t)SStackPop(GCurrentInstructionCountStack) + 1;
                                                             
        InstructionDelta = InstructionDelta * PROGRAM_CODE_ALIGNMENT;                 
        RelativeOffset = RegisterIdentifierAsIntegerConstant(InstructionDelta, 
                                                             GCurrentContext);     
        Instruction = InstrPatchJumpConditionalAddTargetRelative(Instruction, 
                                                                 RelativeOffset);
        
        DestroyIdentifier(RelativeOffset);
    }
    | 
    TKELSE
    {
        PINSTRUCTION IfInstruction;
        PINSTRUCTION ElseInstruction;
        PIDENTIFIER_OBJECT RelativeOffset;
        signed InstructionDelta;        
        
        //
        // We need to generate another jump here in the case the if condition
        // evaluates to true, as well as patching the if jump to this else.
        //
        
        IfInstruction = SStackPop(GPendingInstructionStack);
        ElseInstruction = InstrMakeJump(OPC_JMP, NULL);
        SQueuePush(GInstructionQueue, ElseInstruction);
        GCurrentContext->CodePointer = GCurrentContext->CodePointer + 1*PROGRAM_CODE_ALIGNMENT;
        SStackPush(GPendingInstructionStack, ElseInstruction);
        InstructionDelta = SQueueSize(GInstructionQueue) - 
                           (size_t)SStackPop(GCurrentInstructionCountStack) + 1;
        
        SStackPush(GCurrentInstructionCountStack, 
                   (void*)SQueueSize(GInstructionQueue));
                   
        InstructionDelta = InstructionDelta * PROGRAM_CODE_ALIGNMENT;
        RelativeOffset = RegisterIdentifierAsIntegerConstant(InstructionDelta, 
                                                             GCurrentContext);
                                                             
        InstrPatchJumpConditionalAddTargetRelative(IfInstruction, 
                                                   RelativeOffset);
        
        DestroyIdentifier(RelativeOffset);
    }
    Block
    {
        PINSTRUCTION Instruction;
        PIDENTIFIER_OBJECT RelativeOffset;
        signed InstructionDelta;
                           
        Instruction = SStackPop(GPendingInstructionStack);
        InstructionDelta = SQueueSize(GInstructionQueue) - 
                           (size_t)SStackPop(GCurrentInstructionCountStack) + 1;
                           
        InstructionDelta = InstructionDelta * PROGRAM_CODE_ALIGNMENT;                  
        RelativeOffset = RegisterIdentifierAsIntegerConstant(InstructionDelta, 
                                                             GCurrentContext);
        
        Instruction = InstrPatchJumpTargetRelative(Instruction, 
                                                   RelativeOffset);
        
        DestroyIdentifier(RelativeOffset);
    }
    ;

/* Expressions */

ExpOpAss: 
    '='
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_STR));
    }
    ;
    
ExpOpLog1: 
    TKLOR
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_LOR));
    }
    | 
    TKLAND
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_LAND));
    }
    ;

ExpOpBit1: 
    '^'
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_XOR));
    }
    | 
    '|'
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_OR));
    }
    | 
    '&'
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_AND));
    }
    | 
    '~'
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_NOT));
    }
    ;
    
ExpOpLog2: 
    TKEQ
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_EQ));
    }
    | 
    TKNEQ
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_NEQ));
    }
    | 
    '<'
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_LT));
    }
    | 
    '>'
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_GT));
    }
    | 
    TKLEQ
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_LTE));
    }
    | 
    TKGEQ
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_GTE));
    }
    ;
    
ExpOpAri1: 
    '+'  
    { 
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_PLUS)); 
    }
    | 
    '-'
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_MINUS)); 
    }
    ;
    
ExpOpAri2: 
    '*'
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_TIMES));
    }
    | 
    '/'
    {
        SStackPush(GCurrentExpressionOperatorStack, 
                   RegisterOperator(OPR_TYPE_DIV));
    }
    ;

Exp: ExpPrio1
    ;

ExpPrio1: 
    ExpPrio2
    ExpPrio1Sub1
    {
        //
        // The assignment operator has right-precedence.
        //
        
        GenerateExpressionInstructions(GCurrentExpressionOperandStack,
                                       GCurrentExpressionOperatorStack,
                                       GInstructionQueue,
                                       GOperatorStore,
                                       sizeof(GOperatorStore)/sizeof(GOperatorStore[0]),
                                       GCurrentContext);
    }
    ;

ExpPrio1Sub1: /* empty */
    | ExpOpAss ExpPrio1
    ;
    
ExpPrio2: 
    ExpPrio3
    {
        GenerateExpressionInstructions(GCurrentExpressionOperandStack,
                                       GCurrentExpressionOperatorStack,
                                       GInstructionQueue,
                                       GOperatorLog1,
                                       sizeof(GOperatorLog1)/sizeof(GOperatorLog1[0]),
                                       GCurrentContext);
    }
    ExpPrio2Sub1
    ;

ExpPrio2Sub1: /* empty */
    | ExpOpLog1 ExpPrio2
    ;
    
ExpPrio3: 
    ExpPrio4
    {
        GenerateExpressionInstructions(GCurrentExpressionOperandStack,
                                       GCurrentExpressionOperatorStack,
                                       GInstructionQueue,
                                       GOperatorBit1,
                                       sizeof(GOperatorBit1)/sizeof(GOperatorBit1[0]),
                                       GCurrentContext); 
    }
    ExpPrio3Sub1
    ;

ExpPrio3Sub1: /* empty */
    | ExpOpBit1 ExpPrio3
    ;
    
ExpPrio4: 
    ExpPrio5
    {
        GenerateExpressionInstructions(GCurrentExpressionOperandStack,
                                       GCurrentExpressionOperatorStack,
                                       GInstructionQueue,
                                       GOperatorLog2,
                                       sizeof(GOperatorLog2)/sizeof(GOperatorLog2[0]),
                                       GCurrentContext); 
    }
    ExpPrio4Sub1
    ;

ExpPrio4Sub1: /* empty */
    | ExpOpLog2 ExpPrio4
    ;
    
ExpPrio5: 
    ExpPrio6
    {
        GenerateExpressionInstructions(GCurrentExpressionOperandStack,
                                       GCurrentExpressionOperatorStack,
                                       GInstructionQueue,
                                       GOperatorAri1,
                                       sizeof(GOperatorAri1)/sizeof(GOperatorAri1[0]),
                                       GCurrentContext); 
    }
    ExpPrio5Sub1
    ;

ExpPrio5Sub1: /* empty */
    | ExpOpAri1 ExpPrio5
    ;
    
ExpPrio6: 
    ExpPrio7
    {
        GenerateExpressionInstructions(GCurrentExpressionOperandStack,
                                       GCurrentExpressionOperatorStack,
                                       GInstructionQueue,
                                       GOperatorAri2,
                                       sizeof(GOperatorAri2)/sizeof(GOperatorAri2[0]),
                                       GCurrentContext); 
    }
    ExpPrio6Sub1
    ;

ExpPrio6Sub1: /* empty */
    | ExpOpAri2 ExpPrio6
    ;
    
ExpPrio7: 
    TINT
    {
        SStackPush(GCurrentExpressionOperandStack, 
            RegisterIdentifierAsIntegerConstant($1, GCurrentContext));
    }
    | 
    TFLOAT
    {
        yyerror(ERR_STR_FLOATUSED);
    }
    | 
    '('
    {
        SStackPush(GCurrentExpressionOperatorStack, RegisterOperator(OPR_TYPE_LPAREN));
    }
    ExpPrio1 
    ')'   
    {
        //
        // Pop everything until we reach the LPAREN ?
        //
        
        GenerateExpressionInstructionsUntilMatch(GCurrentExpressionOperandStack,
                                                 GCurrentExpressionOperatorStack,
                                                 GInstructionQueue,
                                                 OPR_TYPE_LPAREN,
                                                 GCurrentContext);
    }
    | 
    FuncCall
    | 
    TIDENTIFIER
    {
        PIDENTIFIER_OBJECT Identifier = GetDeclaredIdentifier($1, GCurrentContext);
        if(Identifier == NULL) {
            yyerror(ERR_STR_UNDECLARED);
        }
        
        SStackPush(GCurrentExpressionOperandStack, Identifier);
    }
    ExpPrio7Sub1
    ;
    
ExpPrio7Sub1: /* empty */
    | 
    '['
    {
        //
        // Ok, we want to access this identifier as an array, so aside from 
        // making sure that it is an array we need to generate an instruction
        // that will add an offset to the base of the identifier that we may
        // then dereference once the index expression is evaluated.
        //
        
        PIDENTIFIER_OBJECT ArrayIdentifier;
        
        ArrayIdentifier = SStackTop(GCurrentExpressionOperandStack);
        
        if(!CheckIdentifierIsArray(ArrayIdentifier)) {
            yyerror(ERR_STR_NOTARRAY);
        }
        
        SStackPush(GCurrentExpressionOperatorStack, RegisterOperator(OPR_TYPE_LBRACK));
    }
    ExpPrio1 
    ']'
    {
        //
        // Pop everything until we reach the LBRACK ?
        //
        
        GenerateExpressionInstructionsUntilMatch(GCurrentExpressionOperandStack,
                                                 GCurrentExpressionOperatorStack,
                                                 GInstructionQueue,
                                                 OPR_TYPE_LBRACK,
                                                 GCurrentContext);
        
        GenerateArrayInstructions(GCurrentExpressionOperandStack,
                                  GInstructionQueue,
                                  GCurrentContext);
    }
    ;
    
%%

int main(int argc, char** argv) {
    
    FILE *SourceFile;
    FILE *CompileFile;
    
#ifndef COMPILE_VERBOSE
    _NUL = fopen("nul", "w");
    if(_NUL == NULL) {
        yyerror(ERR_STR_NULOPENFAIL);
    }
#endif
    
    //
    // Open the source and compile files. No real point doing the work if we
    // can't open either of them right?
    //
    // Also, it would be nice if we had a command line parser.
    //
    
    ProgramOpenInputOutputFiles("src.ut", "out.cut", &SourceFile, &CompileFile);
    if(SourceFile == NULL) {
        yyerror(ERR_STR_FILEINOPEN); 
    }
    
    if(CompileFile == NULL) {
        yyerror(ERR_STR_FILEOUTOPEN);
    }
    
    yyin = SourceFile;
    
    //
    // Initialize the stacks, globals, etc.
    //
    
    GGlobalContext = CreateScopeContext(NULL, NULL);
    SStackInitialize(&GCurrentIdentifierStack);
    SStackInitialize(&GCurrentExpressionOperandStack);
    SStackInitialize(&GCurrentExpressionOperatorStack);
    SStackInitialize(&GPendingInstructionStack);
    SStackInitialize(&GCurrentInstructionCountStack);
    SStackInitialize(&GPendingReturnJumpsStack);
    SStackInitialize(&GPendingReturnInstructionCountStack);
    SStackInitialize(&GCurrentFunctionCallStack);
    SQueueInitialize(&GInstructionQueue);
    SQueueInitialize(&GFunctionSymbolQueue);
    GCurrentIoObject = RegisterIoObject( );
    if(GGlobalContext == NULL                   || 
       GCurrentIdentifierStack == NULL          ||
       GCurrentExpressionOperandStack == NULL   ||
       GCurrentExpressionOperatorStack == NULL  ||
       GPendingInstructionStack == NULL         ||
       GCurrentInstructionCountStack == NULL    ||
       GPendingReturnJumpsStack == NULL         ||
       GPendingReturnInstructionCountStack == NULL ||
       GCurrentFunctionCallStack == NULL        ||
       GInstructionQueue == NULL                ||
       GFunctionSymbolQueue == NULL             ||
       GCurrentIoObject == NULL) {
           
        yyerror(ERR_STR_NOMEM);
    }
    
    GCurrentContext = GGlobalContext;
    
    //
    // Initialize the registers
    //
    
    if(InitializeRegisters() != 0) {
        yyerror(ERR_STR_PROGRAMINIT);
    }
    
    //
    // Create the start block
    //
    
    GenerateProgramStartBlockStage0(GPendingInstructionStack,
                                    GInstructionQueue,
                                    GGlobalContext);
    
	do {
		yyparse();
	} while (!feof(yyin));

    if(GMainAddress == 0) {
        yyerror(ERR_STR_NOMAINFUNCTION);
    }
    
    GenerateProgramStartBlockStage1(GPendingInstructionStack,
                                    GInstructionQueue,
                                    GMainAddress,
                                    GGlobalContext);
    
    ProgramSerializeCode(CompileFile, 
                         GInstructionQueue,
                         GFunctionSymbolQueue,
                         GGlobalContext);
                         
#ifndef COMPILE_VERBOSE
    fclose(_NUL);
#endif	
    return 0;
}

int yyerror(char *err) {
	printf("Line %d: Error: %s\n", yylineno, err);
	exit(-1);
}
