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
    
    This module implements the routines used to register translation objects
    throughout the translator.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15
    
 Revision:
 
    11/17/15        Initial Creation
    11/25/15        Documented functions

**/

#include "register.h"
#include "../Common/registerdef.h"
#include "../../utils/inc/squeue.h"
#include "../../utils/inc/sstack.h"
#include "../../utils/inc/shashmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef COMPILE_VERBOSE
#define PRINT_OUT       stdout
#else
extern FILE* _NUL;
#define PRINT_OUT       _NUL
#endif

typedef struct _WORKING_REGISTERS {
    PIDENTIFIER_OBJECT Registers;
    PIDENTIFIER_OBJECT RegistersIndex;
    unsigned NextAvailable;
    unsigned NextAvailableIndex;
} WORKING_REGISTERS, *PWORKING_REGISTERS;

PWORKING_REGISTERS GWorkingIndexRegisters = NULL;

int
CheckFunctionIsMain (
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine checks if the provided context corresponds to the main
    function, and if it does it checks whether the prototype is valid.
    
 Arguments:
 
    Context - Context for the local function.
    
 Return value:
 
    FUNCTION_IS_MAIN_OK if function is main and correct.
    FUNCTION_IS_MAIN_ERR if function is main and erroneous.
    FUNCTION_IS_NOT_MAIN if function is not main.

*/
    
{
    assert(Context != Context->GlobalContext);
    
    PIDENTIFIER_OBJECT Identifier;
    
    Identifier = Context->Identifier;
    if(strcmp(Identifier->Name, "main") == 0) {
        if(Identifier->ReturnType > IDN_TYPE_UINT32T) {
            return FUNCTION_IS_MAIN_ERR;
        }
        
        if(SQueueSize(Identifier->Parameters) != 1) {
            return FUNCTION_IS_MAIN_ERR;
        }
        
        if(((PIDENTIFIER_OBJECT)SQueueTop(Identifier->Parameters))->DataType > IDN_TYPE_UINT32T) {
            return FUNCTION_IS_MAIN_ERR;
        }
        
        return FUNCTION_IS_MAIN_OK;
    }
    
    return FUNCTION_IS_NOT_MAIN;
}

int
CheckIdentifierExists (
    char* Identifier, 
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine checks if the provided identifier exists in the symbol table
    for the provided context.
    
 Arguments:
 
    Identifier - Identifier to check against the symbol table.
 
    Context - Context for the local function.
    
 Return value:
 
    1 if symbol exists. 0 otherwise.

*/
    
{
    PIDENTIFIER_OBJECT Data;
    
    if(SHashMapGet(Context->SymTable, Identifier, (void**)&Data) == SHASHMAP_OK) {
        return 1;
    }
    
    if(SHashMapGet(Context->GlobalContext->SymTable, Identifier, (void**)&Data) == SHASHMAP_OK) {
        return 1;
    }
    
    return 0;
}

int
CheckIdentifierIsArray (
    PIDENTIFIER_OBJECT Identifier
    )
    
/*

 Routine description:
 
    This routine checks if the provided identifier is an array-type variable.
    
 Arguments:
 
    Identifier - Identifier to check.
    
 Return value:
 
    1 if identifier is an array. 0 otherwise.

*/
    
{
    return ((Identifier->Register != REG_RFN) && (Identifier->ArraySize > 0));
}

int
CheckIdentifierIsFunction (
    PIDENTIFIER_OBJECT Identifier
    )
    
/*

 Routine description:
 
    This routine checks if the provided identifier is a function.
    
 Arguments:
 
    Identifier - Identifier to check.
    
 Return value:
 
    1 if identifier is a function. 0 otherwise.

*/
    
{
    return (Identifier->Register == REG_RFN);
}

PIDENTIFIER_OBJECT
GetDeclaredIdentifier (
    char *Name, 
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine retrieves an identifier from the symbol table pertaining to the
    provided context, or NULL if it does not exist.
    
 Arguments:
 
    Identifier - Identifier to check.
    
    Context - Context for the local function or the global context.
    
 Return value:
 
    Pointer to the identifier if it exists in the table. NULL otherwise.

*/
    
{
    PIDENTIFIER_OBJECT Identifier;
    
    if(SHashMapGet(Context->SymTable, Name, (void**)&Identifier) == SHASHMAP_OK) {
        return Identifier;
    }
    
    if(SHashMapGet(Context->GlobalContext->SymTable, Name, (void**)&Identifier) == SHASHMAP_OK) {
        return Identifier;
    }
    
    return NULL;
}

PIDENTIFIER_OBJECT
RegisterIdentifier (
    char* Identifier, 
    IDN_TYPE DataType, 
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine registers an identifier. The resulting identifier is incomplete
    until it is specified as a function, array, local variable, or parameter.*
    
    * This list may increase in the future.
    
 Arguments:
 
    Identifier - Name of the identifier to register.
    
    DataType - Data type for the identifier. 
    
    Context - Context for the local function or the global context.
    
 Return value:
 
    Pointer to the newly registered identifier.

*/
    
{    
    PIDENTIFIER_OBJECT Obj = malloc(sizeof(IDENTIFIER_OBJECT));
    memset(Obj, 0, sizeof(IDENTIFIER_OBJECT));
    Obj->Name = Identifier;
    Obj->Register = REG_INV;
    
    //
    // DataType is a union with ReturnType so we can use the appropriate name later.
    //
    
    Obj->DataType = DataType;
    SHashMapInsert(Context->SymTable, Identifier, (void*)Obj);
    
    fprintf(_NUL,
            "Registered Object %s of type %d: 0x%p\n", 
            Obj->Name, 
            (int)Obj->DataType, 
            (void*)Obj);
    
    return Obj;
}

PIDENTIFIER_OBJECT
RegisterIdentifierAsIntegerConstant (
    signed long Value, 
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine registers an identifier as an integer constant. This identifier
    uses the special REG_RCT pseudo-register.
    
 Arguments:
 
    Value - signed numerical value to assign to the identifier.
    
    Context - Context for the local function or the global context.
    
 Return value:
 
    Pointer to the newly registered identifier.

*/
    
{
    (void)Context;
    PIDENTIFIER_OBJECT NewIdentifier;
    
    NewIdentifier = malloc(sizeof(IDENTIFIER_OBJECT));
    if(NewIdentifier == NULL) {
        return NULL;
    }
    
    memset(NewIdentifier, 0, sizeof(IDENTIFIER_OBJECT));
    NewIdentifier->Name = malloc(strlen("REG_RCT")*sizeof(char)+sizeof(char));
    strcpy(NewIdentifier->Name, "REG_RCT");
    NewIdentifier->Register = REG_RCT;
    NewIdentifier->RelOffset = Value;
    return NewIdentifier;
}

PIDENTIFIER_OBJECT
RegisterIdentifierAsVariable (
    PIDENTIFIER_OBJECT Identifier, 
    int IsAtomic,
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine registers an identifier as a local variable. This identifier
    uses either the REG_RST or REG_RGD register, depending on if Context is
    the global context.
    
 Arguments:
 
    Identifier - Pointer to the identifier to register as a local variable.
    
    IsAtomic - 0 if variable is non-atomic. Any other value otherwise. 
    
    Context - Context for the local function or the global context.
    
 Return value:
 
    Pointer to the modified identifier.

*/
    
{
    if(Context->GlobalContext == Context) {
        Identifier->RelOffset = Context->DataPointer - PROGRAM_DATA_START;
        Context->DataPointer += PROGRAM_STACK_ALIGNMENT;
        Identifier->Register = REG_RGD;
    } else {
        Identifier->RelOffset = Context->DataPointer - PROGRAM_STACK_ALIGNMENT;
        Context->DataPointer += -PROGRAM_STACK_ALIGNMENT;
        Identifier->Register = REG_RST;
    }
    
    Identifier->IsAtomic = IsAtomic;    
    fprintf(_NUL,
            "Identifier %s (0x%p) registered as stack variable offset %ld "
            "atomic: %d assigned to Identifier 0x%p\n",
            Identifier->Name,
            (void*)Identifier,
            Identifier->RelOffset,
            Identifier->IsAtomic,
            (void*)Context);
           
    return Identifier;
}

PIDENTIFIER_OBJECT
RegisterIdentifierAsFunction (
    PIDENTIFIER_OBJECT Identifier, 
    PSCOPE_CONTEXT Context
    )

/*

 Routine description:
 
    This routine registers an identifier as a function. This identifier uses the
    special REG_RGN pseud-register.
    
 Arguments:
 
    Identifier - Pointer to the identifier to register as a function.
    
    Context - Context for the local function or the global context.
    
 Return value:
 
    Pointer to the modified identifier.

*/
    
{
    Identifier->Register = REG_RFN;
    Identifier->AbsOffset = Context->CodePointer;
    fprintf(_NUL,
            "Identifier %s registered as function starting at 0x%p\n",
            Identifier->Name,
            (void*)Context->CodePointer);
    
    return Identifier;
}

PIDENTIFIER_OBJECT
RegisterIdentifierAsParameter (
    char* Identifier, 
    IDN_TYPE DataType, 
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine registers an identifier as a parameter to a function. This 
    identifier uses the REG_RST register.
    
 Arguments:
 
    Identifier - Name of the parameter.
    
    DataType - Data type for the parameter.
    
    Context - Context for the local function.
    
 Return value:
 
    Pointer to the newly registered identifier.

*/
    
{
    assert(Context != Context->GlobalContext);
    
    PIDENTIFIER_OBJECT NewParameter;
    
    NewParameter = malloc(sizeof(IDENTIFIER_OBJECT));
    memset(NewParameter, 0, sizeof(IDENTIFIER_OBJECT));
    NewParameter->Register = REG_RST;
    NewParameter->DataType = DataType;    
    if(Identifier[0] == '\0') {
        NewParameter->Name = malloc(sizeof(char));
        NewParameter->Name[0] = '\0';
    } else {
        NewParameter->Name = Identifier;
    }
    
    //
    // Note the relative offsets for function parameters will be reversed to 
    // match parameter pushing during function calls.
    //
    
    NewParameter->RelOffset = Context->ParameterPointer;
    Context->ParameterPointer = Context->ParameterPointer + PROGRAM_STACK_ALIGNMENT;
    SHashMapInsert(Context->SymTable, Identifier, (void*)NewParameter);
    
    fprintf(_NUL,
            "Identifier %s registered as parameter with stack offset %ld: 0x%p\n",
            NewParameter->Name,
            NewParameter->RelOffset,
            (void*)NewParameter);
    
    return NewParameter;    
}

PIDENTIFIER_OBJECT
RegisterArrayToIdentifier (
    PIDENTIFIER_OBJECT Identifier, 
    unsigned long long ArraySize, 
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine registers an identifier as an array. This routine does not 
    change the identifiers register.
    
 Arguments:
 
    Identifier - Pointer to the identifier to register as an array.
    
    ArraySize - Unsigned value representing the number of Identifier->DataType
                to use for the array size.
    
    Context - Context for the local function or the global context.
    
 Return value:
 
    Pointer to the modified identifier.
    
 Remarks:
 
    An array-type variable may not be atomic. Why? Because.

*/
    
{
    assert(!Identifier->IsAtomic);
    
    Identifier->ArraySize = ArraySize*PROGRAM_STACK_ALIGNMENT;
    if(Context->GlobalContext == Context) {
        Context->DataPointer += ArraySize*PROGRAM_STACK_ALIGNMENT;
    } else {
        Context->DataPointer += ArraySize*-PROGRAM_STACK_ALIGNMENT;
    }
    
    fprintf(_NUL,
            "Added array size %lu: 0x%p\n", 
            (unsigned long)ArraySize, 
            (void*)Identifier);
           
    return Identifier;
}

PIDENTIFIER_OBJECT
RegisterParameterToIdentifier (
    PIDENTIFIER_OBJECT Identifier, 
    PIDENTIFIER_OBJECT Parameter
    )
    
/*

 Routine description:
 
    This routine registers an a parameter identifier to a function identifier.
    
 Arguments:
 
    Identifier - Pointer to the function identifier.
    
    Parameter - Pointer to the parameter identifier.
    
 Return value:
 
    Pointer to the modified function identifier.

*/
    
{
    assert(Identifier->Register == REG_RFN);    
    
    if(Identifier->Parameters == NULL) {
        SQueueInitialize(&Identifier->Parameters);
        if(Identifier->Parameters == NULL) {
            return NULL;
        }
    }
    
    fprintf(_NUL,
            "Adding parameter 0x%p type %d named %s to object:0x%p\n", 
            (void*)Parameter, 
            Parameter->DataType, 
            Parameter->Name, 
            (void*)Identifier);
    
    SQueuePush(Identifier->Parameters, (void*)Parameter);
    return Identifier;
}

POPERATOR_OBJECT
RegisterOperator (
    OPR_TYPE Opr
    )
    
/*

 Routine description:
 
    This routine registers an operator.
    
 Arguments:
 
    Opr - The operator to register.
    
 Return value:
 
    Pointer to the registered operator.

*/
    
{
    POPERATOR_OBJECT Operator;
    
    Operator = malloc(sizeof(OPERATOR_OBJECT));
    if(Operator == NULL) {
        return NULL;
    }
    
    memset(Operator, 0, sizeof(OPERATOR_OBJECT));
    Operator->Type = Opr;
    return Operator;
}

void
DestroyIdentifier (
    PIDENTIFIER_OBJECT Identifier
    )
    
/*

 Routine description:
 
    This routine frees the memory occupied by an Identifier.
    
 Arguments:
 
    Identifier - Pointer to the identifier to free.
    
 Return value:
 
    void.
    
 Remarks:
 
    This routine is very dangerous. It must ONLY be called on Identifiers 
    registered through the Register( ) functions. It must __NOT__ be called
    on index or RTn type registers.

*/    

{
    (void)Identifier;
}

void
DestroyOperator (
    POPERATOR_OBJECT Operator
    )
    
/*

 Routine description:
 
    This routine frees the memory occupied by an Operator.
    
 Arguments:
 
    Operator - Pointer to operator to free.
    
 Return value:
 
    void.

*/
    
{
    (void)Operator;
}

void
ResetWorkingIndexRegister (
    PIDENTIFIER_OBJECT Register
    )
    
/*

 Routine description:
 
    This routine resets an RTn or IXn type register to it's default state.
    
 Arguments:
 
    Register - Pointer to the register to reset.
    
 Return value:
 
    void.

*/
    
{
    assert(IS_REGISTER_WORKING(Register->Register) || 
           IS_REGISTER_INDEX_IX(Register->Register));
    
    Register->RelOffset = 0;
    Register->AbsOffset = 0;
    Register->ArraySize = 0;
    Register->RegisterReferenceCount = 0;
}

PFUNCTIONCALL_OBJECT
RegisterFunctionCall (
    PIDENTIFIER_OBJECT FunctionIdentifier
    )
    
/*

 Routine description:
 
    This routine registers a function call object.
    
 Arguments:
 
    FunctionIdentifier - Pointer to the function identifier for which to
                         generate the function call object.
    
 Return value:
 
    A pointer to the newly registered function call object.

*/
    
{
    assert(FunctionIdentifier->Register == REG_RFN);
    
    PFUNCTIONCALL_OBJECT FunctionCall;
    
    FunctionCall = malloc(sizeof(FUNCTIONCALL_OBJECT));
    if(FunctionCall == NULL) {
        return NULL;
    }
    
    memset(FunctionCall, 0, sizeof(FUNCTIONCALL_OBJECT));
    FunctionCall->FunctionIdentifier = FunctionIdentifier;
    FunctionCall->CurrentParameterCount = 0;
    
    return FunctionCall;
}
    
void
DestroyFunctionCall (
    PFUNCTIONCALL_OBJECT FunctionCall
    )
    
/*

 Routine description:
 
    This routine frees the memory occupied a function call object.
    
 Arguments:
 
    Operator - Pointer to function call object to free.
    
 Return value:
 
    void.

*/
    
{
    (void)FunctionCall;
}

PIO_OBJECT
RegisterIoObject (
    void
    )
    
/*

 Routine description:
 
    This routine registers an IO object.
    
 Arguments:
 
    void
    
 Return value:
 
    A pointer to the newly registered IO object.

*/
    
{
    PIO_OBJECT IoObject;
    
    IoObject = malloc(sizeof(IO_OBJECT));
    if(IoObject == NULL) {
        return NULL;
    }
    
    memset(IoObject, 0, sizeof(IO_OBJECT));
    IoObject->IoIdentifierCount = 0;
    
    return IoObject;
}

PIO_OBJECT
CleanIoObject (
    PIO_OBJECT IoObject
    )
    
/*

 Routine description:
 
    This routine resets an IO object to default state.
    
 Arguments:
 
    IoObject - Pointer to the IO object to clean.
    
 Return value:
 
    A pointer to the cleaned IO object.

*/
    
{
    IoObject->IoIdentifierCount = 0;
    
    return IoObject;
}

int
InitializeRegisters (
    void
    )
    
/*

 Routine description:
 
    This routine initializes the RTn and IXn registers, and must be called at 
    program init.
    
 Arguments:
 
    void
    
 Return value:
 
    0 on success. Any other value on failure.

*/
    
{
    int i;
    unsigned WorkingRegisterSize;
    unsigned IndexRegisterSize;
    
    GWorkingIndexRegisters = malloc(sizeof(WORKING_REGISTERS));
    if(GWorkingIndexRegisters == NULL) {
        return -1;
    }
    
    //
    // Working registers RT0-RT7
    //
    
    WorkingRegisterSize = sizeof(IDENTIFIER_OBJECT) * WORKING_REGISTER_COUNT;    
    GWorkingIndexRegisters->NextAvailable = 0;
    GWorkingIndexRegisters->Registers = malloc(WorkingRegisterSize);
    if(GWorkingIndexRegisters->Registers == NULL) {
        return -1;
    }
    
    memset(GWorkingIndexRegisters->Registers, 0, WorkingRegisterSize);
    for(i=0; i<WORKING_REGISTER_COUNT; ++i) {
        GWorkingIndexRegisters->Registers[i].Name = malloc(strlen("REG_RT0")*sizeof(char)+sizeof(char));
        if(GWorkingIndexRegisters->Registers[i].Name == NULL) {
            return -1;
        }
        
        memcpy(GWorkingIndexRegisters->Registers[i].Name, "REG_RT", strlen("REG_RT")*sizeof(char));
        GWorkingIndexRegisters->Registers[i].Name[6] = '0'+i;
        GWorkingIndexRegisters->Registers[i].Name[7] = '\0';
        GWorkingIndexRegisters->Registers[i].Register = REG_RT0 + i;
    }
    
    //
    // Index registers IX0-IX3
    //
    
    IndexRegisterSize = sizeof(IDENTIFIER_OBJECT) * INDEX_REGISTER_COUNT;    
    GWorkingIndexRegisters->NextAvailableIndex = 0;
    GWorkingIndexRegisters->RegistersIndex = malloc(IndexRegisterSize);
    if(GWorkingIndexRegisters->RegistersIndex == NULL) {
        return -1;
    }
    
    memset(GWorkingIndexRegisters->RegistersIndex, 0, IndexRegisterSize);
    for(i=0; i<INDEX_REGISTER_COUNT; ++i) {
        GWorkingIndexRegisters->RegistersIndex[i].Name = malloc(strlen("REG_IX0")*sizeof(char)+sizeof(char));
        if(GWorkingIndexRegisters->RegistersIndex[i].Name == NULL) {
            return -1;
        }
        
        memcpy(GWorkingIndexRegisters->RegistersIndex[i].Name, "REG_IX", strlen("REG_IX")*sizeof(char));
        GWorkingIndexRegisters->RegistersIndex[i].Name[6] = '0'+i;
        GWorkingIndexRegisters->RegistersIndex[i].Name[7] = '\0';
        GWorkingIndexRegisters->RegistersIndex[i].Register = REG_IX0 + i;
    }
    
    return 0;
}

PIDENTIFIER_OBJECT
ReferenceRegisterWorking (
    PIDENTIFIER_OBJECT Register
    )

/*

 Routine description:
 
    This routine bumps the reference count for an RTn type register.
    
 Arguments:
 
    Register - Register to increment the reference count on.
    
 Return value:
 
    A pointer to the provided register.

*/
    
{
    assert(IS_REGISTER_WORKING(Register->Register));
    
    //
    // We operate under the assumption that this register never has a negative
    // reference count, as well as the fact that it is always UNDER the next
    // available register. 
    //
    
    assert(Register->RegisterReferenceCount >= 0);
    assert((Register->Register - REG_RT0) <= (GWorkingIndexRegisters->NextAvailable - 1));
    
    Register->RegisterReferenceCount = Register->RegisterReferenceCount + 1;
    return Register;
}

void
DereferenceRegisterWorking (
    PIDENTIFIER_OBJECT Register
    )
    
/*

 Routine description:
 
    This routine decreases the reference count for an RTn type register.
    
 Arguments:
 
    Register - Register to decrement the reference count on.
    
 Return value:
 
    void. Why doesn't this function also return the register? Man, I'm baffled
          by some of my design choices.

*/
    
{
    assert(IS_REGISTER_WORKING(Register->Register));
    assert(Register->RegisterReferenceCount > 0);
    
    Register->RegisterReferenceCount = Register->RegisterReferenceCount - 1;
    if(Register->RegisterReferenceCount == 0) {
        
        //
        // By dereferencing down to 0 we are freeing this register for future
        // use. Thus, this better damn well be the last register we used.
        //
        
        assert((Register->Register - REG_RT0) == (GWorkingIndexRegisters->NextAvailable - 1));
        
        ResetWorkingIndexRegister(Register);
        GWorkingIndexRegisters->NextAvailable = GWorkingIndexRegisters->NextAvailable-1;
    }
}

PIDENTIFIER_OBJECT
ReferenceRegisterIndex (
    PIDENTIFIER_OBJECT Register
    )
    
/*

 Routine description:
 
    This routine bumps the reference count for an IXn type register.
    
 Arguments:
 
    Register - Register to increment the reference count on.
    
 Return value:
 
    A pointer to the provided register.

*/
    
{
    assert(IS_REGISTER_INDEX_IX(Register->Register));
    
    //
    // We operate under the assumption that this register never has a negative
    // reference count, as well as the fact that it is always UNDER the next
    // available index register. 
    //
    
    assert(Register->RegisterReferenceCount >= 0);
    assert((Register->Register - REG_IX0) <= (GWorkingIndexRegisters->NextAvailableIndex - 1));
    
    Register->RegisterReferenceCount = Register->RegisterReferenceCount + 1;
    return Register;
}

void
DereferenceRegisterIndex (
    PIDENTIFIER_OBJECT Register
    )
    
/*

 Routine description:
 
    This routine decreases the reference count for an IXn type register.
    
 Arguments:
 
    Register - Register to decrement the reference count on.
    
 Return value:
 
    void. Why doesn't this function also return the register? Man, I'm baffled
          by some of my design choices.

*/
    
{
    assert(IS_REGISTER_INDEX_IX(Register->Register));
    assert(Register->RegisterReferenceCount > 0);
    
    Register->RegisterReferenceCount = Register->RegisterReferenceCount - 1;
    if(Register->RegisterReferenceCount == 0) {
        
        //
        // By dereferencing down to 0 we are freeing this register for future
        // use. Thus, this better damn well be the last index register we used.
        //
        
        assert((Register->Register - REG_IX0) == (GWorkingIndexRegisters->NextAvailableIndex - 1));
        
        ResetWorkingIndexRegister(Register);
        GWorkingIndexRegisters->NextAvailableIndex = GWorkingIndexRegisters->NextAvailableIndex-1;
    }
}

PIDENTIFIER_OBJECT
ReferenceRegister (
    PIDENTIFIER_OBJECT Register
    )
    
/*

 Routine description:
 
    This routine bumps the reference count for an RTn or IXn type register.
    
 Arguments:
 
    Register - Register to bump the reference count on.
    
 Return value:
 
    A pointer to the provided register.          
          
 Remarks:
 
    This function is a no-op for non-RTn or non-IXn type registers.

*/
    
{
    if(IS_REGISTER_INDEX_IX(Register->Register)) {
        return ReferenceRegisterIndex(Register);
    } else if(IS_REGISTER_WORKING(Register->Register)) {
        return ReferenceRegisterWorking(Register);
    } else {
        return Register;
    }
}

void
DereferenceRegister(
    PIDENTIFIER_OBJECT Register
    )
    
/*

 Routine description:
 
    This routine decreases the reference count for an RTn or IXn type register.
    
 Arguments:
 
    Register - Register to decrement the reference count on.
    
 Return value:
 
    void. Why doesn't this function also return the register? Man, I'm baffled
          by some of my design choices.          
          
 Remarks:
 
    This function is a no-op for non-RTn or non-IXn type registers.

*/
    
{
    if(IS_REGISTER_INDEX_IX(Register->Register)) {
        DereferenceRegisterIndex(Register);
    } else if(IS_REGISTER_WORKING(Register->Register)) {
        DereferenceRegisterWorking(Register);
    } else {
        return;
    }
}

PIDENTIFIER_OBJECT
NextAvailableRegister (
    void
    )
    
/*

 Routine description:
 
    This routine obtains the next available RTn type register and increments the
    count for the next available.
    
 Arguments:
 
    void.
    
 Return value:
 
    A pointer to the next available RTn register, or NULL if none is available.         
          
 Remarks:
 
    This function should've been renamed to NextAvailableRegisterWorking.

*/
    
{
    PIDENTIFIER_OBJECT Register;
        
    assert(GWorkingIndexRegisters->NextAvailable < WORKING_REGISTER_COUNT);
    
    if(GWorkingIndexRegisters->NextAvailable >= WORKING_REGISTER_COUNT) {
        return NULL;
    }
    
    Register = &GWorkingIndexRegisters->Registers[GWorkingIndexRegisters->NextAvailable];
    
    assert(Register->RegisterReferenceCount == 0);
    
    Register->RegisterReferenceCount = Register->RegisterReferenceCount + 1;    
    GWorkingIndexRegisters->NextAvailable = GWorkingIndexRegisters->NextAvailable+1;
    
    return Register;
}

PIDENTIFIER_OBJECT
NextAvailableRegisterIndex (
    void
    )
    
/*

 Routine description:
 
    This routine obtains the next available IXn type register and increments the
    count for the next available.
    
 Arguments:
 
    void.
    
 Return value:
 
    A pointer to the next available IXn register, or NULL if none is available.

*/
    
{
    PIDENTIFIER_OBJECT Register;
        
    assert(GWorkingIndexRegisters->NextAvailableIndex < INDEX_REGISTER_COUNT);
    
    if(GWorkingIndexRegisters->NextAvailableIndex >= INDEX_REGISTER_COUNT) {
        return NULL;
    }
    
    Register = &GWorkingIndexRegisters->RegistersIndex[GWorkingIndexRegisters->NextAvailableIndex];
    
    assert(Register->RegisterReferenceCount == 0);
    
    Register->RegisterReferenceCount = Register->RegisterReferenceCount + 1;    
    GWorkingIndexRegisters->NextAvailableIndex = GWorkingIndexRegisters->NextAvailableIndex+1;
    
    return Register;
}

void
FreeAllRegisters (
    void
    )
    
/*

 Routine description:
 
    This routine resets all RTn and IXn type registers and resets the next 
    available count to 0 for both types.
    
 Arguments:
 
    void.
    
 Return value:
    
    void.

*/
    
{
    unsigned i;
    PIDENTIFIER_OBJECT Register;
    
    assert(GWorkingIndexRegisters->NextAvailable <= 1);
    assert(GWorkingIndexRegisters->NextAvailableIndex <= 1);
    
    for(i=0; i < WORKING_REGISTER_COUNT; ++i) {
        Register = &GWorkingIndexRegisters->Registers[i];
        ResetWorkingIndexRegister(Register);
    }
    
    for(i=0; i < INDEX_REGISTER_COUNT; ++i) {
        Register = &GWorkingIndexRegisters->RegistersIndex[i];
        ResetWorkingIndexRegister(Register);
    }
    
    GWorkingIndexRegisters->NextAvailable = 0;
    GWorkingIndexRegisters->NextAvailableIndex = 0;
}

PIDENTIFIER_OBJECT
RegisterSpecialRegister (
    REGISTER Register
    )
    
/*

 Routine description:
 
    This routine registers a special register. This is helpful in situations
    a special register is needed, for example in the case of REG_RIP relative
    jumps, where an identifier for REG_RIP is needed. (no shit sherlock).
    
 Arguments:
 
    Register - The special register to register.
    
 Return value:
 
    A pointer to the registered special register.

*/
    
{
    assert(IS_REGISTER_SPECIAL(Register));
    
    PIDENTIFIER_OBJECT SpecialReg;
    
    SpecialReg = malloc(sizeof(IDENTIFIER_OBJECT));
    if(SpecialReg == NULL) {
        return NULL;
    }
    
    memset(SpecialReg, 0, sizeof(IDENTIFIER_OBJECT));
    SpecialReg->Register = Register;
    
    return SpecialReg;
}

PSCOPE_CONTEXT
CreateScopeContext (
    PIDENTIFIER_OBJECT Identifier, 
    PSCOPE_CONTEXT GlobalContext
    )
    
/*

 Routine description:
 
    This routine creates a scope context.
    
 Arguments:
 
    Identifier - A pointer to the function identifier for this context if the
                 context is local to a function. NULL otherwise.
    
    GlobalContext - A pointer to the global context. NULL if the global context
                    is the one being created.
    
 Return value:
 
    A pointer to the created scope context.

*/
    
{
    PSCOPE_CONTEXT Context;
    
    Context = malloc(sizeof(SCOPE_CONTEXT));
    if(Context == NULL) {
        return NULL;
    }
    
    memset(Context, 0, sizeof(SCOPE_CONTEXT));
    Context->Identifier = Identifier;
    SHashMapInitialize(&Context->SymTable);
    if(Context->SymTable == NULL) {
        free(Context);
        return NULL;
    }
    
    if(GlobalContext == NULL) {
        
        assert(Identifier == NULL);
        
        //
        // This is the global context, so lets adjust it a bit
        //
        
        Context->GlobalContext = Context;
        Context->CodePointer = PROGRAM_CODE_START;
        Context->DataPointer = PROGRAM_DATA_START;
        Context->StackPointer = PROGRAM_CODE_START;
        Context->ParameterPointer = 0;
        Context->Identifier = NULL;
    } else {
        Context->GlobalContext = GlobalContext;
        Context->StackPointer = 0;
        Context->DataPointer = 0;
        Context->CodePointer = GlobalContext->CodePointer;
        Context->ParameterPointer = 8;
        Context->Identifier = Identifier;
    }
    
    fprintf(_NUL,
            "Created context for identifier %s starting at code pointer 0x%p: 0x%p\n", 
            (Identifier ? Identifier->Name : "NULL"), 
            (void*)Context->GlobalContext->CodePointer,
            (void*)Context);
    
    return Context;
}

void
DestroyScopeContext (
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine frees the memory allocated for a scope context.
    
 Arguments:
 
    Context - A pointer to the context to free.
    
 Return value:
 
    void.

*/
    
{
    (void)Context;
}

//
// Symbols
//

PFUNCTION_SYMBOL
RegisterFunctionSymbol (
    PSCOPE_CONTEXT Context
    )
    
/*

 Routine description:
 
    This routine registers a function symbol object.
    
 Arguments:
 
    Context - A pointer to the context for the function whose symbol is being
              created.
    
 Return value:
 
    A pointer to the newly registered function symbol object.

*/
    
{
    assert(Context != Context->GlobalContext);
    
    PSCOPE_CONTEXT GlobalContext;
    PIDENTIFIER_OBJECT FunctionIdentifier;
    PFUNCTION_SYMBOL FunctionSymbol;
    unsigned long ParameterCount;
    
    GlobalContext = Context->GlobalContext;
    FunctionIdentifier = Context->Identifier;
    FunctionSymbol = malloc(sizeof(FUNCTION_SYMBOL));
    if(FunctionSymbol == NULL) {
        return NULL;
    }
    
    memset(FunctionSymbol, 0, sizeof(FUNCTION_SYMBOL));
    FunctionSymbol->FunctionAddress = GlobalContext->CodePointer;
    
    assert(SQueueSize(FunctionIdentifier->Parameters) >= 1);
    
    if(((PIDENTIFIER_OBJECT)SQueueTop(FunctionIdentifier->Parameters))->DataType == IDN_TYPE_VOID) {
        ParameterCount = 0;
    } else {
        ParameterCount = SQueueSize(FunctionIdentifier->Parameters);
    }
    
    FunctionSymbol->ParameterCount = ParameterCount;
    
    fprintf(_NUL,
            "Registered function symbol for function %s Address 0x%p Parameters: %lu\n",
            FunctionIdentifier->Name,
            (void*)(unsigned long long)FunctionSymbol->FunctionAddress,
            FunctionSymbol->ParameterCount);
    
    return FunctionSymbol;
}
