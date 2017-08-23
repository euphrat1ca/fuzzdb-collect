/*****************************************************************************
*  MOCKS, a RFC1928 compliant SOCKSv5 server                         
*  Copyright (C) 2004  Dan Horobeanu <dhoro@spymac.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include "error.h"
#include "misc.h"
#include "child.h"
#include "socksd.h"
#include "up_proxy.h"




int  socks_port        = 1080;
char sz_logfile[256]   = PROG_NAME".log";
char sz_pidfile[256]   = PROG_NAME".pid";
int  buf_size          = 64*1024;
int  backlog           = 5;
int  neg_timeo         = 5;
int  con_idle_timeo    = 300;
int  bind_timeo        = 30;
int  shutd_timeo       = 3;
int  max_con_cnt       = 50;
struct t_proxy *up_proxy = NULL;
int filter_policy      = FP_ALLOW;
int filter_except_cnt  = 0;
uint32_t filter_excepts[MAX_FILTER_EXCEPTS];
int filter_except_masks[MAX_FILTER_EXCEPTS];

char            sz_cfgfile[256] = PROG_NAME".conf";
int             con_cnt         = 0;
int             child           = 0;
int             sk_socks;
struct sockaddr ad_socks;


/*******************
 * main() function
 *******************/
int main( int argc, char **argv )
{
    int              force_load = FALSE;
    int              sksize;
    char             s[256];
    struct sigaction sa;
    int              res;
    int              i;
    int              act=-1;
    FILE             *f;
    int              allow;
    uint32_t         adam;

    
    /*
    ** Parse command line parameters
    */
    for( i=1; i<argc; i++ )
	if( cmdln_arg_in(i,"-h","--help") ) {
	    print_help(argv[0]);
	    return 1;
	}
	else 
	    if( cmdln_arg_in(i,"-v","--version") ) {
		print_ver();
		return 1;
	    }
        else 
	    if( cmdln_arg_in(i,"-c","--config") ) 
		strcpy(sz_cfgfile,argv[++i]);
        else 
	    if( cmdln_arg_in(i,"-f","--forceload") )
		force_load = TRUE;
	else
	    if( !strcmp(argv[i],"start") )
		act = ACT_START;
        else 
	    if( !strcmp(argv[i],"shutdown") )
		act = ACT_SHUTDOWN;
       	else {
	    fprintf(stderr,"Invalid argument: %s\n",argv[i]);
	    return 2;
	}


    /*
    ** If mocks was run with no action parameter, output 
    ** a short error message and exit.
    */
    if( act<0 ) {
	fprintf(stderr,"Missing action\nSee %s -h for help\n",argv[0]);
	return 1;
    }

    /*
    ** Load and parse configuration file
    */
    if( load_config() != ERR_NONE ) {
	fprintf(stderr,"Missing or bad configuration file.\n"); 
	return 2;
    }

    /*
    ** Are we starting or shutting down?
    */
    if( act==ACT_SHUTDOWN ) {
	
	printf("Shutting down "PROG_NAME" ... ");
	fflush(stdout);
	f = fopen(sz_pidfile,"r");
	if( !f ) {
	    printf("ERROR\n");
	    printf("\tPID file could not be opened ");
	    printf("(maybe "PROG_NAME" is not running?)\n");
	    return 2;
	}
	if( fscanf(f,"%u",&res)<=0 ) {
	    printf("ERROR\n");
	    printf("\tCould not read PID file\n");
	    return 2;
	}
	fclose(f);
	if( res<=0 ) {
	    printf("ERROR\n");
	    printf("\tInvalid PID\n");
	    return 2;
	}
	if( kill(-res,SIGTERM)<0 ) {
	    printf("ERROR\n");
	    printf("\tkill() failed\n");
	    return 2;
	}
	/*
	** Give mocks some time to shut down cleanly and
	** then kill it
	*/
	sleep(shutd_timeo);
	kill(-res,SIGKILL);
	printf("OK\n");
	logstr(PROG_NAME" "PROG_VERSION" killed",NULL);
	return 0;
	
    }
    else {
	printf("Starting "PROG_NAME" ... ");
	fflush(stdout);
    }

    

    /*
    ** Check to see if a PID file already exists
    */
    if( (f=fopen(sz_pidfile,"r"))!=NULL && !force_load ) {
	printf("\nAn instance of "PROG_NAME" might already be running.\n");
	printf("Use %s -f to force execution\n",argv[0]);
	fclose(f);
	return 2;
    }
          

    /*
    ** Register signal handler for SIGCHILD and SIGTERM
    */
    memset(&sa,0,sizeof(struct sigaction));
    sa.sa_handler = handle_sig;
    sigaction(SIGTERM,&sa,0);
    sigaction(SIGCHLD,&sa,0);
    sigaction(SIGALRM,&sa,0);

    /*
    ** Start listening for client connections
    */
    ad_socks.sa_family         = AF_INET;
    *(ushort*)ad_socks.sa_data = htons(socks_port);
    res = open_serv_sock(&sk_socks,&ad_socks);
    if( res != ERR_NONE ) {
	printf("ERROR\n\t%s\n",sz_error[res]);
	return res;
    }
    

    /*
    ** SOCKS daemon successfully started
    */
    printf("OK\n");
    strcpy(s,PROG_NAME" "PROG_VERSION" started");
    logstr(s,NULL);
    
    strcpy(s,"Listening on ");
    addr_to_ip(&ad_socks,s+strlen(s));
    sprintf( s+strlen(s), ":%u", 
	     ntohs(((struct sockaddr_in*)&ad_socks)->sin_port) );
    logstr(s,NULL);

    if( up_proxy ) {
	strcpy(s,"Relaying all traffic through ");
	sprintf( s+strlen(s),"%u.%u.%u.%u:%u",
		 up_proxy->ip & 0xFF,
		 (up_proxy->ip >> 8) & 0xFF,
		 (up_proxy->ip >> 16) & 0xFF,
		 up_proxy->ip >> 24,
		 up_proxy->port );
	logstr(s,NULL);
    }


    /*
    ** Now, remember we must act as a daemon, 
    ** so let's fork() and detach.
    */
    switch( fork() ) {
      case 0:
	setsid();
	break;
      case -1:
	printf("ERROR\n\tfork() failed - cannot detach daemon\n");
	exit(1);
      default:
	exit(0);
    }
    

    /*
    ** Create PID file
    */
    f = fopen(sz_pidfile,"w");
    if( !f ) 
	logstr("WARNING: PID file could not be created!\n",NULL);
    else {
	fprintf(f,"%u",getpid());
	fclose(f);
    }
    



    /*
    ** Entering the accept loop
    */
    while( 1 ) {

	/*
	** If we're already working with  the maximum number of
	** client connections, wake up every second to check
	** if any connection closed and if not, ignore any other
	** connection requests.
	*/
	if( con_cnt >= max_con_cnt ) {
	    sleep(1);
	    continue;
	}

	sksize    = SOCK_SIZE;
	sk_client = accept( sk_socks,&ad_client,&sksize );
	if( sk_client==-1 ) {
	    if( errno!=EINTR )
		logstr("Error: accept() failed",NULL);
	    continue;
	}

	/*
	 * Match the connection against our
	 * client filter.
	 */
	allow = (filter_policy == FP_ALLOW);
	for( i=0; i<filter_except_cnt; i++ ) {
	    memcpy(&adam,ad_client.sa_data+2,4);
	    adam ^= filter_excepts[i];
	    adam &= htonl(0xFFFFFFFF << (32-filter_except_masks[i]));
	    if( !adam ) {
		allow = !allow;
		break;
	    }
	}
	
	if( !allow ) {
	    logstr("Blocked by filter!",&ad_client);
	    close(sk_client);
	    continue;
	}


	switch( fork() ) {
	  case  0:
	    child = 1;
	    close(sk_socks);
	    logstr("Incoming connection",&ad_client);
	    res = handle_con();
	    close(sk_client);
	    exit(res);
	    break;
	  case -1:
	    logstr("Could not accept connection (fork() error)",&ad_client);
	    close(sk_client);
	    break;
	  default:
	    if( ++con_cnt >= max_con_cnt ) {
		printf("Maximum daemon load reached: %d active connections",
			con_cnt);
		logstr(s,NULL);
	    }
	    close(sk_client);
	    break;
	} /* end switch */

    } /* end while */

    close(sk_socks);
    return 0;
}


