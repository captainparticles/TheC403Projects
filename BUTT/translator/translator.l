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
    
    translator.l
    
 Abstract:
    
    This module implements the lexer language for BUTT.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

%{
#include <stdio.h>
#include <string.h>

#include "../../utils/inc/squeue.h"
#include "objtypes.h"
#include "translator.tab.h"

#ifdef __cplusplus
#define YY_DECL extern "C" int yylex()
#else
#define YY_DECL int yylex()
#endif

%}

%option nounput
%option noinput
%option noyywrap
%option yylineno

%%

\n                      { }
[ \t]+ 

\/\/.*

for                     { return TKFOR; }
while                   { return TKWHILE; }

if                      { return TKIF; }
else                    { return TKELSE; }

thread                  { return TKTHREAD; }
as                      { return TKAS; }
sync                    { return TKSYNC; }
async                   { return TKASYNC; }
atomic                  { return TKATOMIC; }

void                    { return TKVOID; }
int8                    { return TKINT8; }
uint8                   { return TKUINT8; }    
int16                   { return TKINT16; }    
uint16                  { return TKUINT16; }
int32                   { return TKINT32; }    
uint32                  { return TKUINT32; }    
float                   { return TKFLOAT; }

return                  { return TKRETURN; }

print                   { return TKPRINT; }
read                    { return TKREAD; }

\|\|                    { return TKLOR; }
&&                      { return TKLAND; }
==                      { return TKEQ; }
!=                      { return TKNEQ; }
\<=                     { return TKLEQ; }
\>=                     { return TKGEQ; }

[0-9]+\.[0-9]*          { yylval.Float = atof(yytext); return TFLOAT; }
[0-9]+                  { yylval.Int = atoi(yytext); return TINT; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.String = strdup(yytext); return TIDENTIFIER; }
.                       { return yytext[0]; }

%%
