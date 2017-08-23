/*
 *      nsocks.c
 *
 * 		Netcat like who pass through a socks5
 *
 *      Created on: 2011-04-12
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

#include <libsocks/bor-util.h>
#include <libsocks/net-util.h>

#include <libsocks/output-util.h>
#include <libsocks/socks-common.h>
#include <libsocks/socks5-client.h>

#include <config.h>
#include <getopt.h>


struct globalArgs_t {
	char *host;				// -h option
	unsigned int port;		// -p option
	unsigned int listen;	// -l option
	unsigned int verbosity;	// -v

#ifdef HAVE_LIBSSL
	unsigned int ssl;		// -k option
	char *certfile;			// -c option
#endif

	char *uname;			// -u option
	char *passwd;			// -p option

	char *sockshost;		// -s host:port
	int socksport;

} globalArgs;

int boucle_princ = 1;
void capte_fin (int sig){
    TRACE(L_VERBOSE, "client: signal %d caught\n", sig);
    boucle_princ = 0;
}

void netcat_like(s_socket *s){

	/* Catch CTRL-C */
    bor_signal (SIGINT, capte_fin, SA_RESTART);

	int maxfd=0, res;
	fd_set set_read, set_write;
	char buf[4096];
	int buf_a = 0, buf_b = 0, k = 0;

	while (boucle_princ){
		FD_ZERO (&set_read);
		FD_ZERO (&set_write);

		FD_SET (0, &set_read);
		FD_SET (s->soc, &set_read);
		if (s->soc > maxfd) maxfd = s->soc; /* Fix maxfd */

		if ( buf_b - buf_a > 0 ){
			FD_SET (s->soc, &set_write);
		}

		res = select (maxfd+1, &set_read, &set_write, NULL, NULL);
        if (res > 0) {  /* Search eligible sockets */

			/* Read on stdin ? */
			if (FD_ISSET (0, &set_read)){
				k = read(0, buf+buf_b, sizeof(buf)-buf_b-1);
				if ( k < 0 ) { perror("read stdin"); close(s->soc); exit(1); }
				if ( k == 0 ) { ERROR(L_DEBUG, "client: read 0 bytes on stdin"); boucle_princ = 0; }
				//printf("client: read %d bytes in stdin\n", k);
				buf_b += k;
			}

			/* Read on socket ? */
			if (FD_ISSET (s->soc, &set_read)){
#ifdef HAVE_LIBSSL
				if ( s->ssl != NULL ){
					k = SSL_read(s->ssl, buf+buf_b, sizeof(buf)-buf_b-1);
					if (k < 0){ perror("read socket"); close(s->soc); exit(1); }
					if (k == 0){ ERROR(L_DEBUG, "client: read 0 bytes!"); boucle_princ = 0; }
					k = write(1, buf, k);
					continue;
				}
#endif
				k = read(s->soc, buf+buf_b, sizeof(buf)-buf_b-1);
				if ( k < 0 ) { perror("read socket"); close(s->soc); exit(1); }
				if ( k == 0 ) { ERROR(L_DEBUG, "client: read 0 bytes!"); boucle_princ = 0; }
				//printf("client: read %d bytes in socket\n", k);
				k = write(1, buf, k);
			}

			/* Write on socket ? */
			if(FD_ISSET (s->soc, &set_write)){
#ifdef HAVE_LIBSSL
				if ( s->ssl != NULL ){
					k = SSL_write(s->ssl, buf+buf_a, buf_b - buf_a);
					if (k < 0){perror("write socket"); boucle_princ = 0; }
					buf_a += k;
					if ( buf_b - buf_a == 0 ){
						buf_b = 0;
						buf_a = 0;
					}
					continue;
				}
#endif
				k = write(s->soc, buf+buf_a, buf_b - buf_a);
				if ( k < 0 ) { perror("write socket"); boucle_princ = 0; }
				//printf("client: wrote %d bytes on socket\n", k);
				buf_a += k;
				if ( buf_b - buf_a == 0 ){
					buf_b = 0;
					buf_a = 0;
				}
			}

        } else if ( res == 0){
            /* Timeout */

        }else if (res < 0) {
            if (errno == EINTR) ; /* Received signal, it does nothing */
            else { perror ("select"); boucle_princ = 0; }
        }
	}
}


void netcat_socks(char *sockshost, int socksport,
				char *host, int port, int listen,
				char *uname, char *passwd,
				int ssl){
	s_socket s;


	int r = new_socket_with_socks(&s, sockshost, socksport,
		uname, passwd, host, port, listen,
		SOCKS5_V, ssl,
		(listen != 0) ? CMD_BIND : CMD_CONNECT);

	if ( r < 1 ){
		ERROR(L_NOTICE, "client: connection error");
		exit(1);
	}

	TRACE(L_VERBOSE, "client: established connection");
	netcat_like(&s);
	TRACE(L_VERBOSE, "client: close socket ...");

#ifdef HAVE_LIBSSL
	if(s.ssl != NULL){
		ssl_close(s.ssl);
	}
	if(globalArgs.ssl == 1){
		ssl_cleaning();
	}
#endif /* HAVE_LIBSSL */
	close(s.soc);


}


