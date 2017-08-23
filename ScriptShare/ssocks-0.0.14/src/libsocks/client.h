/*
 *      client.h
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
#ifndef CLIENT__H
#define CLIENT__H

#include "socks-common.h"

#define MAXCLI 512
#define BUFFER_SIZE 4096

typedef struct{
	int id;

	s_socket soc;
	s_socks socks;
	s_buffer buf;

	s_socket soc_stream;
	s_socks socks_stream;
	s_buffer stream_buf;

	s_socket soc_bind;
	s_socks_conf *conf;
} s_client;


void init_client (s_client *c, int id, int mode, s_socks_conf *conf);
void disconnection(s_client *c);
int new_connection(int soc_ec, s_client *tc, int ssl);



#endif /* CLIENT__H */
