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
    
    register.h
    
 Abstract:
    
    This module defines the routines used to register translation objects
    throughout the translator.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#ifndef __REGISTER_H__
#define __REGISTER_H__

#include "objtypes.h"
#include "../Common/symdef.h"

#define PROGRAM_STACK_TOP       0x7FFFE
#define PROGRAM_DATA_START      0x80000
#define PROGRAM_CODE_START      0xA0000

#define PROGRAM_STACK_ALIGNMENT 0x04
#define PROGRAM_CODE_ALIGNMENT  0x08

#define FUNCTION_IS_MAIN_OK     0
#define FUNCTION_IS_MAIN_ERR    1
#define FUNCTION_IS_NOT_MAIN    2

//
// Identifiers & Operators
//

int
CheckFunctionIsMain (
    PSCOPE_CONTEXT Context
    );

int
CheckIdentifierExists (
    char* Identifier, 
    PSCOPE_CONTEXT Context
    );

int
CheckIdentifierIsArray (
    PIDENTIFIER_OBJECT Identifier
    );

int
CheckIdentifierIsFunction (
    PIDENTIFIER_OBJECT Identifier
    );

PIDENTIFIER_OBJECT
GetDeclaredIdentifier (
    char *Name, 
    PSCOPE_CONTEXT Context
    );

PIDENTIFIER_OBJECT
RegisterIdentifier (
    char* Identifier, 
    IDN_TYPE DataType, 
    PSCOPE_CONTEXT Context
    );

PIDENTIFIER_OBJECT
RegisterIdentifierAsIntegerConstant (
    signed long Value, 
    PSCOPE_CONTEXT 
    Context
    );

PIDENTIFIER_OBJECT
RegisterIdentifierAsFunction (
    PIDENTIFIER_OBJECT Identifier, 
    PSCOPE_CONTEXT Context
    );

PIDENTIFIER_OBJECT
RegisterIdentifierAsVariable (
    PIDENTIFIER_OBJECT Identifier, 
    int IsAtomic,
    PSCOPE_CONTEXT Context
    );

PIDENTIFIER_OBJECT
RegisterArrayToIdentifier (
    PIDENTIFIER_OBJECT Identifier, 
    unsigned long long ArraySize, 
    PSCOPE_CONTEXT Context
    );

PIDENTIFIER_OBJECT
RegisterParameterToIdentifier (
    PIDENTIFIER_OBJECT Identifier, 
    PIDENTIFIER_OBJECT Parameter
    );

PIDENTIFIER_OBJECT
RegisterIdentifierAsParameter (
    char* Identifier, 
    IDN_TYPE DataType, 
    PSCOPE_CONTEXT Context
    );

POPERATOR_OBJECT
RegisterOperator (
    OPR_TYPE Opr
    );

void
DestroyIdentifier (
    PIDENTIFIER_OBJECT Identifier
    );

void
DestroyOperator (
    POPERATOR_OBJECT Operator
    );
    
//
// Function Call
//

PFUNCTIONCALL_OBJECT
RegisterFunctionCall (
    PIDENTIFIER_OBJECT FunctionIdentifier
    );
    
void
DestroyFunctionCall (
    PFUNCTIONCALL_OBJECT FunctionCall
    );
    
//
// I/O
//

PIO_OBJECT
RegisterIoObject (
    void
    );
    
PIO_OBJECT
CleanIoObject (
    PIO_OBJECT IoObject
    );

//
// Registers
//

int
InitializeRegisters (
    void
    );

PIDENTIFIER_OBJECT
ReferenceRegister (
    PIDENTIFIER_OBJECT Register
    );

void
DereferenceRegister (
    PIDENTIFIER_OBJECT Register
    );

PIDENTIFIER_OBJECT
NextAvailableRegister (
    void
    );

PIDENTIFIER_OBJECT
NextAvailableRegisterIndex (
    void
    );

void
FreeAllRegisters (
    void
    );

PIDENTIFIER_OBJECT
RegisterSpecialRegister (
    REGISTER Register
    );

//
// Scope
//

PSCOPE_CONTEXT
CreateScopeContext (
    PIDENTIFIER_OBJECT Identifier, 
    PSCOPE_CONTEXT GlobalContext
    );

void
DestroyScopeContext (
    PSCOPE_CONTEXT Context
    );

void
DebugPrintParameterList (
    PIDENTIFIER_OBJECT Identifier
    );
    
//
// Symbols
//

PFUNCTION_SYMBOL
RegisterFunctionSymbol (
    PSCOPE_CONTEXT Context
    );


#endif // __REGISTER_H__