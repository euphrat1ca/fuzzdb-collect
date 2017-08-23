/*
 *      socks5-server.c
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

#include "socks-common.h"
#include "socks5-server.h"
#include "net-util.h"
#include "log-util.h"


#include "bor-util.h"

#include <config.h>


/* Test version packet in buf,
 * It check if version is allowed in c->config.srv->allowed_version
 * and save it in s->version
 * It check if version is allowed in c->config.srv->allowed_methods
 * and save it in s->method
 *
 * Return:
 * 	-1, error wrong version
 * 	-2, error wrong method
 * 	 0, success
 *
 * From RFC1928:
 * The client connects to the server, and sends a version
 * identifier/method selection message:
 *
 * +----+----------+----------+
 * |VER | NMETHODS | METHODS  |
 * +----+----------+----------+
 * | 1  |    1     | 1 to 255 |
 * +----+----------+----------+
 *
 * The values currently defined for METHOD are:
 * o  X'00' NO AUTHENTICATION REQUIRED (supported)
 * o  X'01' GSSAPI
 * o  X'02' USERNAME/PASSWORD (supported)
 * o  X'03' to X'7F' IANA ASSIGNED
 * o  X'80' to X'FE' RESERVED FOR PRIVATE METHODS
 * o  X'FF' NO ACCEPTABLE METHODS
 */
int test_version(s_socks *s, s_socks_conf *c, s_buffer *buf){
	int i, j;
	Socks5Version req;
	TRACE(L_DEBUG, "server [%d]: testing version ...",
		s->id);

	memcpy(&req, buf->data, sizeof(Socks5Version));

	/* Testing version */
	char *allowed;
	for (i=0; i <  c->config.srv->n_allowed_version; ++i){
		allowed = &c->config.srv->allowed_version[i];
		if ( *allowed == req.ver ){
			s->version = *allowed;
			TRACE(L_DEBUG, "server [%d]: version %d",
				s->id, s->version);
			break;
		}
	}

	/* No valid version find */
	if ( s->version == -1 ){
		ERROR(L_VERBOSE, "server [%d]: version error (%d)",
			s->id, req.ver);
		return -1;
	}

	if ( s->version == SOCKS4_V){
		return 0;
	}

	/* If too much method we truncate */
	if (sizeof(req.methods) < (unsigned int)req.nmethods){
		ERROR(L_VERBOSE, "server [%d]: truncate methods",
			s->id);
		req.nmethods = sizeof(req.methods);
	}

	/* Show only in debug mode */
	if ( L_DEBUG <= verbosity ){
		printf("server [%d]: methods ", s->id);
	}

	/* Copy in methods the methods in the packet
	 * memcpy can do the trick too */
	for (i=0; i <  req.nmethods; ++i){
		req.methods[i] = *(buf->data + 2 + i );
		/* Show only in debug mode */
		if ( L_DEBUG <= verbosity ){
			printf("0x%02X,",req.methods[i]);
		}
	}

	/* Show only in debug mode */
	if ( L_DEBUG <= verbosity ){
		printf("\n");
	}

	/* Searching valid methods:
	 * Methods 0x00, no authentication
	 *         0x01, GSSAPI no supported
	 *         0x02, username/password RFC1929
	 */
	for (i=0; i <  req.nmethods && s->method == -1; ++i){
		for (j = 0; j < c->config.srv->n_allowed_method; ++j ){
			if ( c->config.srv->allowed_method[j] == req.methods[i] ){
				s->method = c->config.srv->allowed_method[j];
				break;
			}
		}
	}

	/* No valid method find */
	if ( s->method == -1 ){
		ERROR(L_VERBOSE, "server [%d]: method not supported",
			s->id);
		return -2;
	}

	return 0;
}

/* Build version packet ack in buf
 *
 * From RFC1928:
 * The server selects from one of the methods given in METHODS, and
 * sends a METHOD selection message:
 *
 * +----+--------+
 * |VER | METHOD |
 * +----+--------+
 * | 1  |   1    |
 * +----+--------+
 */