/*****************************************************************
 * handle_sig() : signal handler for SIGCHLD, SIGTERM and SIGALRM
 *****************************************************************/
void handle_sig( int sig )
{
    char s[256];

    switch( sig ) {
      case SIGTERM:
	if( !child ) {
	    unlink(sz_pidfile);
	    sprintf( s, PROG_NAME" "PROG_VERSION" shutting down cleanly "
		     ": killing %d connections",con_cnt );
	    logstr(s,NULL);
	}
	exit(0);
	break;
      case SIGCHLD:
	while( waitpid(-1,NULL,WNOHANG) > 0 )
	    con_cnt--;
	break;
      case SIGALRM:
	if( child ) {
	    sprintf(s,"Connection closed (%s)",sz_error[ERR_NEGTIME]);
	    logstr(s,&ad_client);
	    exit(ERR_NEGTIME);
	}
	exit(0);
	break;
    }
}


/**********************************************
 * print_help() : output a short help message
 **********************************************/
void print_help( char *exe_path )
{
    printf("\n");
    print_ver();
    printf("USAGE:\n\t%s [OPTIONS] ACTION\n",exe_path);
    printf("OPTIONS\n");
    printf("\t-h,      --help          display this help screen and exit\n");
    printf("\t-v,      --version       display program version and exit\n");
    printf("\t-c file, --config file   use configuration file 'file'\n");
    printf("\t-f,      --forceload     force loading SOCKS daemon even if\n");
    printf("\t                         PID file exists\n");
    printf("ACTIONS\n");
    printf("\tstart                    start SOCKS daemon\n");
    printf("\tshutdown                 shutdown SOCKS daemon\n");
    printf("\n");
}


