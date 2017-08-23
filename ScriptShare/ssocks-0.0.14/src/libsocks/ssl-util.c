/*
 *      ssl-util.c
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
#include "ssl-util.h"

#ifdef HAVE_LIBSSL

#include "output-util.h"




SSL_CTX *ctx;

int ssl_close(SSL *ssl){
	TRACE(L_DEBUG, "server-ssl: shutdown ..");
	if ( ssl == NULL) {
		/* ERROR(L_DEBUG, "server-ssl: ssl is null"); */
		return -1;
	}
	if ( SSL_shutdown(ssl) != 1 ){
		ERR_print_errors_fp(stderr);
		return -1;
	}
	SSL_free(ssl);

	return 0;
}

void ssl_cleaning(){
	TRACE(L_DEBUG, "server-ssl: cleaning ctx structure");
	SSL_CTX_free(ctx);
}

/*TODO: Write a shutdown function for ssl */
int ssl_init_client(char *serv_cert){
	const SSL_METHOD *meth;

	/* Create an SSL_METHOD structure
	 * (choose an SSL/TLS protocol version) */
	meth = SSLv3_method();
	/* Create an SSL_CTX structure */
	ctx = SSL_CTX_new(meth);
	if (ctx == NULL){
		ERR_print_errors_fp( stderr);
		return -1;
	}

	/* Load the RSA CA certificate into the SSL_CTX structure
	 * This will allow this client to verify the server's
	 * certificate. */
	if (SSL_CTX_load_verify_locations(ctx, serv_cert, NULL) != 1) {
		ERR_print_errors_fp(stderr);
		return -1;
	}

	/* Set flag in context to require peer (server) certificate
	 * verification */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	SSL_CTX_set_verify_depth(ctx, 1);

	return 0;
}

int ssl_init_server(char *certfile, char *privkeyfile, int type){
	const SSL_METHOD *meth;

	/* Create an SSL_METHOD structure
	 * (choose an SSL/TLS protocol version) */
	meth = SSLv3_method();
	/* Create an SSL_CTX structure */
	ctx = SSL_CTX_new(meth);
	if (ctx == NULL){
		ERR_print_errors_fp( stderr);
		return -1;
	}

	/* Load the server certificate into the SSL_CTX structure */
	TRACE(L_VERBOSE, "server-ssl: load the server certificate");
	if (SSL_CTX_use_certificate_file(ctx, certfile, type) <= 0) {
		ERR_print_errors_fp( stderr);
		return -1;
	}

	/* Load the private-key corresponding to the server certificate */
	TRACE(L_VERBOSE, "server-ssl: load the private-key");
	if (SSL_CTX_use_PrivateKey_file(ctx, privkeyfile, type) <= 0) {
		ERR_print_errors_fp( stderr);
		return -1;
	}

	TRACE(L_VERBOSE, "server-ssl: check if the server certificate "\
			"and private-key matches");
	/* Check if the server certificate and private-key matches */
	if (!SSL_CTX_check_private_key(ctx)) {
		ERROR(L_VERBOSE,"Private key does not match the "\
				"certificate public key");
		return -1;
	}

	return 0;
}

SSL *ssl_neogiciate_client(int soc){
	SSL *ssl;
	/* An SSL structure is created */
	ssl = SSL_new (ctx);
	if ( ssl == NULL ){
		ERR_print_errors_fp(stderr);
		return NULL;
	}

	/* Assign the socket into the SSL structure (SSL and socket without BIO) */
	SSL_set_fd(ssl, soc);

	/* Perform SSL Handshake on the SSL server */
	int r = SSL_connect(ssl);
	if ( r != 1){
		ERROR(L_VERBOSE, "client-ssl: sucks %d", SSL_get_error(ssl, r));
		ERR_print_errors_fp(stdout);
		if ( SSL_shutdown(ssl) != 1 ){
			ERR_print_errors_fp(stderr);
		}
		return NULL;
	}

	return ssl;
}

SSL *ssl_neogiciate_server(int soc){
	SSL *ssl;
	/* TCP connection is ready. A SSL structure is created */
	ssl = SSL_new(ctx);
	if ( ssl == NULL ){
		ERR_print_errors_fp(stderr);
		return NULL;
	}

	/* Assign the socket into the SSL structure (SSL and socket without BIO) */
	SSL_set_fd(ssl, soc);

	/* Perform SSL Handshake on the SSL server */
	int r = SSL_accept(ssl);
	if ( r != 1){
		ERROR(L_NOTICE, "server-ssl: sucks %d", SSL_get_error(ssl, r));
		ERR_print_errors_fp(stderr);
		if ( SSL_shutdown(ssl) != 1 ){
			ERR_print_errors_fp(stderr);
		}
		return NULL;
	}

	return ssl;
}

#endif /* HAVE_LIBSSL */
