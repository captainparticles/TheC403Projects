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
    
    registerdef.c
    
 Abstract:
    
    This module implements the string name array for the registers.
    
 Author:
    
    Omar Carey      Carey403@gmail.com      11/17/15

 Revision:
 
    11/17/15        Initial Creation

**/

#include "registerdef.h"

const char * const _REGISTER_NAMES[] = {
    "RIP",
    "RGD",
    "RST",
    "RSB",
    "RRV",
    "RCT",
    
    "IX0",
    "IX1",
    "IX2",
    "IX3",
    
    "RT0",
    "RT1",
    "RT2",
    "RT3",
    "RT4",
    "RT5",
    "RT6",
    "RT7",
    
    "RFN",
    "INV"
};
