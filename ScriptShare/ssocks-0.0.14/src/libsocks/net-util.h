/*
 *      net-util.h
 *      
 *      Created on: 2011-03-30
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

#ifndef NET_UTIL__H
#define NET_UTIL__H


#include <netinet/ip.h>
#include <config.h>


/* Prototypes  */
int new_socket_tcpip(int port, struct sockaddr_in *addr);
int build_addr_server(char *name, int port, struct sockaddr_in *addr);
int new_listen_socket (const char *name, int nport, int maxpend, struct sockaddr_in *addrS);
int new_client_socket(char *nameS, uint16_t nport,
		struct sockaddr_in *addrC, struct sockaddr_in *addrS);

int new_client_socket_no(char *nameS, uint16_t nport, struct sockaddr_in *addrC,
		struct sockaddr_in *addrS);

int new_client_socket_no_ip(char ip[4], uint16_t nport, struct sockaddr_in *addrC,
		struct sockaddr_in *addrS);

int set_blocking(int fd);
int set_non_blocking(int fd);



#endif /* NET_UTIL__H */

