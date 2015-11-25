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
    
    objtypes.h
    
 Abstract:
    
    This module defines the structures used throughout the translator.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#ifndef __OBJTYPES_H__
#define __OBJTYPES_H__

#include <inttypes.h>
#include "../Common/registerdef.h"
#include "../../utils/inc/squeue.h"
#include "../../utils/inc/sstack.h"
#include "../../utils/inc/shashmap.h"

typedef enum _IDN_TYPE {
    
    //
    // This order matters! THREADT, VOID, and ERR must come AFTER all the 
    // numerical data types.
    //
    // Modifications to this structure require changes in opcodes.c and
    // instruction.c, as well as potentially others.
    //
    
    IDN_TYPE_INT8T          = 0,
    IDN_TYPE_UINT8T         = 1,
    
    IDN_TYPE_INT16T         = 2,
    IDN_TYPE_UINT16T        = 3,
    
    IDN_TYPE_INT32T         = 4,
    IDN_TYPE_UINT32T        = 5,
    
    IDN_TYPE_FLOATT         = 6,
    
    IDN_TYPE_THREADT        = 7,
    
    IDN_TYPE_VOID           = 8, // Only for functions!
    
    IDN_TYPE_ERR            = 9
} IDN_TYPE;

typedef enum OPR_TYPE {
    OPR_TYPE_STR            = 0,
    
    OPR_TYPE_LOR            = 1,
    OPR_TYPE_LAND           = 2,
    
    OPR_TYPE_XOR            = 3,
    OPR_TYPE_OR             = 4,
    OPR_TYPE_AND            = 5,
    OPR_TYPE_NOT            = 6,
    
    OPR_TYPE_EQ             = 7,
    OPR_TYPE_NEQ            = 8,
    OPR_TYPE_LT             = 9,
    OPR_TYPE_GT             = 10,
    OPR_TYPE_LTE            = 11,
    OPR_TYPE_GTE            = 12,
    
    OPR_TYPE_PLUS           = 13,
    OPR_TYPE_MINUS          = 14,
    OPR_TYPE_TIMES          = 15,
    OPR_TYPE_DIV            = 16,
    
    OPR_TYPE_LPAREN         = 17,
    OPR_TYPE_RPAREN         = 18,
    
    OPR_TYPE_LBRACK         = 19,
    OPR_TYPE_RBRACK         = 20,

    OPR_TYPE_ERR            = 21
} OPR_TYPE, *POPR_TYPE;

typedef struct _IDENTIFIER_OBJECT {
    char* Name;
    REGISTER Register;
    unsigned RegisterReferenceCount;        // RT/IX Registers
    union {
        signed long RelOffset;
        unsigned long AbsOffset;
    };
    
    union {
        IDN_TYPE DataType;                  // variables
        IDN_TYPE ReturnType;                // functions
    };
    
    unsigned IsAtomic;                      // Arrays may not be atomic.
    unsigned long ReturnCount;
    union {
        unsigned long ArraySize;
        PSQUEUE Parameters;
    };
} IDENTIFIER_OBJECT, *PIDENTIFIER_OBJECT;

typedef struct _FUNCTIONCALL_OBJECT {
    PIDENTIFIER_OBJECT FunctionIdentifier;
    unsigned CurrentParameterCount;
} FUNCTIONCALL_OBJECT, *PFUNCTIONCALL_OBJECT;

typedef struct _IO_OBJECT {
    unsigned IoIdentifierCount;
} IO_OBJECT, *PIO_OBJECT;

typedef struct _OPERATOR_OBJECT {
    OPR_TYPE Type; 
} OPERATOR_OBJECT, *POPERATOR_OBJECT;

typedef struct _SCOPE_CONTEXT {
    struct _SCOPE_CONTEXT* GlobalContext;
    PIDENTIFIER_OBJECT Identifier;
    PSHASHMAP SymTable;
    unsigned long long CodePointer;
    unsigned long long ParameterPointer;
    unsigned long long DataPointer; 
    unsigned long long StackPointer; // Only for the global context.
} SCOPE_CONTEXT, *PSCOPE_CONTEXT;

#endif // __OBJTYPES_H__
