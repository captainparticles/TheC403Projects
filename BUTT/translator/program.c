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
    
    program.c
    
 Abstract:
    
    This module implements the routines used to serialize the code.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation
    11/25/15        Documented functions

**/

#include "program.h"
#include "instruction.h"
#include "../Common/symdef.h"
#include "../Common/progdef.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void
ProgramOpenInputOutputFiles (
    char *SourceInName,
    char *CompileOutName,
    FILE **SourceIn,
    FILE **CompileOut
    )
    
/*

 Routine description:
 
    This routine is in charge of opening the text source file and the binary 
    output file. 
    
 Arguments:
 
    SourceInName - The name of the text source file.
    
    SourceOutName - The name of the output compiled file.
    
    SourceIn - A pointer that receives the opened source file.
    
    CompileOut - A pointer that receives the opened compilation target file.
    
 Return value:
 
    void.

*/
    
{
    FILE *In;
    FILE *Out;
    
    In = fopen(SourceInName, "r");
    Out = fopen(CompileOutName, "wb");
    *SourceIn = In;
    *CompileOut = Out;
}

void
ProgramSerializeQueue (
    void *WriteBuffer,
    unsigned long WriteBufferSize,
    FILE *File,
    PSQUEUE Queue,
    unsigned long DataSize
    )
    
/*

 Routine description:
 
    This routine is in charge of serializing a PSQUEUE given the size of its 
    elements.
    
 Arguments:
 
    WriteBuffer - The buffer we accumulate to before serializing.
    
    WriteBufferSize - The size in bytes of the WriteBuffer.
    
    File - Pointer the write-binary-opened file to serialize to.
    
    Queue - The queue to serialize.
    
    DataSize - The size in bytes of each element in the queue.
    
 Return value:
 
    void.
     
 Remarks:
 
    I mean... we could create the write buffer in this function. The only reason
    it's received is because we had already created it in the caller. Then again
    given this function is mostly caller-independent, it would seem more correct
    to create the buffer here. Maybe someday.

*/
    
{
    void *CurrentNode;
    void *NodeData;
    size_t WriteBufferLength;
    size_t WriteBufferCount;
    size_t CountsWritten;
    
    assert((WriteBufferSize % DataSize) == 0);
    
    WriteBufferLength = 0;
    CurrentNode = SQueueTopNode(Queue);
    while(CurrentNode != NULL) {
        NodeData = SQueueDataFromNode(CurrentNode);
        
        assert(NodeData != NULL);
        assert(WriteBufferLength <= WriteBufferSize);
        
        if(WriteBufferLength == WriteBufferSize) {
            CountsWritten = fwrite(WriteBuffer,
                                   DataSize,
                                   WriteBufferCount,
                                   File);
                                  
            assert(CountsWritten == WriteBufferCount);
            
            WriteBufferLength = 0;
            WriteBufferCount = 0;
        }
        
        memcpy(WriteBuffer+WriteBufferLength,
               NodeData,
               DataSize);
               
        WriteBufferLength += DataSize;
        WriteBufferCount += 1;
        CurrentNode = SQueueNextFromNode(CurrentNode);
    }
    
    if(WriteBufferLength > 0) {
        CountsWritten = fwrite(WriteBuffer,
                               DataSize,
                               WriteBufferCount,
                               File);
            
        assert(CountsWritten == WriteBufferCount);
        
        WriteBufferLength = 0;
        WriteBufferCount = 0;
    }
}

void
ProgramSerializeCode (
    FILE  *OutFile,
    PSQUEUE InstructionQueue,
    PSQUEUE FunctionSymbolQueue,
    PSCOPE_CONTEXT GlobalContext
    )
    
/*

 Routine description:
 
    This routine is in charge of creating the output binary. It generates the 
    program header and serializes it as well as the function symbols and the 
    code itself.
    
 Arguments:
 
    OutFile - Pointer the write-binary-opened file to serialize to.
    
    InstructionQueue - Pointer to the global instruction queue for the program.
    
    FunctionSymbolQueue - Pointer to the function symbol queue.
    
    GlobalContext - Pointer to the programs global context.
    
 Return value:
 
    void.

*/
    
{
    assert(GlobalContext->GlobalContext == GlobalContext);
    
    unsigned char WriteBuffer[4096];
    PFUNCTION_SYMBOL FunctionSymbolWriteBuffer[4096/sizeof(FUNCTION_SYMBOL)];
    PINSTRUCTION InstructionWriteBuffer[4096/sizeof(INSTRUCTION)];
    size_t WriteBufferLength;
    size_t BytesWritten; 
    PROGRAM_HEADER ProgramHeader;

    assert((sizeof(WriteBuffer) % sizeof(INSTRUCTION)) == 0);
    
    //
    // Header
    //
    
    memset(&ProgramHeader, 0, sizeof(PROGRAM_HEADER));
    ProgramHeader.MagicNumber = HEADER_MAGIC_NUMBER;
    ProgramHeader.VersionMajor = COMPILER_VERSION_MAJOR;
    ProgramHeader.VersionMinor = COMPILER_VERSION_MINOR;
    ProgramHeader.StackAlignment = PROGRAM_STACK_ALIGNMENT;
    ProgramHeader.StackTop = PROGRAM_STACK_TOP;
    ProgramHeader.DataStart = PROGRAM_DATA_START;
    ProgramHeader.CodeStart = PROGRAM_CODE_START;
    ProgramHeader.StackSize = PROGRAM_STACK_TOP;
    ProgramHeader.DataSize = (GlobalContext->DataPointer - PROGRAM_DATA_START) * PROGRAM_STACK_ALIGNMENT;
    ProgramHeader.CodeSize = SQueueSize(InstructionQueue) * sizeof(INSTRUCTION);
    ProgramHeader.SymbolSize = SQueueSize(FunctionSymbolQueue) * sizeof(FUNCTION_SYMBOL);
    ProgramHeader.SymbolBinaryLocation = HEADER_SIZE_BYTES;
    ProgramHeader.CodeBinaryLocation = ProgramHeader.SymbolBinaryLocation + 
                                       ProgramHeader.SymbolSize + sizeof(FUNCTION_SYMBOL);
    
    memset(WriteBuffer, 0, sizeof(WriteBuffer));
    memcpy(WriteBuffer, &ProgramHeader, sizeof(PROGRAM_HEADER));
    WriteBufferLength = HEADER_SIZE_BYTES;
    BytesWritten = fwrite(WriteBuffer, sizeof(char), WriteBufferLength, OutFile);
    
    assert(BytesWritten = WriteBufferLength);
    
    //
    // Function symbols
    //
    
    ProgramSerializeQueue(FunctionSymbolWriteBuffer, 
                          sizeof(FunctionSymbolWriteBuffer),
                          OutFile,
                          FunctionSymbolQueue,
                          sizeof(FUNCTION_SYMBOL));
    
    ProgramSerializeQueue(InstructionWriteBuffer,
                          sizeof(InstructionWriteBuffer),
                          OutFile,
                          InstructionQueue,
                          sizeof(INSTRUCTION));
    
    fclose(OutFile);
    
    DebugPrettyPrintProgramHeader(&ProgramHeader); 
    DebugPrintProgram(InstructionQueue);
}