void build_version_ack(s_socks *s, s_socks_conf *c, s_buffer *buf)
{
	Socks5VersionACK res;
	init_buffer(buf);
	res.ver = s->version;
	res.method = s->method;

	/* Copy in buffer for send */
	memcpy(buf->data, &res, sizeof(Socks5VersionACK));

	/* Reset counter and fix b flag */
	buf->a = 0;
	buf->b = sizeof(Socks5VersionACK);
}

/* Analyse authentication packet and check uname/password
 * It set s->auth to 0 if authentication failed and to 1 if success
 *
 * Return:
 *  -1 error, wrong subnegotiation version
 *   0 success
 * From RFC1929:
 * Once the SOCKS V5 server has started, and the client has selected the
 * Username/Password Authentication protocol, the Username/Password
 * subnegotiation begins.  This begins with the client producing a
 * Username/Password request:
 *
 * +----+------+----------+------+----------+
 * |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
 * +----+------+----------+------+----------+
 * | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
 * +----+------+----------+------+----------+
 *
 * The VER field contains the current version of the subnegotiation,
 * which is X'01'
 */
int test_auth(s_socks *s, s_socks_conf *c, s_buffer *buf)
{
	Socks5Auth req;

	TRACE(L_DEBUG, "server [%d]: testing authentication ...",
		s->id);

	/* Rebuild the packet in Socks5Auth struct */
	memcpy(&req, buf->data, 2);
	memcpy(&req.plen, buf->data + 2 + (int)req.ulen , 2);

	/* Check username and password length truncate if too long
	 * RFC tell us max length 255 */
	if ( (unsigned int)req.ulen > sizeof(req.uname)-1){
		ERROR(L_NOTICE, "server [%d]: username too long",
			s->id);
		req.ulen = sizeof(req.uname)-1;

		ERROR(L_VERBOSE, "server [%d]: authentication NOK!", 
			s->id);
		s->auth = 0;

		return 0;
	}
	if ( (unsigned int)req.plen > sizeof(req.passwd)-1){
		ERROR(L_NOTICE, "server [%d]: password too long",
			s->id);
		req.plen = sizeof(req.passwd)-1;

		ERROR(L_VERBOSE, "server [%d]: authentication NOK!", 
			s->id);
		s->auth = 0;
		
		return 0;
	}

	/* Extract username and fix NULL byte */
	strncpy(req.uname, buf->data + 2, req.ulen);
	*(req.uname + req.ulen) = '\0';

	/* Extract passwd and fix NULL byte */
	strncpy(req.passwd, buf->data + 2 + (int)req.ulen + 1, req.plen);
	*(req.passwd + req.plen) = '\0';
	//DUMP(buf->data, buf->b);
	TRACE(L_VERBOSE, "server [%d]: authentication attempt "\
						"v0x%02X (%d,%d) %s:%s",
		s->id,
		req.ver, req.ulen, req.plen, req.uname, req.passwd);

	/* Test version need 0x01 RFC */
	if ( req.ver != 0x01 ){
		ERROR(L_NOTICE, "server [%d]: wrong subnegotiation version need to be 0x01",
			s->id);
		return -1;
	}

	/* Check username and password in authentication file */
	if ( c->config.srv->check_auth == NULL ){
		ERROR(L_NOTICE, "server [%d]: wrong configuration no check_auth callback set",
			s->id);
		return -2;
	}
	if ( (*c->config.srv->check_auth)(req.uname, req.passwd) == 1 ){
		TRACE(L_VERBOSE, "server [%d]: authentication OK!",
			s->id);
		strcpy(s->uname, req.uname);
		s->auth = 1;
	}else{
		ERROR(L_VERBOSE, "server [%d]: authentication NOK!",
			s->id);
		s->auth = 0;
	}

	return 0;
}

/* Build authentication packet ack in buf
 * Check s->auth to set status field
 *
 * From RFC1929:
 * The server verifies the supplied UNAME and PASSWD, and sends the
 * following response:
 *
 * +----+--------+
 * |VER | STATUS |
 * +----+--------+
 * | 1  |   1    |
 * +----+--------+
 *
 * The VER field contains the current version of the subnegotiation,
 * which is X'01'
 *
 * A STATUS field of X'00' indicates success. If the server returns a
 *`failure' (STATUS value other than X'00') status, it MUST close the
 * connection.
 */
