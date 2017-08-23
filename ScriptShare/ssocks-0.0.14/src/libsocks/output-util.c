/*
 *      output-util.c
 *      
 *      Created on: 2011-04-03
 *      Author:     Hugo Caron
 *      Email:      <h.caron@codsec.com>
 * 
 * Copyright (C) 2011 by Hugo Caron
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "output-util.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h> /* isspace */


int verbosity = 0;

void ERROR(int level, char *template, ...){
	va_list ap;
	va_start(ap, template);
	if ( level <= verbosity ){
		vfprintf(stderr, template, ap);
		printf("\n");
	}
	va_end(ap);	
}

void TRACE(int level, char *template, ...){
	va_list ap;
	va_start(ap, template);
	if ( level <= verbosity ){
		vfprintf(stdout, template, ap);
		printf("\n");
	}
	va_end(ap);
}

void DUMP(const char *s, size_t n){
	unsigned int i;

	for (i=0; i < n; i++)
		printf ("0x%x ", s[i]);
		
	printf ("\n");
}

void trim(char * s) {
    char *p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(*p && isspace(*p)) ++p, --l;

    memmove(s, p, l + 1);
}
