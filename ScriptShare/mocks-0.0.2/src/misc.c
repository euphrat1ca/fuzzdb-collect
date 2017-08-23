/*****************************************************************************
*  MOCKS, a RFC1928 compliant SOCKSv5 server                         
*  Copyright (C) 2003  Dan Horobeanu <dhoro@spymac.com>
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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include "error.h"
#include "socksd.h"
#include "misc.h"


/***************************************************************
 * get_host_ip() : get IPv4 address of the host running socksd. 
 *                 The 'ip' parameter should point to a 4-byte 
 *                 buffer to hold the IPv4 address.   
 ***************************************************************/
void get_host_ip( uchar *ip )
{
    char           name[256];
    struct hostent *he;

    gethostname(name,255);
    he = gethostbyname(name);
    memcpy(ip,he->h_addr_list[0],4);
}


/*******************************************************************
 * open_serv_sock() : create a socket, bind it to address 'ad' and 
 *                    start listening for client connections. Upon
 *                    function completion, if no error occured,
 *                    'sk' holds the socket descriptor.
 *                    Returns: error code (ERR_NONE if successful).
 *******************************************************************/
int open_serv_sock( int *sk, struct sockaddr *ad )
{
    *sk = socket(PF_INET,SOCK_STREAM,PROTO_TCP);
    if( *sk < 0 )
	return ERR_CREATESK;

    if( bind(*sk,ad,SOCK_SIZE) < 0 ) {
	close(*sk);
	return ERR_BIND;
    }
    
    if( listen(*sk,backlog) < 0 ) {
	close(*sk);
	return ERR_LISTEN;
    }

    return ERR_NONE;
}


/*****************************************************************
 * readnb() : read n bytes from a socket. If this call does
 *            not return an error then the buffer (buf) is
 *            guaranteed to be filled with the whole amount
 *            of data asked for (cnt bytes).
 *            Returns: the error code (ERR_NONE if successful).
 *****************************************************************/
int readnb( int sk, void *buf, int cnt )
{
    int  res;
    int  br;

    br = 0;
    do {
	while( (res=read(sk,(uchar*)buf+br,cnt-br))<0 && errno==EINTR);
	if( res<=0 )
	    return ERR_READ;
	br += res;
    } while( br<cnt );

    return ERR_NONE;
}


/*****************************************************************
 * writenb() : write n bytes to a socket. If this call does
 *             not return an error then the whole buffer
 *             (cnt bytes long) is guaranteed to have been 
 *             written to the destination socket.
 *             Returns: the error code (ERR_NONE if successful).
 *****************************************************************/
int writenb( int sk, void *buf, int cnt )
{
    int  res;
    int  bw;


    bw = 0;
    do {
	while( (res=write(sk,(uchar*)buf+bw,cnt-bw))<0 && errno==EINTR);
	if( res<=0 )
	    return ERR_WRITE;
	bw += res;
    } while( bw<cnt );

    return ERR_NONE;
}


/**********************************************************************
 * logstr() : add a string to socksd log file. The 'ad' parameter
 *            identifies an address that the log entry referes to.
 *            If there is no such address, this parameter should
 *            be NULL.
 **********************************************************************/
void logstr( char *s, struct sockaddr *ad  )
{
    struct tm     *t;
    time_t        adam;
    char          szip[20]="";
    FILE          *f_log;

    f_log = fopen(sz_logfile,"a");
    if( !f_log )
      return;

    adam = time(NULL);
    t    = localtime(&adam);

    if( ad ) {
        addr_to_ip(ad,szip);
	strcat(szip," - ");
    }

    fprintf( f_log,"[%02d/%02d/%02d][%02d:%02d:%02d] %s%s\n",
	     t->tm_mday, t->tm_mon+1, t->tm_year-100,
	     t->tm_hour, t->tm_min, t->tm_sec,
	     szip, s );

    fclose(f_log);
}


/*****************************************************************
 * addr_to_ip() : extract the IP addres from a sockaddr structure
 *                in human readable dotted format.
 *****************************************************************/
char *addr_to_ip( struct sockaddr *addr, char *szip )
{
    sprintf( szip,"%u.%u.%u.%u",
	     (uchar)addr->sa_data[2], (uchar)addr->sa_data[3],
             (uchar)addr->sa_data[4], (uchar)addr->sa_data[5] );
    return szip;
}


/************************************************************
 * start_timer() : starts a timer which decrements in real
 *                 time context and expires in 'timeout'
 *                 seconds. 
 ************************************************************/
void start_timer( int timeout )
{
    struct itimerval itv;

    memset(&itv,0,sizeof(struct itimerval));
    itv.it_value.tv_sec = timeout;
    setitimer(ITIMER_REAL,&itv,NULL);
}


/*************************************************************
 * stop_timer() : stops the timer currently decrementing in
 *                real time context.
 *************************************************************/
void stop_timer()
{
    struct itimerval itv;

    memset(&itv,0,sizeof(struct itimerval));
    setitimer(ITIMER_REAL,&itv,NULL);
}
