/*
 *      configd-util.h
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
#ifndef CONFIGD_UTIL__H
#define CONFIGD_UTIL__H

#include <config.h>
#include <string.h>



struct globalArgsServer_t {
	char fileauth[255];				// -a option
	char fileconfig[255];			// -f option
	char filelog[255];			// -l option
	char bindAddr[16];                  // -b option
	unsigned int port;			// -p option
	unsigned int verbosity;		// -v option
	unsigned int daemon;		// -d option
	unsigned int ssl;			// -s option
#ifdef HAVE_LIBSSL
	char filecert[255];			// -c option
	char filekey[255];			// -k option
#endif
	unsigned int guest;
	char **inputFiles;			// input files
	int numInputFiles;			// # of input files
} globalArgsServer;
 

int loadConfigFile(char *filename, struct globalArgsServer_t *c);
int writePID(char *filename);
int removePID(char *filename);
char *strncpy_s(char *dest, const char *src, size_t n);

#endif /* CONFIGD_UTIL__H */
