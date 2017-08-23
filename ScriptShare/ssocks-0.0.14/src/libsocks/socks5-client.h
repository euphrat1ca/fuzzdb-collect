/*
 *      socks5-client.h
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
#include "client.h"

void build_version(s_socks *s, s_socks_conf *c, s_buffer *buf);
int test_version_ack(s_socks *s, s_socks_conf *c, s_buffer *buf);

int build_auth(s_socks *s, s_socks_conf *c, s_buffer *buf);
int test_auth_ack(s_socks *s, s_socks_conf *c, s_buffer *buf);

void build_request(s_socks *s, s_socks_conf *c, s_buffer *buf);
int test_request_ack(s_socks *s, s_socks_conf *c, s_buffer *buf);


int dispatch_client_write(s_socket *soc, s_socks *socks,
		s_buffer *buf, s_socks_conf *conf);
int dispatch_client_read(s_socket *soc, s_socket *soc_stream,
		s_socks *socks, s_buffer *buf, s_buffer *buf_stream, s_socks_conf *conf);

void dispatch_client(s_client *client, fd_set *set_read, fd_set *set_write);

void dispatch_dynamic(s_client *client, fd_set *set_read, fd_set *set_write);

void init_select_client (s_socket *soc, s_socks *s, s_buffer *buf, int *maxfd,
		fd_set *set_read, fd_set *set_write);

void init_select_dynamic (int soc_ec, s_client *tc, int *maxfd,
		fd_set *set_read, fd_set *set_write);

int new_socket_with_socks(s_socket *s,
		char *sockshost, int socksport,
		char *username, char *password,
		char *host, int port, int listen,
		int version, int ssl, int cmd);
