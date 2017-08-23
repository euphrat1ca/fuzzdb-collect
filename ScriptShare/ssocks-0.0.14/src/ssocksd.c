/*
 *      ssocksd.c
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

#include <libsocks/net-util.h>
#include <libsocks/bor-util.h>
#include <libsocks/output-util.h>
#include <libsocks/client.h>

#include <libsocks/socks-common.h>
#include <libsocks/socks5-server.h>
#include <libsocks/log-util.h>

#include "auth-util.h"

#include "configd-util.h"
#include "auth-util.h"

#include <getopt.h>
#include <unistd.h>
#include <config.h>

#define DEFAULT_PORT 1080
#define PID_FILE "/var/run/ssocksd.pid"

int boucle_princ = 1;
void capte_fin (int sig){
    printf ("serveur: signal %d caught\n", sig);
    boucle_princ = 0;
}

/* TODO: Add --pid-file option to server
 */
void usage(char *name){
	printf("ssockd - Server Socks5 v%s\n", PACKAGE_VERSION);

	printf("Usage:\n");
	printf("\t%s --port 8080\n", name);
	printf("\t%s -b 127.0.0.1 --port 8080\n", name);
	printf("\t%s --address 127.0.0.1 --port 8080\n", name);
	printf("\t%s -p 8080 -a ssocksd.auth -d\n", name);
	printf("\t%s -vv\n", name);
	printf("\n");
	printf("Options:\n");
#ifdef HAVE_LIBSSL
	printf("\t--ssl      enable secure socks5 protocol\n");
	printf("\t--cert {file.crt}   set server certificate\n");
	printf("\t--key  {file.pem}    set server private key\n");
#endif
	printf("\t--daemon   daemon mode (background)\n");
	printf("\t--verbose  increase verbose level\n\n");
	printf("\t--port {port}  listening port (default 1080)\n");
	printf("\t--bind {ip} listening on ip (default all)\n");
	printf("\t--file {file}  see man 5 ssocksd.conf\n");
	printf("\t--auth {file}  see man 5 ssocksd.auth\n");
	printf("\t--log {file}   if set connections are log in this file\n");
	printf("\n");
	printf("Bug report %s\n", PACKAGE_BUGREPORT);
}

