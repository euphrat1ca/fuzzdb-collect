/*
 *      socks5-server.h
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
#ifndef SOCKS5_SERVER__H
#define SOCKS5_SERVER__H

#include "socks-common.h"
#include "client.h"



int build_request_accept_bind(s_socks *s, s_socks_conf *c,
		s_socket *stream, s_socket *bind, s_buffer *buf);
void build_request_ack(s_socks *s, s_socks_conf *c,
		s_socket *stream, s_socket *bind, s_buffer *buf);

int test_request_dynamic(s_socks *s, s_socks_conf *c, s_buffer *buf);
int analyse_request(s_socks *s, s_socket *stream, s_socket *bind,
		s_socks_conf *c, s_buffer *buf);

void build_auth_ack(s_socks *s, s_socks_conf *c, s_buffer *buf);
int test_auth(s_socks *s, s_socks_conf *c, s_buffer *buf);

void build_version_ack(s_socks *s, s_socks_conf *c, s_buffer *buf);
int test_version(s_socks *s, s_socks_conf *c, s_buffer *buf);


int dispatch_server(s_client *client, fd_set *set_read, fd_set *set_write);

int dispatch_server_read(s_socket *soc, s_socket *soc_stream, s_socket *soc_bind,
		s_socks *socks, s_buffer *buf, s_buffer *buf_stream, s_socks_conf *conf);

int dispatch_server_write(s_socket *soc, s_socket *soc_stream, s_socks *socks,
		s_buffer *buf, s_socks_conf *conf);

/* TODO: Implement this function:
void dispatch_server(s_socket *soc, s_socket *soc_stream, s_socket *soc_bind,
		s_socks *socks, s_buffer *buf, s_buffer *buf_stream, s_socks_conf *conf,
		fd_set *set_read, fd_set *set_write);
*/

void init_select_server_stream (s_socket *soc, s_socks *socks, s_buffer *buf,
		s_buffer *buf_stream, int *maxfd,	fd_set *set_read, fd_set *set_write);
void init_select_server_cli (s_socket *soc,	s_socks *s, s_buffer *buf,
		s_buffer *buf_stream, int *maxfd,	fd_set *set_read, fd_set *set_write);

void init_select_server (int soc_ec, s_client *tc, int *maxfd,
		fd_set *set_read, fd_set *set_write);

int init_select_server_reverse (s_client *tc, int *maxfd,
		int ncon, fd_set *set_read, fd_set *set_write, int ssl);

#endif /* SOCKS5_SERVER__H */

