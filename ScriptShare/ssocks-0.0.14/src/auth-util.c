/*
 *      auth-util.c
 *      
 *      Created on: 2011-04-02
 *      Author:     Hugo Caron
 *      Email:      <h.caron@codsec.com>
 * 
 * 
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

#define MAX_AUTH_LOGIN 128
#define MAX_AUTH_UNAME 255

#include <libsocks/output-util.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	char uname[MAX_AUTH_UNAME];
	char passwd[MAX_AUTH_UNAME];
}Cred;

Cred tcred[MAX_AUTH_LOGIN];
int ncred = 0;

int load_auth_file(char *filename){
	int k;
	char *line = NULL;
	char buf[(MAX_AUTH_UNAME*2)+1];

	TRACE(L_DEBUG, "auth: open file %s ...", filename);
	FILE *fp = fopen(filename, "r");
	
	if ( fp == 0 ){
		//perror("fopen");
		ERROR(L_NOTICE, "auth: can't open file %s", filename);
		return -1;
	}
	
	while(ncred < MAX_AUTH_LOGIN && !feof(fp) ){
		//printf("line : %s\n", fp);
		if(fgets(buf, sizeof(buf), fp) == NULL)
			break;

		/* Warn: Potential overflow */
		k = sscanf(buf, "%254[^#:]:%254s", tcred[ncred].uname, tcred[ncred].passwd);

		if ( k != 2 ){
			/* Error or comment or blank line */
			/*TRACE(L_VERBOSE, "auth: file config format error");*/
			continue;
		}
		/* Avoid whitespace maybe fscanf can do this */
		trim(tcred[ncred].uname); trim(tcred[ncred].passwd);
		TRACE(L_DEBUG, "auth: load user %s:%s",
				tcred[ncred].uname, tcred[ncred].passwd);

		ncred++;
	}
	
	TRACE(L_DEBUG, "auth: close file");
	fclose(fp);
	free(line);
	return ncred;
}

int check_auth(char *uname, char *passwd){
	int i;
	for (i=0; i < ncred; ++i)
		if ( strcmp(uname, tcred[i].uname) == 0 
				&& strcmp(passwd, tcred[i].passwd) == 0)
			return 1;

	return -1;
}