void build_auth_ack(s_socks *s, s_socks_conf *c, s_buffer *buf)
{
	Socks5AuthACK res;
	init_buffer(buf);
	res.ver = 0x01;
	res.status = (s->auth) ? 0x00 : 0xFF; /* 0x00 == win! */

	/* Copy in buffer for send */
	memcpy(buf->data, &res, sizeof(Socks5VersionACK));

	/* Reset counter and fix b flag */
	buf->a = 0;
	buf->b = sizeof(Socks5VersionACK);
}

/* Test request packet in buf, and execute the request
 *
 * Return:
 * 	-EINVAL, invalid argument (typ, cmd, udp)
 *      -EAGAIN, expects more bytes in buffer
 *      -1, other error
 * 	 0, success
 *
 * From RFC1928:
 * The SOCKS request is formed as follows:
 * +----+-----+-------+------+----------+----------+
 * |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
 * +----+-----+-------+------+----------+----------+
 * | 1  |  1  | X'00' |  1   | Variable |    2     |
 * +----+-----+-------+------+----------+----------+
 *
 * Where:
 * o  VER    protocol version: X'05'
 * o  CMD
 *	 o  CONNECT X'01' ( define in CMD_CONNECT )
 *	 o  BIND X'02'    ( define in CMD_BIND )
 *	 o  UDP ASSOCIATE X'03'  ( define in CMD_UDP )
 * o  RSV    RESERVED
 * o  ATYP   address type of following address
 *	 o  IP V4 address: X'01'
 *	 o  DOMAINNAME: X'03'
 *	 o  IP V6 address: X'04'
 * o  DST.ADDR       desired destination address
 * o  DST.PORT desired destination port in network octet
 *	 order
 *
 * TODO: server implement UDP support CMD_UDP
 * TODO: server implement ATYP with IPv6
 */
int analyse_request(s_socks *s, s_socket *stream, s_socket *bind,
		s_socks_conf *c, s_buffer *buf)
{
	Socks5Req req;
	TRACE(L_DEBUG, "server [%d]: testing client request ...",
		s->id);

	uint16_t port = 0, *p;
	char domain[256];
	char chAddr[4];
	uint8_t l;
	struct in_addr addr;
	/* Rebuild the packet but don't extract
	 * DST.ADDR and DST.PORT in Socks5Req struct */
	memcpy(&req, buf->data, sizeof(Socks5Req));
	TRACE(L_DEBUG, "server [%d]: v0x%x, cmd 0x%x, rsv 0x%x, atyp 0x%x",
		s->id, req.ver,
		req.cmd, req.rsv, req.atyp);

	/* Save the request cmd */
	s->cmd = req.cmd;

	/* Check ATYP
	 * ATYP address type of following address
	 *    -  IP V4 address: X'01'
	 *    -  DOMAINNAME: X'03'
	 *    -  IP V6 address: X'04'
	 */
	switch ( req.atyp ){
		case 0x03: /* Domain name */

			if (!PEEK_DISP(buf, sizeof(Socks5Req) + 1)) return -EAGAIN;

			/* First byte is the domain len */
			l = *(buf->data + sizeof(Socks5Req));

			/* 1 addrlen + l domain + 2 port */
			if (!PEEK_DISP(buf, sizeof(Socks5Req) + 1 + l + 2)) return -EAGAIN;

			/* Copy the domain name and blank at end
			 * little cheat to avoid overflow (dangerous here) */
			memcpy(domain, buf->data + sizeof(Socks5Req) + 1, l);
			domain[(int)l] = '\0';

			/* After domain we have the port
			 * big endian on 2 bytes*/
			p = (uint16_t*)(buf->data + sizeof(Socks5Req) + l + 1) ;
			port = ntohs(*p);

			TRACE(L_DEBUG, "Server [%d]: asking for %s:%d", s->id, domain, port);
			break;

		case 0x01: /* IP address */
			
			if (!PEEK_DISP(buf, sizeof(Socks5Req) + sizeof(chAddr) + 2)) return -EAGAIN;

			memcpy(&chAddr, (buf->data + sizeof(Socks5Req)), sizeof(chAddr));
			inet_aton(chAddr, &addr);

			/* After domain we have the port
			 * big endian on 2 bytes*/
			p = (uint16_t*)(buf->data + sizeof(Socks5Req) + 4  ) ;
			port = ntohs(*p);
			break;

		/* TODO: ipv6 support */
		default:
			ERROR(L_NOTICE, "server [%d]: support domain name "\
								"and ipv4 only",
				s->id);
			return -EINVAL;
	}



	/* CMD:
	 *  - CONNECT X'01'
	 *  - BIND X'02'
	 *  - UDP ASSOCIATE X'03'
	 *
	 * Open or bind connection here
	 */
	switch(req.cmd){
		case 0x01: /* TCP/IP Stream connection */
			if ( req.atyp == 0x01 )
				stream->soc = new_client_socket_no_ip(chAddr, port, &stream->adrC, &stream->adrS);
			else
				stream->soc = new_client_socket_no(domain, port, &stream->adrC, &stream->adrS);
			if ( stream->soc < 0 ){
				return -1;
			}
			break;
		case 0x02: /* TCP/IP port binding */
			bind->soc = new_listen_socket(NULL, port, 10, &bind->adrC);
			if ( bind->soc >= 0 ){
				s->connected = 0;
				s->listen = 1;

			}

			break;
		/* TODO: udp support */
		default :
			ERROR(L_NOTICE, "server [%d]: doesn't support udp",
				s->id);
			return -EINVAL;
	}

	return 0;
}

