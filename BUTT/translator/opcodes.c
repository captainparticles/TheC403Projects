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
    
    This module implements the routines used to generate resulting opcodes from
    expressions during code generation.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#include "opcodes.h"
#include "objtypes.h"
#include <assert.h>

OPCODES
OpcodeFromIdn (
    IDN_TYPE T
    )
{
    switch(T) {
    case IDN_TYPE_INT8T:
        return OPC_STRI8;
    case IDN_TYPE_UINT8T:
        return OPC_STRU8;
    case IDN_TYPE_INT16T:
        return OPC_STRI16;
    case IDN_TYPE_UINT16T:
        return OPC_STRU16;
    case IDN_TYPE_INT32T:
        return OPC_STRI32;
    case IDN_TYPE_UINT32T:
        return OPC_STRU32;
    case IDN_TYPE_FLOATT:
        return OPC_STRF;
    default:
        return OPC_ERR;
    }
}

OPCODES
OpcodeFromOperator (
    OPR_TYPE O, 
    IDN_TYPE L, 
    IDN_TYPE R
    )
{
    OPCODES OpcodeAdd;
    OPCODES OpcodeSub;
    OPCODES OpcodeMul;
    OPCODES OpcodeDiv;
    
    if(L == IDN_TYPE_FLOATT || R == IDN_TYPE_FLOATT) {
        OpcodeAdd = OPC_ADDF;
        OpcodeSub = OPC_SUBF;
        OpcodeMul = OPC_MULF;
        OpcodeDiv = OPC_DIVF;
    } else {
        OpcodeAdd = OPC_ADDI;
        OpcodeSub = OPC_SUBI;
        OpcodeMul = OPC_MULI;
        OpcodeDiv = OPC_DIVI;
    }
    
    switch(O) {
    case OPR_TYPE_LAND:
        return OPC_LAND;
    case OPR_TYPE_LOR:
        return OPC_LOR;
    case OPR_TYPE_XOR:
        return OPC_XOR;
    case OPR_TYPE_OR:
        return OPC_OR;
    case OPR_TYPE_AND:
        return OPC_AND;
    case OPR_TYPE_NOT:
        return OPC_NOT;
    case OPR_TYPE_EQ:
        return OPC_EQ;
    case OPR_TYPE_NEQ:
        return OPC_NEQ;
    case OPR_TYPE_LT:
        return OPC_LT;
    case OPR_TYPE_GT:
        return OPC_GT;
    case OPR_TYPE_LTE:
        return OPC_LTE;
    case OPR_TYPE_GTE:
        return OPC_GTE;
    case OPR_TYPE_PLUS:
        return OpcodeAdd;
    case OPR_TYPE_MINUS:
        return OpcodeSub;
    case OPR_TYPE_TIMES:
        return OpcodeMul;
    case OPR_TYPE_DIV:
        return OpcodeDiv;
    default:
        return OPC_ERR;
    }
}

int
IsOperandSigned (
    IDN_TYPE T
    )
{
    //
    // The way IDN_TYPE is set up right now we could check if T is even or odd,
    // however I consider this extremely ugly but more important too fragile and
    // not future proof. So we check against all signed types.
    //
    
    return (T == IDN_TYPE_INT8T || T == IDN_TYPE_INT16T || T == IDN_TYPE_INT32T);
}

OPCODES
GenerateOpcode (
    IDN_TYPE L, 
    IDN_TYPE R, 
    OPR_TYPE O
    )
{
    if(L > IDN_TYPE_THREADT || R > IDN_TYPE_THREADT) {
        return OPC_ERR;
    }
    
    switch(O) {
    case OPR_TYPE_STR:
        if(L == IDN_TYPE_THREADT) {
            if (R != IDN_TYPE_THREADT) {
                return OPC_ERR;
            } else {
                return OPC_STRTH;
            }
        }
        
        //
        // Both are of numerical (decimal or float) type here, so we return the
        // lvalue. This is because we truncate or sign-extend on data type size
        // mismatch. We are admittedly quite liberal with this language.
        //
        
        return OpcodeFromIdn(L);
    
    case OPR_TYPE_LAND:
    case OPR_TYPE_LOR:
    case OPR_TYPE_XOR:
    case OPR_TYPE_OR:
    case OPR_TYPE_AND:
    case OPR_TYPE_NOT:
    case OPR_TYPE_EQ:
    case OPR_TYPE_NEQ:
    case OPR_TYPE_LT:
    case OPR_TYPE_GT:
    case OPR_TYPE_LTE:
    case OPR_TYPE_GTE:
    case OPR_TYPE_PLUS:
    case OPR_TYPE_MINUS:
    case OPR_TYPE_TIMES:
    case OPR_TYPE_DIV:
        if(L == IDN_TYPE_THREADT || R == IDN_TYPE_THREADT) {
            return OPC_ERR;
        }
        
        //
        // All these operators can function with any numerical data type. Of course
        // by "can" we don't mean correctly, just that something will happen.
        // Your fault if you xor a float with a char, for example.
        //
        
        return OpcodeFromOperator(O, L, R);
        
    default:
        return OPC_ERR;
    }
    
    return OPC_ERR;
}

IDN_TYPE
GenerateResultingDataType (
    IDN_TYPE L, 
    IDN_TYPE R, 
    OPR_TYPE O
    )
{
    if(L >= IDN_TYPE_THREADT || R >= IDN_TYPE_THREADT) {
        return IDN_TYPE_ERR;
    }
    
    if(L == IDN_TYPE_FLOATT || R == IDN_TYPE_FLOATT) {
        return IDN_TYPE_FLOATT;
    }
    
    if(O == OPR_TYPE_STR) {
        
        //
        // If this is a store then we will end up pushing a new register or
        // stack value into the operand stack so we can chain assignments. This
        // new type is truncated or extended to the lvalue, so we simply return
        // L here.
        //
        
        return L;
    }
    
    assert(O != OPR_TYPE_STR);
    
    //
    // This is not an assignment, and we work all intermediate operations with
    // the largest data type. Since operations between signed and unsigned types
    // are undefined behavior, we treat them as a signed result. Thus, if we have
    // a single signed type we return IDN_TYPE_INT32T, otherwise IDN_TYPE_UINT32T
    //
    
    if(IsOperandSigned(L) || IsOperandSigned(R)) {
        return IDN_TYPE_INT32T;
    }
    
    return IDN_TYPE_UINT32T;
}
