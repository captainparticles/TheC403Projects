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
    
    main.c
    
 Abstract:
   
    Entry point.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/24/15        Initial Creation

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "../Common/def.h"
#include "../../utils/inc/shashmap.h"
#include "../../utils/inc/sstack.h"
#include "../../utils/inc/squeue.h"
#include "program.h"
#include "exec.h"
#include "error.h"


#ifndef COMPILE_VERBOSE
FILE* _NUL;
#endif

PPROGRAM GProgram = NULL;

INT 
main (
    INT argc, 
    PCHAR *argv
    )
{
    FILE *FileCompiled;
    
#ifndef COMPILE_VERBOSE
    _NUL = fopen("nul", "w");
    if(_NUL == NULL) {
        VmFatal(ERR_STR_NULOPENFAIL);
    }
#endif
    
    FileCompiled = fopen("out.cut", "rb");
    if(FileCompiled == NULL) {
        VmFatal(ERR_STR_NOINPUTFILE);
    }
    
    ProgramRead(FileCompiled, &GProgram);
    
    ExecPrimeProgram( );
    
#ifndef COMPILE_VERBOSE
    fclose(_NUL);
#endif
    return 0;
}