int test_request_dynamic(s_socks *s, s_socks_conf *c, s_buffer *buf)
{
	return -1;
}

/* Build request packet ack in buf
 *
 * From RFC1928:
 * The server verifies the supplied UNAME and PASSWD, and sends the
 * following response:
 * +----+-----+-------+------+----------+----------+
 * |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
 * +----+-----+-------+------+----------+----------+
 * | 1  |  1  | X'00' |  1   | Variable |    2     |
 * +----+-----+-------+------+----------+----------+
 *
 * Where:
 * o  VER    protocol version: X'05'
 * o  REP    Reply field:
 *	 o  X'00' succeeded
 *	 o  X'01' general SOCKS server failure
 *	 o  X'02' connection not allowed by ruleset
 *	 o  X'03' Network unreachable
 *	 o  X'04' Host unreachable
 *	 o  X'05' Connection refused
 * 	 o  X'06' TTL expired
 *	 o  X'07' Command not supported
 *	 o  X'08' Address type not supported
 *	 o  X'09' to X'FF' unassigned
 * o  RSV    RESERVED (must be set to 0x00)
 * o  ATYP   address type of following address
 * o  IP V4 address: X'01'
 *	 o  DOMAINNAME: X'03'
 *	 o  IP V6 address: X'04'
 * o  BND.ADDR       server bound address
 * o  BND.PORT       server bound port in network octet order
 *
 * TODO: Handle server request error, in case of fails send 0X01
 */
