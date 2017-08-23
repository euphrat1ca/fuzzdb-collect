/*
 *      rcsocks
 *
 *      Created on: 2011-04-13
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
#include <libsocks/socks5-server.h>

#include <config.h>
#include <getopt.h>


#define PORT 1080

struct globalArgs_t {
	char *host;				// -h option
	unsigned int port;		// -p option
	unsigned int listen;	// -l option
	unsigned int verbosity;	// -v
	unsigned int background;// -b

#ifdef HAVE_LIBSSL
	unsigned int ssl;		// -s option
	char *filecert;			// -c option
	char *filekey;			// -k option
#endif

	char *uname;			// -u option
	char *passwd;			// -p option

	char *sockshost;		// -s host:port
	int socksport;
} globalArgs;

int boucle_princ = 1;
void capte_fin (int sig){
    TRACE(L_VERBOSE, "server: signal %d caught\n", sig);
    boucle_princ = 0;
}

/*
 * TODO: Bind localhost not 0.0.0.0
 */
void usage(char *name){
	printf("rcSocks Reverse Client Socks5 v%s\n", PACKAGE_VERSION);
	printf("Usage:\n");
	printf("\t%s -p 1088 -l 1080 -b\n", name);
	printf("\t%s -p 1088 -l 1080\n", name);
	printf("In this config you run rssocks like this:\n");
	printf("rssocks --socks rcsocksserv:1080\n");
	printf("and set your tool to connect trought the socks5 server rcsocksserv:1088\n\n");
	printf("Options:\n");
	printf("\t--verbose (increase verbose level)\n\n");
	printf("\t--listen {port} where rssocks connect back\n");
	printf("\t--port {port} the socks5 port you want\n");
#ifdef HAVE_LIBSSL
	printf("\t--ssl      enable secure socks5 protocol\n");
	printf("\t--cert  {certfile.crt} Certificate of dst server (enable SSL)\n");
	printf("\t--key  {file.pem}    set server private key\n");
#endif
	printf("\t--background\n");
	printf("\n");
	printf("Bug report %s\n", PACKAGE_BUGREPORT);
}

void capte_usr1(){
	TRACE(L_DEBUG, "server: catch USR1 signal ...");
}

void new_connection_reverse (int soc_ec, s_client *tc, s_socket *socks_pool)
{
    int nc, nc2, soc_tmp;
    struct sockaddr_in adrC_tmp;

    TRACE(L_DEBUG, "server: connection in progress (reverse) ...");
    soc_tmp = bor_accept_in (soc_ec, &adrC_tmp);
    if (soc_tmp < 0) { return; }

    /* Search free space in tc[].soc */
    for (nc = 0; nc < MAXCLI; nc++)
        if (tc[nc].soc.soc == -1) break;

    /* Search for a relay in socks_pool */
    for (nc2 = 0; nc2 < MAXCLI; nc2++)
        if (socks_pool[nc2].soc != -1) break;

    if (nc < MAXCLI && nc2 < MAXCLI) {
    	init_client(&tc[nc], tc[nc].id, tc[nc].socks.mode, tc[nc].conf);
        tc[nc].soc.soc = soc_tmp;
        tc[nc].socks.state = S_REPLY;
        tc[nc].socks.connected = 1;

        memcpy(&tc[nc].soc_stream, &socks_pool[nc2], sizeof(s_socks));

        /* Remove from the pool */
        socks_pool[nc2].soc = -1;

        memcpy(&tc[nc].soc.adrC, &adrC_tmp, sizeof(struct sockaddr_in));
        TRACE(L_VERBOSE, "server [%d]: established connection with %s",
            nc, bor_adrtoa_in(&adrC_tmp));

        //append_log_client(&tc[nc], "%s", bor_adrtoa_in(&adrC_tmp));
		//set_non_blocking(tc[nc].soc);
    } else {
        close (soc_tmp);
        ERROR (L_NOTICE, "server: %s connection refused : too many clients!",
            bor_adrtoa_in(&adrC_tmp));
    }

}

