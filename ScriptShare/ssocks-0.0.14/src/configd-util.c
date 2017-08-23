/*
 *      configd-util.c
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

#include "configd-util.h"
#include <libsocks/output-util.h>

#include <stdio.h>
#include <strings.h>       
#include <string.h>
#include <stdlib.h>

/* Daemon function */
#include <unistd.h>


int loadConfigFile(char *filename, struct globalArgsServer_t *c){
	int k;
	char var[255];
	char val[255];
	char buf[510];

	TRACE(L_DEBUG, "config: open file %s ...", filename);


	FILE *fp = fopen(filename, "r");
	if ( fp == 0 ){
		ERROR(L_NOTICE, "config: can't open file %s", filename);
		//perror("fopen");
		return -1;
	}
	
	while(!feof(fp)){
		if(fgets(buf, sizeof(buf), fp) == NULL)
			break;

		k = sscanf(buf, "%254[^#=]=%254[^\n]\n", var, val);

		//printf("%d\n", k);
		if ( k != 2 ){
			//TRACE(L_VERBOSE, "config: file malformated");
			//break;
			continue;
		}
		trim(var); trim(val);
		if(strcasecmp(var, "PORT") == 0 ){
			c->port = atoi(val);
		}else if(strcasecmp(var, "AUTH") == 0 ){
			strncpy_s(c->fileauth, val, 
				sizeof(globalArgsServer.fileauth));
		}else if(strcasecmp(var, "LOG") == 0 ){
			strncpy_s(c->filelog, val, 
				sizeof(globalArgsServer.filelog));
		}else if(strcasecmp(var, "DAEMON") == 0 ){
			c->daemon = atoi(val);
		}else if(strcasecmp(var, "BIND") == 0 ){
			strncpy_s(c->bindAddr, val, 
				sizeof(globalArgsServer.bindAddr));
		}else if(strcasecmp(var, "VERBOSITY") == 0 ){
			c->verbosity = atoi(val);
		}else if(strcasecmp(var, "GUEST") == 0 ){
			c->guest = atoi(val);
#ifdef HAVE_LIBSSL
		}else if(strcasecmp(var, "CERT") == 0 ){
			strncpy_s(c->filecert, val, 
				sizeof(globalArgsServer.filecert));
		}else if(strcasecmp(var, "KEY") == 0 ){
			strncpy_s(c->filekey, val, 
				sizeof(globalArgsServer.filekey));
		}else if(strcasecmp(var, "SSL") == 0 ){
			c->ssl = atoi(val);
#endif /* HAVE_LIBSSL */
		}else{
			ERROR(L_NOTICE, "config: unknown option %s", var);
			return -1;
		}
		
		TRACE(L_DEBUG, "config: option %s=%s", var, val);		
	}
	
	TRACE(L_DEBUG, "config: close file");
	fclose(fp);

	return 0;
}

int removePID(char *filename){
	if ( unlink(filename) != 0 ){
		/* Don't show this is useless actually */
		/* perror("unlink");*/
		return -1;
	}
	return 0;
}

int writePID(char *filename){
	FILE *fp = fopen(filename, "w");
	if ( fp == 0 ){
		perror("fopen");
		return -1;
	}
	fprintf(fp, "%ld\n", (long)getpid());
	fclose(fp);
	
	/* atexit((void(*)()) removePID); */
	return 0;
}



/* Same function in unistd.h daemon */
void background(){
	pid_t pid, sid;

	
	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	
	/* If we got a good PID, then
	we can exit the parent process. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	
	/* Open any logs here */
	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		/* Log any failure */
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
		/* Log any failure here */
		exit(EXIT_FAILURE);
	}
	
	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

/* Fast hack for secure strcpy, to lazy to check if is trully secure */
char *strncpy_s(char *dest, const char *src, size_t n){
	dest[n] = 0;
	char *s = strncpy(dest, src, n-1);
	return s;
}
