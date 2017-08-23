/*
 *      net-util.c
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
#include "net-util.h"
#include "bor-util.h"
#include "output-util.h"

int new_socket_tcpip(int port, struct sockaddr_in *addr){
	int soc;
	/* Internet domain socket creation in connected mode */
	soc = socket(AF_INET, SOCK_STREAM, 0);
	if ( soc < 0 ){ perror("socket"); return -1; }
	
	/* Making local address */
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	/* All Local Addresses */
	addr->sin_addr.s_addr = htonl(INADDR_ANY);
	
	TRACE(L_DEBUG, "socket: attachment to a local socket port ...");
	if ( bor_bind_in(soc, addr) < 0 ) {
		close(soc);
		return -1;
	}
	
	/* Recovering the client port */
	if ( bor_getsockname_in(soc, addr) < 0 ){
		close(soc);
		return -1;
	}
	
	TRACE(L_DEBUG, "socket: local port %d open", ntohs(addr->sin_port));
	return soc;
}

int build_addr(char ip[4], int port, struct sockaddr_in *addr){
	struct hostent *hp;
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	memcpy(&addr->sin_addr.s_addr, ip, 4);
	return 1;
}

int build_addr_server(char *name, int port, struct sockaddr_in *addr){
	struct hostent *hp;
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	TRACE(L_DEBUG, "dns: server address resolution %s ...", name);
	if( ( hp = gethostbyname(name) ) == NULL ){
		herror("gethostbyname");
		return -1;
	}
	memcpy(&addr->sin_addr.s_addr, hp->h_addr, hp->h_length);
	return 1;
}

static char* _uitoa(unsigned int n, char (*digits)[21]) {
	char *p = &(*digits)[sizeof(*digits)-1];

	*p = '\0';
	do {
		p--;
		*p = '0' + (n%10);
		n = n / 10;
	} while (n>0);

	return p;
}

int new_listen_socket (const char *bindAddr, int nport, int backlog, struct sockaddr_in *addrS){

    int soc_ec;
    int error;
    char portstring[21];

    struct addrinfo hints = {0,};
    struct addrinfo *res = NULL;
    struct addrinfo *rp = NULL;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG | AI_PASSIVE | AI_NUMERICSERV;

    error = getaddrinfo(bindAddr, _uitoa(nport, &portstring), &hints, &res);
    if (0 != error) {
	    ERROR(L_NOTICE, "server: resolution error in getaddrinfo: %s\n", gai_strerror(error));
	    return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
	    soc_ec = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	    if (soc_ec == -1) continue;

	    if (bind(soc_ec, rp->ai_addr, rp->ai_addrlen) == 0) {
		    memcpy(addrS, rp->ai_addr, sizeof(*addrS));
		    break;
	    }

	    close(soc_ec);
    }

    freeaddrinfo(res);
    res = NULL;

    if (rp == NULL) {
	    ERROR(L_NOTICE, "server: could not bind any address.");
	    return -1;
    }

    /* Recovery of the port as network endian */
    if (bor_getsockname_in (soc_ec, addrS) < 0)
      { perror ("getsockname ip"); close (soc_ec); return -1; }
    TRACE(L_DEBUG, "server: port %d open", ntohs(addrS->sin_port));
      
    /* Ouverture du service ; le second param est le nb max de connexions
       pendantes, limité à SOMAXCONN (=128 sur Linux) */
    if (listen (soc_ec, backlog) < 0) { perror ("listen"); return -1; }
    TRACE(L_NOTICE, "server: listening on %s", bor_adrtoa_in (addrS));
    
    return soc_ec;
}

int new_client_socket_no_ip(char ip[4], uint16_t nport, struct sockaddr_in *addrC,
		struct sockaddr_in *addrS){
	int soc = new_socket_tcpip(0, addrC);

	if ( soc < 0 ){
		return -1;
	}
	set_non_blocking(soc);
	if ( build_addr(ip, nport, addrS) < 0 ){
		close(soc);
		return -1;
	}

	TRACE(L_VERBOSE, "client: server connection on %s:%d ...",
			inet_ntoa(addrS->sin_addr), ntohs(addrS->sin_port));
	connect (soc, (struct sockaddr *) addrS, sizeof(struct sockaddr_in));

	return soc;
}

int new_client_socket_no(char *nameS, uint16_t nport, struct sockaddr_in *addrC,
		struct sockaddr_in *addrS){
	int soc = new_socket_tcpip(0, addrC);

	if ( soc < 0 ){
		return -1;
	}
	set_non_blocking(soc);
	if ( build_addr_server(nameS, nport, addrS) < 0 ){
		close(soc);
		return -1;
	}

	TRACE(L_VERBOSE, "client: server connection on %s:%d ...",
		nameS, ntohs(addrS->sin_port));
	connect (soc, (struct sockaddr *) addrS, sizeof(struct sockaddr_in));

	return soc;
}

int new_client_socket(char *nameS, uint16_t nport,
		struct sockaddr_in *addrC, struct sockaddr_in *addrS){
	int soc;
	/*struct sockaddr_in addrS, addrC;*/
	
	soc = new_socket_tcpip(0, addrC);
	if ( soc < 0 ){
		return -1;
	}
	
	if ( build_addr_server(nameS, nport, addrS) < 0 ){
		close(soc);
		return -1;
	}
	
	TRACE(L_VERBOSE, "client: server connection on %s:%d ...", 
		nameS, ntohs(addrS->sin_port));
	if ( bor_connect_in(soc, addrS) < 0 ){
		close(soc);
		return -1;
	}

	/* Recovering the client address and port after the connection*/
	if ( bor_getsockname_in(soc, addrC) < 0 ){
		close(soc);
		return -1;
	}

	//TRACE(L_VERBOSE, "client: established connection ...");
	return soc;
}

int set_blocking(int fd){
	int  r = 0;
	return r;
}

/* Find on the board don't remember were,
 * I just add error handling with perror
 */
int set_non_blocking(int fd){
    int flags, r;

    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* FIXME: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (r < 0) bor_perror (__func__);
    return r;
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    r = ioctl(fd, FIOBIO, &flags);
    if (r < 0) bor_perror (__func__);
    return r;
#endif
}