void new_connection_socket(int soc_ec, s_socket *tc, int ssl)
{
    int nc, soc_tmp;
    struct sockaddr_in adrC_tmp;

    TRACE(L_DEBUG, "server: connection server in progress (socket) ...");
    soc_tmp = bor_accept_in (soc_ec, &adrC_tmp);
    if (soc_tmp < 0) { return; }

    /* Search free space in tc[].soc */
    for (nc = 0; nc < MAXCLI; nc++)
        if (tc[nc].soc == -1) break;

    if (nc < MAXCLI) {
    	init_socket(&tc[nc]);

        tc[nc].soc = soc_tmp;
        memcpy (&tc[nc].adrC, &adrC_tmp, sizeof(struct sockaddr_in));
        TRACE(L_VERBOSE, "server [%d]: established server connection with %s",
            nc, bor_adrtoa_in(&adrC_tmp));

#ifdef HAVE_LIBSSL
		/* Init SSL here
		 */
		if ( ssl == 1 ){
			TRACE(L_DEBUG, "server [%d]: socks5 enable ssl  ...", nc);
			tc[nc].ssl = ssl_neogiciate_server(tc[nc].soc);
			if ( tc[nc].ssl == NULL ){
				ERROR(L_VERBOSE, "server [%d]: ssl error", nc);
				close_socket(&tc[nc]);
				return;
			}
			TRACE(L_DEBUG, "server [%d]: ssl ok.", nc);
			set_non_blocking(tc[nc].soc);
		}
#endif /* HAVE_LIBSSL */

        //append_log_client(&tc[nc], "%s", bor_adrtoa_in(&adrC_tmp));
		//set_non_blocking(tc[nc].soc);
    } else {
        close (soc_tmp);
        ERROR (L_NOTICE, "server: %s connection refused : too many clients!",
            bor_adrtoa_in(&adrC_tmp));
    }
}

void init_select_reverse (int soc_ec, int soc_ec_cli, s_client *tc, int *maxfd,
		fd_set *set_read, fd_set *set_write)
{
    int nc;
    /* TODO: move FD_ZERO */
    FD_ZERO (set_read);
    FD_ZERO (set_write);

    FD_SET (soc_ec, set_read);
    FD_SET (soc_ec_cli, set_read);

    *maxfd = soc_ec_cli;
    for (nc = 0; nc < MAXCLI; nc++){
		s_client *client = &tc[nc];

		init_select_server_cli(&client->soc, &client->socks,
				&client->buf, &client->stream_buf,	maxfd, set_read, set_write);

		init_select_server_stream(&client->soc_stream, &client->socks,
				&client->stream_buf, &client->buf, maxfd, set_read, set_write);


		if ( client->soc_bind.soc != -1 ){
			FD_SET(client->soc_bind.soc, set_read);
			if (client->soc_bind.soc > *maxfd) *maxfd = client->soc_bind.soc;
		}
	}
}