void parseArg(int argc, char *argv[]){
	int c;

	globalArgsServer.fileauth[0] = 0;
	globalArgsServer.filelog[0] = 0;
	globalArgsServer.fileconfig[0] = 0;
	globalArgsServer.bindAddr[0] = 0;
	globalArgsServer.port = DEFAULT_PORT;
	globalArgsServer.verbosity = 0;
	globalArgsServer.guest = 1;
	globalArgsServer.ssl = 0;

#ifdef HAVE_LIBSSL
	globalArgsServer.filecert[0] = 0;
	globalArgsServer.filekey[0] = 0;
#endif


	while (1){
		static struct option long_options[] = {
			{"help",    no_argument,       0, 'h'},
			{"verbose", no_argument,       0, 'v'},
			{"daemon",  no_argument,       0, 'd'},
#ifdef HAVE_LIBSSL
			{"ssl",  no_argument,       0, 's'},
			{"cert", required_argument, 0, 'c'},
			{"key",  required_argument, 0, 'k'},
#endif
			{"guest",no_argument,       0, 'g'},
			{"bind", required_argument, 0, 'b'},
			{"port", required_argument, 0, 'p'},
			{"file", required_argument, 0, 'f'},
			{"auth", required_argument, 0, 'a'},
			{"log",  required_argument, 0, 'l'},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "h?vsgdk:c:f:a:b:p:l:",
					long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

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
				globalArgsServer.verbosity++;
				verbosity++;
				break;

			case 'd':
				globalArgsServer.daemon = 1;
				break;

#ifdef HAVE_LIBSSL
			case 's':
				globalArgsServer.ssl = 1;
				break;
			case 'c':
				strncpy_s(globalArgsServer.filecert, optarg, 
					sizeof(globalArgsServer.filecert));
				break;
			case 'k':
				strncpy_s(globalArgsServer.filekey, optarg, 
					sizeof(globalArgsServer.filekey));
				break;
#endif
			case 'g':
				globalArgsServer.guest = 1;
				break;

			case 'p':
				globalArgsServer.port = atoi(optarg);
				break;

			case 'a':
				strncpy_s(globalArgsServer.fileauth, optarg, 
					sizeof(globalArgsServer.fileauth));
				break;
			case 'b':
				strncpy_s(globalArgsServer.bindAddr, optarg, 
					sizeof(globalArgsServer.bindAddr));
				// globalArgsServer.bindAddr = optarg;
				break;

			case 'l':
				strncpy_s(globalArgsServer.filelog, optarg, 
					sizeof(*globalArgsServer.filelog));
				break;

			case 'f':
				strncpy_s(globalArgsServer.fileconfig, optarg, 
					sizeof(globalArgsServer.fileconfig));
				if ( loadConfigFile(optarg, &globalArgsServer) < 0 ){
					ERROR(L_NOTICE, "config: config file error");
					ERROR(L_NOTICE, "server: can't start configuration error");
					exit(1);
				}
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


	if ( globalArgsServer.filelog[0] != 0 ){
		open_log(globalArgsServer.filelog);
	}
	if ( globalArgsServer.fileauth[0] != 0 ){
		globalArgsServer.guest = 0;
		if ( (c = load_auth_file(globalArgsServer.fileauth)) <= 0 ){
			ERROR(L_NOTICE, "auth: no username load");
			ERROR(L_NOTICE, "server: can't start configuration error");
			exit(1);
		}else{
			TRACE(L_NOTICE, "auth: %d usernames load", c);
		}
	}else{
		TRACE(L_NOTICE, "warning: no authentication enable");
	}

#ifdef HAVE_LIBSSL
	if (globalArgsServer.ssl == 1){
		SSL_load_error_strings();  /* readable error messages */
		SSL_library_init();        /* initialize library */
		if ( globalArgsServer.filecert[0] == 0 ){
			ERROR(L_NOTICE, "server: need a certificate file to use ssl");
			exit(1);
		}
		if ( globalArgsServer.filekey[0] == 0 ){
			ERROR(L_NOTICE, "server: need a private key file to use ssl");
			exit(1);
		}
		if ( ssl_init_server(globalArgsServer.filecert,
				globalArgsServer.filekey, SSL_FILETYPE_PEM) != 0){
			ERROR(L_NOTICE, "server: ssl configuration error");
			exit(1);
		}
	}
#endif

	verbosity = globalArgsServer.verbosity;
}

void capte_usr1(){
	TRACE(L_DEBUG, "server: catch USR1 signal ...");
}
void capte_sigpipe(){
	TRACE(L_DEBUG, "server: catch SIGPIPE signal ...");
}

void server(const char *bindAddr, int port, int ssl){
    int soc_ec = -1, maxfd, res, nc;
    s_client tc[MAXCLI];
    fd_set set_read;
    fd_set set_write;
    struct sockaddr_in addrS;
    char methods[2];

    s_socks_conf conf;
    s_socks_server_config config;
    conf.config.srv = &config;

	char versions[] = { SOCKS5_V,
		SOCKS4_V };

    config.allowed_version = versions;
    config.n_allowed_version = sizeof(versions);

    if ( globalArgsServer.fileauth[0] != 0 ){
    	methods[0] = 0x02;
    	--config.n_allowed_version; /* Disable socks4 don't support auth */
    }else{
    	methods[0] = 0x00;
    }


    config.allowed_method = methods;
    config.n_allowed_method = 1;
    config.check_auth = check_auth;

    /* Init client tab */
    for (nc = 0; nc < MAXCLI; nc++)
		init_client (&tc[nc], nc, M_SERVER, &conf);

	if(bindAddr[0] == 0)
	    soc_ec = new_listen_socket (NULL, port, 0, &addrS);
	else
	    soc_ec = new_listen_socket (bindAddr, port, 0, &addrS);

    if (soc_ec < 0) goto fin_serveur;


	if ( globalArgsServer.daemon == 1 ){
		TRACE(L_NOTICE, "server: mode daemon ...");
		if ( daemon(0, 0) != 0 ){
			perror("daemon");
			exit(1);
		}
		writePID(PID_FILE);
	}

    bor_signal (SIGINT, capte_fin, SA_RESTART);

    /* Need in daemon to remove the PID file properly */
    bor_signal (SIGTERM, capte_fin, SA_RESTART);
    bor_signal (SIGPIPE, capte_sigpipe, SA_RESTART);
    /* TODO: Find a better way to exit the select and recall the init_select
     * SIGUSR1 is send by a thread to unblock the select */
    bor_signal (SIGUSR1, capte_usr1, SA_RESTART);

    while (boucle_princ) {
        init_select_server (soc_ec, tc, &maxfd, &set_read, &set_write);

        res = select (maxfd+1, &set_read, &set_write, NULL, NULL);

        if (res > 0) { /* Search eligible sockets */

            if (FD_ISSET (soc_ec, &set_read))
                new_connection (soc_ec, tc, ssl);

            for (nc = 0; nc < MAXCLI; nc++){
				dispatch_server(&tc[nc], &set_read, &set_write);
			}

        } else if ( res == 0){

        }else if (res < 0) {
            if (errno == EINTR) ; /* Received signal, it does nothing */
            else { perror ("select"); goto fin_serveur; }
        }
    }

fin_serveur:
#ifdef HAVE_LIBSSL
	if (globalArgsServer.ssl == 1)
		ssl_cleaning();
#endif
    TRACE(L_NOTICE, "server: closing sockets ...");
    close_log();
    for (nc = 0; nc < MAXCLI; nc++) disconnection(&tc[nc]);
    if (soc_ec != -1) close (soc_ec);
    if ( globalArgsServer.daemon == 1 )	removePID(PID_FILE);
}

int main (int argc, char *argv[]){
	parseArg(argc, argv);
	server(globalArgsServer.bindAddr, globalArgsServer.port, globalArgsServer.ssl);
    exit (0);
}