void build_request_ack(s_socks *s, s_socks_conf *c,
		s_socket *stream, s_socket *bind, s_buffer *buf)
{

	Socks5ReqACK res;
	int k;
	socklen_t socklen = sizeof(int);
	res.ver = s->version;
	res.rsv = 0;
	res.atyp = 0x01;

	init_buffer(buf);

	switch(s->cmd){
		case 0x01:
			/* 0x00 succeeded, 0x01 general SOCKS failure ... */

			if ( getsockopt(stream->soc, SOL_SOCKET, SO_ERROR, &k, &socklen) < 0){
				perror("getsockopt");
				close_socket(stream);
				s->connected = 0;
				res.rep = 0x01;
				break;
			}

			if (k != 0){
				ERROR(L_VERBOSE, "client: error %d", k);
				close_socket(stream);
				s->connected = 0;
				res.rep = 0x01;
				break;
			}

			/* Recovering the client address and port after the connection*/
			if ( bor_getsockname_in(stream->soc, &stream->adrC) < 0 ){
				close_socket(stream);
				s->connected = 0;
				res.rep = 0x01;
				break;
			}

			/* In the reply to a CONNECT, BND.PORT contains
			 * the port number that the server assigned to
			 * connect to the target host, while BND.ADDR
			 * contains the associated IP address. */
			res.rep = 0x00;
			s->connected = 1;
			memcpy(&res.bndaddr, &stream->adrC.sin_addr.s_addr,
					sizeof(stream->adrC.sin_addr.s_addr));
			memcpy(&res.bndport, &stream->adrC.sin_port,
					sizeof(stream->adrC.sin_port));
			break;

		case 0x02:
			/* In the reply to a BIND, two replies are sent from the SOCKS server
			 * to the client during a BIND operation. */
			if ( s->listen == 1 && s->connected == 0 ){
				/* First replies
				 * The BND.PORT field contains the port number that the
				 * SOCKS server assigned to listen for an incoming connection.  The
				 * BND.ADDR field contains the associated IP address.*/
				res.rep = 0x00;
				memcpy(&res.bndaddr, &bind->adrC.sin_addr.s_addr,
						sizeof(bind->adrC.sin_addr.s_addr));
				memcpy(&res.bndport, &bind->adrC.sin_port,
						sizeof(bind->adrC.sin_port));
			}else if ( s->listen == 1 && s->connected == 1 ){
				/* Second replies
				 * The second reply occurs only after the anticipated incoming
	   	   	   	 * connection succeeds or fails. In the second reply,
	   	   	   	 * the BND.PORT and BND.ADDR fields contain the
	   	   	   	 * address and port number of the connecting host.*/
				res.rep = 0x00;
				memcpy(&res.bndaddr, &stream->adrC.sin_addr.s_addr,
						sizeof(stream->adrC.sin_addr.s_addr));
				memcpy(&res.bndport, &stream->adrC.sin_port,
						sizeof(stream->adrC.sin_port));
			}else{
				res.rep = 0x01;
			}


			break;

		default:
			res.rep = 0x01;
			break;
	}

	/* Copy in buffer for send */
	memcpy(buf->data, &res, sizeof(Socks5ReqACK));

	/* Reset counter and fix b flag */
	buf->a = 0;
	buf->b = sizeof(Socks5ReqACK);
}

/* Build request accept bind packet in buf
 * It's accept a connection on the socket bind
 * and associate with socket stream
 *
 * Return:
 * 	-1 error, accept error socket problem
 * 	 0 success
 *
 */
int build_request_accept_bind(s_socks *s, s_socks_conf *c,
		s_socket *stream, s_socket *bind, s_buffer *buf)
{
	init_buffer(buf);

	TRACE(L_VERBOSE, "server [%d]: build binding packet ...",
		s->id);

	stream->soc  = bor_accept_in (bind->soc, &stream->adrC);
	if ( stream->soc < 0 ){
		s->connected = -1; /* Send a error request ack */
		return -1;
	}

	s->connected = 1;

	TRACE(L_DEBUG, "server: established connection with %s",
		bor_adrtoa_in(&stream->adrC));

	build_request_ack(s, c, stream, bind, buf);

	return 0;
}

/* Dispatch server write state, following socks5 RFC
 * In each state, it deal with write buf on soc and
 * change state to next
 *
 * Return:
 * 	-1, error something happen we need to disconnect the client
 * 	 0, success
 */