void server_relay(int port, int listen, int ssl){
    int soc_ec_cli = -1, soc_ec = -1, maxfd, res, nc;
    fd_set set_read;
    fd_set set_write;
    struct sockaddr_in addrS;



    s_socket socks_pool[MAXCLI];
    s_client tc[MAXCLI];

    /* Init client tab */
    for (nc = 0; nc < MAXCLI; nc++)
    	init_socket(&socks_pool[nc]);

    for (nc = 0; nc < MAXCLI; nc++)
    	init_client (&tc[nc], nc, 0, NULL);

    TRACE(L_NOTICE, "server: set listening client socks relay ...");
    soc_ec = new_listen_socket (NULL, port, MAXCLI, &addrS);
    if (soc_ec < 0) goto fin_serveur;

    TRACE(L_NOTICE, "server: set server relay ...");
    soc_ec_cli = new_listen_socket (NULL, listen, MAXCLI, &addrS);
    if (soc_ec_cli < 0) goto fin_serveur;

	if ( globalArgs.background == 1 ){
		TRACE(L_NOTICE, "server: background ...");
		if ( daemon(0, 0) != 0 ){
			perror("daemon");
			exit(1);
		}
	}

    bor_signal (SIGINT, capte_fin, SA_RESTART);

    /* TODO: Find a better way to exit the select and recall the init_select
     * SIGUSR1 is send by a thread to unblock the select */
    bor_signal (SIGUSR1, capte_usr1, SA_RESTART);
    while (boucle_princ) {
    	init_select_reverse(soc_ec, soc_ec_cli, tc, &maxfd, &set_read, &set_write);

        res = select (maxfd+1, &set_read, &set_write, NULL,NULL);

        if (res > 0) {  /* Search eligible sockets */

            if (FD_ISSET (soc_ec, &set_read))
               new_connection_socket (soc_ec, socks_pool, ssl);

            if (FD_ISSET (soc_ec_cli, &set_read))
                new_connection_reverse (soc_ec_cli, tc, socks_pool);

            for (nc = 0; nc < MAXCLI; nc++){
            	dispatch_server(&tc[nc], &set_read, &set_write);
			}
        } else if ( res == 0){
            /* If timeout was set in select and expired */
        }else if (res < 0) {
            if (errno == EINTR) ;  /* Received signal, it does nothing */
            else { perror ("select"); goto fin_serveur; }
        }
    }

fin_serveur:
#ifdef HAVE_LIBSSL
	if (ssl == 1)
		ssl_cleaning();
#endif
    printf ("Server: closing sockets ...\n");
    if (soc_ec != -1) close (soc_ec);
    for (nc = 0; nc < MAXCLI; nc++) close_socket(&socks_pool[nc]);
    for (nc = 0; nc < MAXCLI; nc++) disconnection(&tc[nc]);
}


void parse_arg(int argc, char *argv[]){
	memset(&globalArgs, 0, sizeof(globalArgs));
	globalArgs.listen = 1088;
	globalArgs.port = 1080;

	int c;
	while (1){
		static struct option long_options[] = {
			{"help",    no_argument,       0, 'h'},
			{"verbose", no_argument,       0, 'v'},
			{"background", no_argument,    0, 'b'},
			{"listen",  required_argument, 0, 'l'},
			{"port",  required_argument, 0, 'p'},
#ifdef HAVE_LIBSSL
			{"ssl",  no_argument,       0, 's'},
			{"cert", required_argument, 0, 'c'},
			{"key",  required_argument, 0, 'k'},
#endif
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "h?bvk:sc:p:l:",
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
				//globalArgs.verbosity++;
				verbosity++;
				break;

#ifdef HAVE_LIBSSL
			case 's':
				globalArgs.ssl = 1;
				break;
			case 'c':
				globalArgs.filecert = strdup(optarg);
				break;
			case 'k':
				globalArgs.filekey = strdup(optarg);
				break;
#endif

			case 'b':
				globalArgs.background = 1;
				break;

			case 'p':
				/* printf("Passwd: %s\n", optarg); */
				globalArgs.port = atoi(optarg);
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

#ifdef HAVE_LIBSSL
	if (globalArgs.ssl == 1){
		SSL_load_error_strings();  /* readable error messages */
		SSL_library_init();        /* initialize library */
		if ( globalArgs.filecert[0] == 0 ){
			ERROR(L_NOTICE, "server: need a certificate file to use ssl");
			exit(1);
		}
		if ( globalArgs.filekey[0] == 0 ){
			ERROR(L_NOTICE, "server: need a private key file to use ssl");
			exit(1);
		}
		if ( ssl_init_server(globalArgs.filecert,
				globalArgs.filekey, SSL_FILETYPE_PEM) != 0){
			ERROR(L_NOTICE, "server: ssl configuration error");
			exit(1);
		}
	}
#endif
}


int main (int argc, char *argv[]){
	parse_arg(argc, argv);
	server_relay(globalArgs.port, globalArgs.listen,
#ifdef HAVE_LIBSSL
			globalArgs.ssl
#else
			0
#endif
			);
	exit(0);
}
