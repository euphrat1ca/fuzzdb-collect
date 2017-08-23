/*
 *      socks4.c
 *
 *      Created on: 2011-04-11
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
#include "socks4.h"

#include <string.h>
#include <stdio.h>


int test_request4(s_socks *s, s_socket *stream, s_socket *bind,
		s_socks_conf *c, s_buffer *buf)
{
	Socks4Req req;
	TRACE(L_DEBUG, "server [%d]: testing client request ...",
		s->id);

	uint16_t port = 0, *p;

	char domain[256];
	unsigned char chAddr[4];
	unsigned int l;

	/* Rebuild the packet but don't extract
	 * DST.ADDR and DST.PORT in Socks5Req struct */
	memcpy(&req, buf->data, sizeof(Socks4Req));
	TRACE(L_DEBUG, "server [%d]: v0x%x, cmd 0x%x,",
		s->id, req.ver,
		req.cmd );

	/* Save the request cmd */
	s->cmd = req.cmd;

	/* CMD:
	 *  - CONNECT X'01'
	 *  - BIND X'02'
	 *  - UDP ASSOCIATE X'03'
	 *
	 * Open or bind connection here
	 */
	switch(req.cmd){
		case 0x01: /* TCP/IP Stream connection */
			stream->soc = new_client_socket_no_ip(req.dstadr,
				ntohs(req.dstport), &stream->adrC, &stream->adrS);
			if ( stream->soc < 0 ){
				return -3;
			}
			break;
		case 0x02: /* TCP/IP port binding */
			bind->soc = new_listen_socket(req.dstport, 10, &bind->adrC);
			if ( bind->soc >= 0 ){
				s->connected = 0;
				s->listen = 1;
			}

			break;
		/* TODO: udp support */
		default :
			ERROR(L_NOTICE, "server [%d]: doesn't support udp",
				s->id);
			return -2;
	}

	return 0;
}

void build_request_ack4(s_socks *s, s_socks_conf *c,
		s_socket *stream, s_socket *bind, s_buffer *buf)
{

	Socks4ReqAck res;
	int k;
	res.ver = 0x00;
	socklen_t socklen = sizeof(int);

	init_buffer(buf);

	switch(s->cmd){
		case 0x01:
			/* 0x00 succeeded, 0x01 general SOCKS failure ... */

			if ( getsockopt(stream->soc, SOL_SOCKET, SO_ERROR, &k, &socklen) < 0){
				perror("getsockopt");
				close_socket(stream);
				s->connected = 0;
				res.rep = 0x5b;
				break;
			}

			if (k != 0){
				ERROR(L_VERBOSE, "client: error %d", k);
				close_socket(stream);
				s->connected = 0;
				res.rep = 0x5b;
				break;
			}

			/* Recovering the client address and port after the connection*/
			if ( bor_getsockname_in(stream->soc, &stream->adrC) < 0 ){
				close_socket(stream);
				s->connected = 0;
				res.rep = 0x5b;
				break;
			}

			/* In the reply to a CONNECT, BND.PORT contains
			 * the port number that the server assigned to
			 * connect to the target host, while BND.ADDR
			 * contains the associated IP address. */
			res.rep = 0x5a;
			s->connected = 1;
			/*memcpy(&res.bndaddr, &stream->adrC.sin_addr.s_addr,
					sizeof(stream->adrC.sin_addr.s_addr));
			memcpy(&res.bndport, &stream->adrC.sin_port,
					sizeof(stream->adrC.sin_port));*/
			break;

		case 0x02:
			/* In the reply to a BIND, two replies are sent from the SOCKS server
			 * to the client during a BIND operation. */
			if ( s->listen == 1 && s->connected == 0 ){
				/* First replies
				 * The BND.PORT field contains the port number that the
				 * SOCKS server assigned to listen for an incoming connection.  The
				 * BND.ADDR field contains the associated IP address.*/
				res.rep = 0x5a;
				/*memcpy(&res.bndaddr, &bind->adrC.sin_addr.s_addr,
						sizeof(bind->adrC.sin_addr.s_addr));
				memcpy(&res.bndport, &bind->adrC.sin_port,
						sizeof(bind->adrC.sin_port));*/
			}else if ( s->listen == 1 && s->connected == 1 ){
				/* Second replies
				 * The second reply occurs only after the anticipated incoming
	   	   	   	 * connection succeeds or fails. In the second reply,
	   	   	   	 * the BND.PORT and BND.ADDR fields contain the
	   	   	   	 * address and port number of the connecting host.*/
				res.rep = 0x5a;
				/*memcpy(&res.bndaddr, &stream->adrC.sin_addr.s_addr,
						sizeof(stream->adrC.sin_addr.s_addr));
				memcpy(&res.bndport, &stream->adrC.sin_port,
						sizeof(stream->adrC.sin_port));*/
			}else{
				res.rep = 0x5b;
			}


			break;

		default:
			res.rep = 0x01;
			break;
	}

	/* Copy in buffer for send */
	memcpy(buf->data, &res, sizeof(Socks4ReqAck));

	/* Reset counter and fix b flag */
	buf->a = 0;
	buf->b = sizeof(Socks4ReqAck);
}