int dispatch_server_write(s_socket *soc, s_socket *soc_stream, s_socks *socks,
		s_buffer *buf, s_socks_conf *conf)
{
	int k = 0;
	socklen_t socklen = sizeof(int);
	if ( soc->con == 0 ) {
		if ( getsockopt(soc->soc, SOL_SOCKET, SO_ERROR, &k, &socklen) < 0){
			perror("getsockopt");
			k = -1;
			return k;
		}

		if (k != 0){
			ERROR(L_VERBOSE, "server [%d]: error %d", socks->id, k);
			k = -1;
			return k;
		}

		/* Recovering the client address and port after the connection*/
		if ( bor_getsockname_in(soc->soc, &soc->adrC) < 0 ){
			k = -1;
			return k;
		}
		TRACE(L_VERBOSE, "server [%d]: server connection on %s OK",socks->id,
			bor_adrtoa_in(&soc->adrS));
		soc->con = 1;
		return;
	}
	switch(socks->state){
		case S_W_VER_ACK:
			WRITE_DISP(k, soc, buf);

			if ( socks->method == 0x02 )
				socks->state = S_R_AUTH;
			else
				socks->state = S_R_REQ;

			break;

		case S_W_AUTH_ACK:
			WRITE_DISP(k, soc, buf);
			if ( socks->auth == 0 ){
				/* close_socket(soc); */
				k = -1;
				break;
			}
			socks->state = S_R_REQ;
			break;

		case S_W_REQ_ACK:
			if ( buf_empty(buf) ){
				if ( socks->version == SOCKS4_V ){
					build_request_ack4(socks, conf,
						soc_stream, NULL,
						buf);
				}else{
					build_request_ack(socks, conf,
						soc_stream, NULL,
						buf);
				}
			}

			WRITE_DISP(k, soc, buf);
			/* If listen and not connected we are in bind mode */
			if ( socks->listen == 1 && socks->connected == 0 ){
				/* Wait until a soc_bind accept a connection */
				socks->state = S_WAIT;
			}else if (socks->connected == 1){
				write_log(socks, soc, soc_stream);
				/* We are connected let's go */
				socks->state = S_REPLY;
			}else{
				/* Error not connected, normally can happen only in bind mode
				 * we return a error */
				k = -1;
			}
			break;

		case S_REPLY:
			k = write_socks(soc, buf);
			if (k < 0){ /* close_socket(soc); */ break; } /* Error */
			init_buffer(buf);
			break;

		default:
			break;
	}

	return k;
}

/* Dispatch server read state, following socks5 RFC
 * In each state, it deal with read in buf with soc,
 * test the packet with right function, build a reponse and
 * change state to next.
 *
 * Return:
 * 	-1, error something happen we need to disconnect the client
 * 	 0, success
 */
int dispatch_server_read(s_socket *soc, s_socket *soc_stream, s_socket *soc_bind,
		s_socks *socks, s_buffer *buf, s_buffer *buf_stream, s_socks_conf *conf){
	int k = 0;
	struct sockaddr_in adrC, adrS;

	switch(socks->state){
		case S_R_VER:
			READ_DISP(k, soc, buf, 3);

			k = test_version(socks, conf, buf);

			if (k < 0){ /* close_socket(soc); */ break; } /* Error */
			if ( socks->version == SOCKS4_V ){
				k = test_request4(socks,
						soc_stream, soc_bind,
						conf, buf);
				init_buffer(buf);
				if (k < 0){ break; } /* Error */

				socks->state = S_WAIT;
				break;
			}

			build_version_ack(socks, conf,
								buf);

			socks->state = S_W_VER_ACK;

			break;

		case S_R_AUTH:
			READ_DISP(k, soc, buf, 4);

			k = test_auth(socks, conf,
								buf);
			if (k < 0){ /* close_socket(soc); */ break; } /* Error */

			build_auth_ack(socks, conf,
								buf);

			socks->state = S_W_AUTH_ACK;

			break;

		case S_R_REQ:
			if ( socks->mode == M_DYNAMIC){
				READ_DISP(k, soc, buf,
					sizeof(Socks5Req)  + 4);
				soc_stream->soc = new_client_socket(conf->config.cli->sockshost,
								    conf->config.cli->socksport,
								    &adrC, &adrS);

				if ( soc_stream->soc < 0 ){
					ERROR(L_NOTICE, "client: connection to socks5 server impossible!");
					k = -1;
					/* close_socket(soc); */
				}

#ifdef HAVE_LIBSSL
				/* Init SSL here
				 */
				if (soc_stream->want_ssl == 1){
					TRACE(L_DEBUG, "client: socks5 enable ssl ...");
					soc_stream->ssl = ssl_neogiciate_client(soc_stream->soc);
					if ( soc_stream->ssl == NULL ){
						ERROR(L_VERBOSE, "client: ssl error");
						return -3;
					}
					TRACE(L_DEBUG, "client: ssl ok.");
				}
#endif /* HAVE_LIBSSL */

				socks->state = S_WAIT;
				break;
			}

			READ_DISP(k, soc, buf, sizeof(Socks5Req));

			k = analyse_request(socks, soc_stream, soc_bind, conf, buf);
			if (k == -EAGAIN) { k=0; break; }

			init_buffer(buf);
			if (k < 0){ break; } /* Error */

			socks->state = S_WAIT;
			break;

		case S_REPLY:
			if ( buf_free(buf_stream) > 0 ){
				k = read_socks(soc, buf_stream, 0);
				if (k < 0){ /* close_socket(soc); */ break; } /* Error */
			}
			break;
		default:
			break;
	}
	return k;
}