void usage(char *name){
	printf("nsocks v%s ( Netcat like with Socks5 support )\n", PACKAGE_VERSION);
	printf("Actually close on EOF (CTRL-D)\n");
	printf("Usage:\n");
	printf("\t%s --socks localhost:1080 mywebserv.com 80\n", name);
	printf("\t%s -s localhost:1080 -u y0ug -p 1234 mywebserv.com 80\n", name);
	printf("\t%s -s localhost:1080 -l 8080\n", name);
	printf("Options:\n");
#ifdef HAVE_LIBSSL
	printf("\t--cert  {certfile.crt} Certificate of dst server (enable SSL)\n");
#endif
	printf("\t--verbose (increase verbose level)\n\n");
	printf("\t--socks {host:port}\n");
	printf("\t--uname {uname}\n");
	printf("\t--passwd {passwd}\n");
	printf("\t--listen {port}\n");
	printf("\n");
	printf("Bug report %s\n", PACKAGE_BUGREPORT);
}

void parseArg(int argc, char *argv[]){
	memset(&globalArgs, 0, sizeof(globalArgs));

	int c;
	while (1){
		static struct option long_options[] = {
			{"help",    no_argument,       0, 'h'},
			{"verbose", no_argument,       0, 'v'},
#ifdef HAVE_LIBSSL
			{"cert",      required_argument, 0, 'c'},
#endif
			{"socks",   required_argument, 0, 's'},
			{"uname",   required_argument, 0, 'u'},
			{"passwd",  required_argument, 0, 'p'},
			{"listen",  required_argument, 0, 'l'},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "h?vc:s:u:p:l:",
					long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		char *port;

		switch (c)	{
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				printf ("option %s", long_options[option_index].name);
				if (optarg)
					printf (" with arg %s", optarg);
				printf ("\n");
				break;

			case 'v':
				//globalArgs.verbosity++;
				verbosity++;
				break;

#ifdef HAVE_LIBSSL
			case 'c':
				globalArgs.ssl = 1;
				globalArgs.certfile = optarg;
				break;
			case 'k':
				globalArgs.ssl = 1;
				break;
#endif

			case 's':

				port = strchr(optarg, ':');
				if ( port == NULL ){
					usage(argv[0]);
					exit(1);
				}
				*port = 0; port++;
				globalArgs.sockshost = optarg;
				globalArgs.socksport = atoi(port);
				/*printf("Connect trought socks %s:%d\n",
					globalArgs.sockshost, globalArgs.socksport);*/
				break;

			case 'u':
				/* printf("Username: %s\n", optarg); */
				globalArgs.uname = optarg;
				break;

			case 'p':
				/* printf("Passwd: %s\n", optarg); */
				globalArgs.passwd = optarg;
				break;

			case 'l':
				/* printf("Listening on port: %d\n", atoi(optarg)); */
				globalArgs.listen = atoi(optarg);
				break;

			case '?':
				/* getopt_long already printed an error message. */
				usage(argv[0]);
				exit(1);
				break;

			case 'h':
				usage(argv[0]);
				exit(1);
				break;

			default:
				abort ();
		}
	}

	if (argc - optind == 2 ){
		globalArgs.host = argv[optind++];
		globalArgs.port = atoi(argv[optind++]);
	}else if(globalArgs.listen != 0){

	}else{
		usage(argv[0]);
		exit(1);
	}

	if ( globalArgs.sockshost == NULL || globalArgs.socksport == 0 ){
		usage(argv[0]);
		exit(1);
	}

#ifdef HAVE_LIBSSL
	/*Initialize ssl with the CA certificate file
	 */
	if (globalArgs.certfile != NULL){
		SSL_load_error_strings();  /* readable error messages */
		SSL_library_init();        /* initialize library */
		TRACE(L_VERBOSE, "client: init ssl ...");
		if (globalArgs.certfile == NULL){
			ERROR(L_NOTICE, "client: actually need CA certificate file");
			exit(1);
		}
		if ( ssl_init_client(globalArgs.certfile) != 0){
			ERROR(L_NOTICE, "client: ssl config error");
			exit(1);
		}
		TRACE(L_VERBOSE, "client: ssl ok.");
	}
#endif
}

int main (int argc, char *argv[]){
	parseArg(argc, argv);

	/*if ( globalArgs.listen != 0 )
		netcat_socks_bind(globalArgs.sockshost, globalArgs.socksport,
					"0.0.0.0", globalArgs.listen,
					globalArgs.uname, globalArgs.passwd,
#ifdef HAVE_LIBSSL
					globalArgs.ssl);
#else
					0);
#endif
	else*/
		netcat_socks(globalArgs.sockshost, globalArgs.socksport,
					globalArgs.host, globalArgs.port, globalArgs.listen,
					globalArgs.uname, globalArgs.passwd,
#ifdef HAVE_LIBSSL
					globalArgs.ssl);
#else
					0);
#endif

	exit(0);
}