/******************************************
 * print_ver() : output program version
 ******************************************/
void print_ver()
{
    printf(PROG_NAME" version "PROG_VERSION" ("PROG_DATE")\n");
}


/******************************************************
 * load_config() : load daemon configuration from file
 ******************************************************/
int load_config()
{
    FILE           *f;
    char           buf[256];
    char           var[32]="";
    char           val[256]="";
    int            i; 
    int            err;
    struct hostent *he;
    char           *p;
    int            proxy_type = 0;
    char           proxy_name[256]="";
    int            proxy_port;
    char           proxy_usr[256]="";
    char           proxy_pwd[256]="";
    char var_name[CFG_VARS_CNT][32] = {
	"PORT", "LOG_FILE", "PID_FILE", 
	"BUFFER_SIZE", "BACKLOG", "NEGOTIATION_TIMEOUT", 
	"CONNECTION_IDLE_TIMEOUT", "BIND_TIMEOUT", "SHUTDOWN_TIMEOUT",
	"MAX_CONNECTIONS", "FILTER_POLICY", "FILTER_EXCEPTION",
	"UP_PROXY_TYPE", "UP_PROXY_ADDR","UP_PROXY_PORT",
	"UP_PROXY_USER", "UP_PROXY_PASSWD", "MOCKS_ADDR"
    };
    void *var_ptr[CFG_VARS_CNT] = {
	&socks_port, sz_logfile, sz_pidfile,
	&buf_size, &backlog, &neg_timeo,
	&con_idle_timeo, &bind_timeo, &shutd_timeo,
	&max_con_cnt, NULL, NULL,
	NULL, proxy_name, &proxy_port,
	proxy_usr, proxy_pwd, ad_socks.sa_data+2
    };
    char var_type[CFG_VARS_CNT] = {
	'n','s','s',
	'n','n','n',
	'n','n','n',
	'n','s','s',
	's','s','n',
	's','s','a'
    };


    //up_proxy = proxy_new(PROXY_SOCKS5,0x0101A8C0,1080,0,NULL,NULL);

    if( (f=fopen(sz_cfgfile,"r"))==NULL )
	return ERR_READ;


    do {

	if( !fgets(buf,256,f) )
	    break;

	for( i=0; buf[i] && buf[i]!='#'; i++ )
	    ;
	buf[i] = 0;
	
	sscanf(buf,"%32s = %s",var,val);

	if( (var[0] && !val[0]) || (!var[0] && val[0]) )
	    return ERR_READ;
	if( !var[0] && !val[0] )
	    continue;


	if( !strcmp(var,"FILTER_POLICY") ) {
	    if( !strcmp(val,"DENY") )
		filter_policy = FP_DENY;
	}
	else 
	    if( !strcmp(var,"FILTER_EXCEPTION") ) {
		p = strstr(val,"/");
		if( p ) {
		    sscanf(p+1,"%u",filter_except_masks+filter_except_cnt);
		    if( filter_except_masks[filter_except_cnt] > 32 )
			return ERR_READ;
		    *p = 0;
		}
		else
		    filter_except_masks[filter_except_cnt] = 32;
		he = gethostbyname(val);
		if( !he )
		    return ERR_READ;
		memcpy(filter_excepts+filter_except_cnt,he->h_addr_list[0],4);
		filter_except_cnt++;
	    }
        else 
	    if( !strcmp(var,"UP_PROXY_TYPE") ) {
		if( !strcmp(val,"SOCKS5") )
		    proxy_type = PROXY_SOCKS5;
		else
		    if( !strcmp(val,"SOCKS4") )
			proxy_type = PROXY_SOCKS4;
                else
		    if( !strcmp(val,"HTTPCONNECT") )
			proxy_type = PROXY_HTTPCONNECT;
		else
		    return ERR_READ;
	    }
	else {
	    err = 1;

	    for( i=0; i<CFG_VARS_CNT; i++ ) 

		if( !strcmp(var,var_name[i]) ) {
		    switch( var_type[i] ) {
		      case 's': 
			strcpy((char*)var_ptr[i],val);
			break;
		      case 'n':
			*(int*)var_ptr[i] = atoi(val);
			break;
		      case 'a':
			he = gethostbyname(val);
			if( !he ) 
			    return ERR_READ;
			memcpy(var_ptr[i],he->h_addr_list[0],4);
			break;
		    }
		    err = 0;
		}
	    
	    if( err )
		return ERR_READ;
	}

    } while( 1 );


    if( proxy_type ) {
	if( proxy_usr[0] ) 
	    up_proxy = proxy_new( proxy_type, proxy_name, proxy_port,
		PROXY_FL_AUTH, proxy_usr, proxy_pwd );
	else
	    up_proxy = proxy_new(proxy_type,proxy_name,proxy_port,0,NULL,NULL);
    }


    fclose(f);
    return ERR_NONE;
}