/* Dispatch server, it's normally called after a select
 * Search client with socket in set_read and set_write and call
 * the right dispatcher.
 *
 * It's responsible for disconnecting the client
 * in case of protocol error or network error.
 */
int dispatch_server(s_client *client, fd_set *set_read, fd_set *set_write)
{
	int k = 0;

	/* Dispatch server socket */
	if (client->soc.soc != -1 && FD_ISSET (client->soc.soc, set_read))
		k = dispatch_server_read(&client->soc, &client->soc_stream, &client->soc_bind,
				&client->socks, &client->buf, &client->stream_buf, client->conf);

	else if (client->soc.soc != -1 &&
			FD_ISSET (client->soc.soc, set_write))
		k = dispatch_server_write(&client->soc, &client->soc_stream, &client->socks, &client->buf, client->conf);
	if (k < 0){ if (client->soc.con == 0) { k = -2; } disconnection(client); }

	/* Dispatch stream socket */
	if (client->socks.connected == 0 && client->soc_stream.soc != -1){
		if (FD_ISSET (client->soc_stream.soc, set_write)){
			client->socks.state = S_W_REQ_ACK;
		}
	}else if (client->soc_stream.soc != -1
			&& FD_ISSET (client->soc_stream.soc, set_read)){
		if ( buf_free(&client->buf) > 0 ){
			k = read_socks(&client->soc_stream, &client->buf, 0);
			if (k < 0){ disconnection(client); } /* Error */
		}

	}else if (client->soc_stream.soc != -1
			&& FD_ISSET (client->soc_stream.soc, set_write)){

			k = write_socks(&client->soc_stream, &client->stream_buf);
			if (k < 0){ disconnection(client); } /* Error */
			init_buffer(&client->stream_buf);
	}

	if (client->soc_bind.soc != -1 &&
			FD_ISSET (client->soc_bind.soc, set_read)){
		if ( build_request_accept_bind(&client->socks, client->conf,
				&client->soc_stream, &client->soc_bind, &client->buf) == 0 ){
			client->socks.state = S_W_REQ_ACK;
		}
	}
}

/* Prepare set_read and set_write for a select  (ssocksd)
 * Initialize set_read and set_write with right socket in function of socks state
 * It's responsible to set maxfd to max soc->soc value in set_read or set_write
 */
void init_select_server_cli (s_socket *soc,	s_socks *s, s_buffer *buf,
		s_buffer *buf_stream, int *maxfd,	fd_set *set_read, fd_set *set_write){
	if ( soc->soc != -1 && soc->con == 0 ) {
		FD_SET(soc->soc, set_write);
		if (soc->soc > *maxfd) *maxfd = soc->soc;
	}else if ( soc->soc != -1 ){
		if ( s->state == S_R_VER ||
			 s->state == S_R_AUTH ||
			 s->state == S_R_REQ )
		{
			FD_SET(soc->soc, set_read);
		}else if (s->state == S_W_VER_ACK ||
				  s->state == S_W_AUTH_ACK ||
			      s->state == S_W_REQ_ACK)
		{
			FD_SET(soc->soc, set_write);
		}else if (s->state == S_WAIT ){

		}else if (s->state == S_REPLY )	{
			if ( buf_empty(buf) == 0 ){
				FD_SET(soc->soc, set_write);
			}else if ( buf_free(buf_stream) > 0 ){
				FD_SET(soc->soc, set_read);
			}
		}
		if (soc->soc > *maxfd) *maxfd = soc->soc;
	}
}

/* Prepare set_read and set_write for a select  (read and reply)
 * Initialize set_read and set_write with right socket in function of socks state
 * It's responsible to set maxfd to max soc->soc value in set_read or set_write
 */
