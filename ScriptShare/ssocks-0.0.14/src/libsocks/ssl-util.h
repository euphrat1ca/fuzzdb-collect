/*
 *      ssl-util.h
 *
 *      Created on: 2011-04-07
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

#ifndef SSL_UTIL__H
#define SSL_UTIL__H

#include <config.h>

#ifdef HAVE_LIBSSL

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

extern SSL_CTX *gCtx;

int ssl_close(SSL *ssl);
void ssl_cleaning();

SSL *ssl_neogiciate_server(int soc);
SSL *ssl_neogiciate_client(int soc);
int ssl_init_server(char *certfile, char *privkeyfile, int type);
int ssl_init_client(char *serv_ca_cert);

#endif /* HAVE_LIBSSL */

#endif /* SSL_UTIL__H */
