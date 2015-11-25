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
   
    This module implements the program read/de-serialization routines.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/24/15        Initial Creation

**/

#include "program.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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

LONG
ProgramRead (
    FILE *ProgramFile,
    PPROGRAM *ProgramOut
    )
{
    LONG RetVal;
    PPROGRAM Program;
    UCHAR HeaderBuffer[sizeof(PROGRAM_HEADER)];
    PFUNCTION_SYMBOL FunctionSymbolBuffer;
    ULONG FunctionSymbolBufferSize;
    ULONG FunctionSymbolBufferCount;
    PCHAR ProgramCode;
    ULONG ProgramCodeSize;
    ULONG ProgramCodeCount;
    PCHAR ProgramData;
    ULONG BytesRead;
    
    *ProgramOut = NULL;
    Program = malloc(sizeof(PROGRAM));
    if(Program == NULL) {
        goto ProgramReadErr;
    }
    
    memset(Program, 0, sizeof(PROGRAM));    
    BytesRead = fread(HeaderBuffer, 
                      sizeof(UCHAR), 
                      sizeof(PROGRAM_HEADER), 
                      ProgramFile);
                      
    assert(BytesRead == sizeof(PROGRAM_HEADER));
    
    memcpy(&Program->Header, HeaderBuffer, sizeof(PROGRAM_HEADER));
    
    DebugPrettyPrintProgramHeader(&Program->Header);
    
    //
    // It would be absolutely lovely if we could use a hashmap for the 
    // function symbols. However the SHASHMAP class takes a char* as the key
    // and we don't have that and I currently lack the time to modify it to 
    // take a hashing function as an initialization parameter to have it be
    // able to hash any data type. 
    //
    // With that in mind, we have an array of the symbols and do a linear
    // search whenever we need one.
    //
    
    FunctionSymbolBufferSize = Program->Header.SymbolSize;
    FunctionSymbolBufferCount = FunctionSymbolBufferSize / sizeof(FUNCTION_SYMBOL);
    FunctionSymbolBuffer = malloc(FunctionSymbolBufferSize);
    if(FunctionSymbolBuffer == NULL) {
        goto ProgramReadErr;
    }
    
    memset(FunctionSymbolBuffer, 0, FunctionSymbolBufferSize);
    BytesRead = fread(FunctionSymbolBuffer, 
                      sizeof(FUNCTION_SYMBOL), 
                      FunctionSymbolBufferCount, 
                      ProgramFile);
                      
    assert(BytesRead == FunctionSymbolBufferCount);
    
    //
    // Ok, finally read the instructions.
    //
    
    //
    // BUGBUG: Shouldn't need the next line. Like, at all.
    //
    
    fseek(ProgramFile, Program->Header.CodeBinaryLocation, SEEK_SET);
    ProgramCodeSize = Program->Header.CodeSize;
    ProgramCodeCount = ProgramCodeSize / sizeof(INSTRUCTION);
    ProgramCode = malloc(ProgramCodeSize);
    if(ProgramCode == NULL) {
        goto ProgramReadErr;
    }
    
    BytesRead = fread(ProgramCode, 
                      sizeof(INSTRUCTION),
                      ProgramCodeCount,
                      ProgramFile);
    
    ProgramData = malloc(Program->Header.DataSize);
    if(ProgramData == NULL) {
        goto ProgramReadErr;
    }
    
    memset(ProgramData, 0, Program->Header.DataSize);
    Program->FunctionSymbols = FunctionSymbolBuffer;
    Program->FunctionSymbolsSize = FunctionSymbolBufferCount;
    Program->GlobalData = ProgramData;
    Program->Code = ProgramCode;
    *ProgramOut = Program;    
    
    RetVal = 0;
    goto ProgramReadEnd;
    
ProgramReadErr:
    if(Program != NULL) {
        free(Program);
    }
    
    if(ProgramCode != NULL) {
        free(ProgramCode);
    }
    
    RetVal = -1;
    
ProgramReadEnd:
    if(FunctionSymbolBuffer != NULL) {
        free(FunctionSymbolBuffer);
    }
    
    return RetVal;
}