void init_select_server_stream (s_socket *soc, s_socks *socks, s_buffer *buf,
		s_buffer *buf_stream, int *maxfd,	fd_set *set_read, fd_set *set_write){
	if ( soc->soc != -1 ){
		if ( socks->connected == 0){
			FD_SET(soc->soc, set_write);
		}else if ( buf_empty(buf) == 0 ){
			FD_SET(soc->soc, set_write);
		}else if ( buf_free(buf_stream) > 0 ){
			FD_SET(soc->soc, set_read);
		}
		if (soc->soc > *maxfd) *maxfd = soc->soc;
	}
}

/* TODO: init_select_server
 */
void init_select_server (int soc_ec, s_client *tc, int *maxfd,
		fd_set *set_read, fd_set *set_write)
{
    int nc;
    /* TODO: move FD_ZERO */
    FD_ZERO (set_read);
    FD_ZERO (set_write);
    FD_SET (soc_ec, set_read);

    *maxfd = soc_ec;
    for (nc = 0; nc < MAXCLI; nc++){
		s_client *client = &tc[nc];

		init_select_server_cli(&client->soc, &client->socks, &client->buf, &client->stream_buf,
				maxfd, set_read, set_write);

		init_select_server_stream(&client->soc_stream, &client->socks, &client->stream_buf, &client->buf,
				maxfd, set_read, set_write);


		if ( client->soc_bind.soc != -1 ){
			FD_SET(client->soc_bind.soc, set_read);
			if (client->soc_bind.soc > *maxfd) *maxfd = client->soc_bind.soc;
		}
	}
}

int init_select_server_reverse (s_client *tc, int *maxfd,
		int ncon, fd_set *set_read, fd_set *set_write, int ssl)
{
	/* Security to avoid segmentation fault on tc tab */
	if ( ncon >= MAXCLI ) ncon = MAXCLI-1;

    int nc, cpt = 0;

    FD_ZERO (set_read);
    FD_ZERO (set_write);

    *maxfd = 0;
    for (nc = 0; nc < MAXCLI; nc++){
		s_client *client = &tc[nc];

		/* Count available connection */
		if ( client->soc.soc != -1 ) cpt++;

		init_select_server_cli(&client->soc, &client->socks, &client->buf, &client->stream_buf,
				maxfd, set_read, set_write);

		init_select_server_stream(&client->soc_stream, &client->socks, &client->stream_buf, &client->buf,
				maxfd, set_read, set_write);


		if ( client->soc_bind.soc != -1 ){
			FD_SET(client->soc_bind.soc, set_read);
			if (client->soc_bind.soc > *maxfd) *maxfd = client->soc_bind.soc;
		}
	}

    /* */
	while(cpt < ncon){
		/* Open connection to the socks client */
		for (nc = 0; nc < MAXCLI; nc++) if ( tc[nc].soc.soc == -1 ) break;
		if (nc >= MAXCLI) return;
		/* Remove nonblockant for ssl */
		tc[nc].soc.soc = new_client_socket(tc[nc].conf->config.cli->sockshost,
				tc[nc].conf->config.cli->socksport, &tc[nc].soc.adrC,
				&tc[nc].soc.adrS);
		tc[nc].soc.con = 0;
		if ( tc[nc].soc.soc < 0 ){
			TRACE(L_DEBUG, "client: connection to %s error",
					tc[nc].conf->config.cli);
			return -1;
		}
		tc[nc].soc.con = 1; /* We block so ok */
#ifdef HAVE_LIBSSL
		/* Init SSL here
		 */
		if (ssl == 1){
			TRACE(L_DEBUG, "client: socks5 enable ssl ...");
			tc[nc].soc.ssl = ssl_neogiciate_client(tc[nc].soc.soc);
			if ( tc[nc].soc.ssl == NULL ){
				ERROR(L_VERBOSE, "client: ssl error");
				return -2;
			}
			TRACE(L_DEBUG, "client: ssl ok.");
		}
#endif /* HAVE_LIBSSL */

		set_non_blocking(tc[nc].soc.soc);

		FD_SET(tc[nc].soc.soc, set_write);
		if (tc[nc].soc.soc > *maxfd) *maxfd = tc[nc].soc.soc;
		//init_select_server_cli(&tc[nc].soc, &tc[nc].socks, &tc[nc].buf,
				 //&tc[nc].stream_buf, maxfd, set_read, set_write);
		cpt++;
	}

    return 0;
}
