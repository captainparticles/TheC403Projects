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
    
    generator.h
    
 Abstract:
    
    This module implements the routine definitions used to generate code for 
    the various language constructs.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#ifndef __GENERATOR_H__
#define __GENERATOR_H__

#include <stdlib.h>
#include "objtypes.h"
#include "instruction.h"
#include "../../utils/inc/sstack.h"
#include "../../utils/inc/squeue.h"

void
GenerateProgramStartBlockStage0 (
    PSSTACK PendingInstructionStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );

void
GenerateProgramStartBlockStage1 (
    PSSTACK PendingInstructionStack,
    PSQUEUE InstructionQueue,
    unsigned long MainAddress,
    PSCOPE_CONTEXT Context
    );

void
GenerateExpressionInstructions (
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    POPR_TYPE OperatorCheck,
    size_t OperatorCheckSize,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateExpressionInstructionsEmptyStacks (
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateExpressionInstructionsUntilMatch (
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    OPR_TYPE OperatorMatch,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateArrayInstructions (
    PSSTACK OperandStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateIoAddReadIdentifier (
    PIO_OBJECT IoObject,
    PIDENTIFIER_OBJECT Identifier,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateIoAddPrintIdentifier (
    PIO_OBJECT IoObject,
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateIoFinishRead (
    PIO_OBJECT IoObject,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateIoFinishPrint (
    PIO_OBJECT IoObject,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateFunctionHeaderStage0 (
    PSSTACK PendingInstructionStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateFunctionHeaderStage1 (
    PSSTACK PendingInstructionStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateFunctionTrailer (
    PSSTACK PendingInstructionStack,
    PSSTACK InstructionCountStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateFunctionCallVoidParameter (
    PFUNCTIONCALL_OBJECT FunctionCall,
    PSSTACK OperatorStack,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateFunctionCallPushParameter (
    PFUNCTIONCALL_OBJECT FunctionCall,
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateFunctionCallMakeCall (
    PFUNCTIONCALL_OBJECT FunctionCall,
    PSSTACK OperandStack,
    PSQUEUE InstructionQueue,
    PINSTRUCTION *LastCallInstruction,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateFunctionReturn (
    PSSTACK OperandStack,
    PSSTACK OperatorStack,
    PSSTACK PendingInstructionStack,
    PSSTACK InstructionCountStack,
    PSQUEUE InstructionQueue,
    PSCOPE_CONTEXT Context
    );
    
void
GenerateFunctionCallPatchParallelSync (
    PINSTRUCTION InstructionCall
    );
    
void
GenerateFunctionCallPatchParallelAsync (
    PINSTRUCTION InstructionCall
    );

#endif // __GENERATOR_H__
