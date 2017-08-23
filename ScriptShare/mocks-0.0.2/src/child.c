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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "error.h"
#include "misc.h"
#include "socksd.h"
#include "child.h"
#include "up_proxy.h"



int             sk_client;
struct sockaddr ad_client;
int             sk_target;
struct sockaddr ad_target;
char            target_ip[MAXIPSTRLEN];
uint16_t        target_port;



/**********************************************************************
 * handle_con() : this takes control whenever a client connection
 *                to the SOCKS server is accepted.
 *                Returns: error code (ERR_NONE if all went well).
 **********************************************************************/
int handle_con()
{
    char             buf[256];
    int              res;
    struct sockaddr  ad_cl_local;


    /*
    ** Start autodestruct timer and begin 
    ** SOCKS connection negotiation
    */
    start_timer(neg_timeo);
    res = negotiate(&ad_cl_local);

    /*
    ** Setup SOCKS reply
    */
    buf[0] = SOCKS_VER;
    buf[1] = err_to_se(res);
    buf[2] = 0;
    buf[3] = ATYP_IP4;

    /*
    ** Check for errors during negotiation
    */
    if( buf[1] != SE_NONE ) {
	writenb(sk_client,buf,4);
	sprintf(buf,"Connection closed (%s)",sz_error[res]);
	logstr(buf,&ad_client);
	return res;
    }
    
    /*
    ** No errors encountered so far. Let the client
    ** know that the negotiation succeeded.
    */
    memcpy(buf+4,ad_cl_local.sa_data+2,4);
    memcpy(buf+8,ad_cl_local.sa_data,2);
    res = writenb(sk_client,buf,10);
    if( res != ERR_NONE )
	return res;

    /*
    ** Negotiation was successful, cancel autodestruct
    */
    stop_timer();

    /*
    ** Log this new valid client connection
    */
    strcpy(buf,"Established connection with target ");
    addr_to_ip(&ad_target,buf+strlen(buf));
    logstr(buf,&ad_client);
    
    /*
    ** Handle the actual data traffic between the client
    ** and its target.
    */
    res = handle_traffic();
    sprintf(buf,"Connection closed (%s)",sz_error[res]);
    logstr(buf,&ad_client);
    
    close(sk_target);
    
    return ERR_NONE;
}


/*********************************************************************
 * negotiate() : negotiates the SOCKSv5 client connection and
 *               returns 0 (ERR_NONE) if no error occured or
 *               the error code otherwise. If negotiate() does
 *               not fail, the global variables 'sk_target' and
 *               'ad_target' are guaranteed to be holding valid
 *               information.
 *               Upon successful completion, '*ad_cl_local'
 *               holds the address information required for
 *               SOCKS reply.
 *********************************************************************/
int negotiate( struct sockaddr *ad_cl_local )
{
    int            i;
    uchar          buf[256];
    int            res;
    int            cmd;
    int            sksize;
    struct hostent *he;


    /*
    ** Read the client-proposed methods
    */
    if( (res=readnb(sk_client,buf,2)) != ERR_NONE )
        return res;
    if( buf[0]!=SOCKS_VER )
	return ERR_BADVER;
    if( (res=readnb(sk_client,buf+2,buf[1])) != ERR_NONE )
	return res;
    for( i=0; i<buf[1]; i++ )
	if( buf[2+i]==SOCKS_METHOD )
	    break;
    if( i>=buf[1] )
       	return ERR_NOMETHOD;

    /*
    ** Select method 0x00 (no auth required)
    */
    buf[0] = SOCKS_VER;
    buf[1] = SOCKS_METHOD;
    if( (res=writenb(sk_client,buf,2)) != ERR_NONE )
	return ERR_WRITE;

    /*
    ** Read SOCKS request header (4 bytes: ver,cmd,rsv,atyp)
    */
    if( (res=readnb(sk_client,buf,4)) != ERR_NONE )
	return res;
    cmd = buf[1];

    /*
    ** Currently supporting CONNECT and BIND commands
    */
    if( cmd!=SC_CONNECT && cmd!=SC_BIND )
	return ERR_CMDNOTSUP;

    /*
    ** Fill out target address struc
    */
    memset(&ad_target,0,SOCK_SIZE);
    ad_target.sa_family = AF_INET;
    switch( buf[3] ) {
      case ATYP_IP4:
	/*
	** Read IPv4 address
	*/
	if( (res=readnb(sk_client,ad_target.sa_data+2,4)) != ERR_NONE )
	    return res;
	sprintf( target_ip,"%u.%u.%u.%u",
		 (uchar)ad_target.sa_data[2],
		 (uchar)ad_target.sa_data[3],
		 (uchar)ad_target.sa_data[4],
		 (uchar)ad_target.sa_data[5] );
	break;
      case ATYP_DOMAIN:
	/*
	** Read DOMAINNAME address
	*/
	if( (res=readnb(sk_client,buf,1)) != ERR_NONE )
	    return res;
	if( buf[0] > MAXIPSTRLEN-1 ) {
	    return ERR_NEGFAIL;
	}
	if( (res=readnb(sk_client,buf+1,buf[0])) != ERR_NONE )
	    return res;
	buf[buf[0]+1] = 0;
	/*
	** Attempt to resolve domain name only
	** if we have no upstream proxy to resolve it
	** for us.
	*/
	logstr(buf+1,&ad_client);
	if( !up_proxy ) {
	    he = gethostbyname(buf+1);
	    if( !he || he->h_length!=4 ) 
		return ERR_RESOLV;
	    memcpy(ad_target.sa_data+2,he->h_addr_list[0],4);
	}
	else 
	    strcpy(target_ip,buf+1);

	break;
      default:
        /*
	** Currently supporting: IPv4 and DOMAINNAME address types
	*/
	return ERR_ATYPNOTSUP;
    }

    /*
    ** Read target port 
    */ 
    if( (res=readnb(sk_client,ad_target.sa_data,2)) != ERR_NONE )
	return res;
    memcpy(&target_port,ad_target.sa_data,2);
    target_port = ntohs(target_port);


    /*
    ** Perform SOCKS command
    */
    switch( cmd ) {
      case SC_CONNECT:
	res = connect_client();
	break;
      case SC_BIND:
	memset(ad_target.sa_data+2,0,4);
	res = bind_client();
	break;
    }
    if( res != ERR_NONE )
	return res;

    /*
    ** Set client local address for SOCKS reply
    */
    sksize = SOCK_SIZE;
    getsockname(sk_target,ad_cl_local,&sksize);


    return ERR_NONE;
}


/*********************************************************************
 * connect_client() : perform CONNECT command. This function expects
 *                    global 'ad_target' or, when using an upstream
 *                    proxy, 'target_ip' and 'target_port' to hold 
 *                    a valid address. If this call is successful, 
 *                    global variable 'sk_target' is guaranteed to be 
 *                    holding valid data.
 *                    Returns: error code (ERR_NONE for success).
 *********************************************************************/
int connect_client()
{
    int                       adam;
    struct t_proxy_connection *pcon;


    if( up_proxy ) {
	pcon = proxy_connect(up_proxy,target_ip,target_port,PROXY_CMD_CONNECT);
	if( !pcon ) 
	    return ERR_NEGFAIL;
	sk_target = pcon->sock;
	free(pcon->target_name);
	free(pcon->remote_name);
	free(pcon);
	return ERR_NONE;
    }

    sk_target = socket( PF_INET,SOCK_STREAM,PROTO_TCP );
    if( sk_target < 0 )
	return ERR_NEGFAIL;

    if( connect(sk_target,&ad_target,SOCK_SIZE) < 0 ) {
	adam = errno;
	close(sk_target);
	switch( adam ) {
	  case ECONNREFUSED:
	    return ERR_CONREFUSE;
	  case ENETUNREACH:
	    return ERR_NETUNREACH;
	  default:
	    return ERR_NEGFAIL;
	}
    }
    
    return ERR_NONE;
}


/*******************************************************************
 * bind_client() : perform BIND command. This function expects 
 *                 global variable  'ad_target' to hold valid data. 
 *                 Upon a successful return, 'sk_target' holds the
 *                 the expected target client connection on the 
 *                 SOCKS client's listening socket.
 *                 Returns: error code (ERR_NONE on success).
 *******************************************************************/
int bind_client()
{
    int             res;
    int             sksize;
    uchar           buf[16];
    

    /*
    ** Open a listening socket for the client
    */
    res = open_serv_sock(&sk_target,&ad_target);
    if( res != ERR_NONE )
	return res;

    /*
    ** Setup first BIND reply buffer
    */
    buf[0] = SOCKS_VER;
    buf[1] = SE_NONE;
    buf[2] = 0x00;
    buf[3] = ATYP_IP4;
    get_host_ip(buf+4);
    buf[8] = ad_target.sa_data[0];
    buf[9] = ad_target.sa_data[1];

    /*
    ** Send first reply to BIND command
    */
    writenb(sk_client,buf,10);

    /*
    ** We must now wait for an incoming connection to the
    ** newly created socket. This might take a while,
    ** therefore we must restart the autodestruct timer 
    ** with another (greater) value.
    */
    start_timer(bind_timeo);
    
    /*
    ** Wait for an incoming connection on the 
    ** listening socket allocated for the client
    */
    sksize = SOCK_SIZE;
    while( (res=accept(sk_target,&ad_target,&sksize))<0 && errno==EINTR );
    
    /*
    ** No more blocking calls until the negotiation ends, so
    ** we can safely turn of the autodestruct timer
    */
    stop_timer();

    /*
    ** We don't need the listening socket anymore
    */
    close(sk_target);
    if( res<0 )
	return res;

    /*
    ** The connecting socket is our real target
    */
    sk_target = res;

    return ERR_NONE;
}


/**********************************************************************
 * handle_traffic() : handle the actual data trafic between the SOCKS
 *                    client and its target. This function expects
 *                    globals 'sk_client', 'ad_client', 'sk_target'
 *                    and 'ad_target' to hold valid data.
 *                    Returns: error code (ERR_NONE if successful).
 **********************************************************************/
int handle_traffic()
{
    uchar         *buf;
    int           btop = 0;
    int           bptr = 0;
    int           bown = -1;
    struct pollfd pfd[2];
    int           res;
    
    
    /*
    ** Allocate memory for buffer
    */
    buf = (uchar*)malloc(buf_size);

    memset(pfd,0,2*sizeof(struct pollfd));
    pfd[0].fd = sk_client;
    pfd[1].fd = sk_target; 

	
    while( 1 ) {
	/*
	** Setup event masks: if the buffer is not empty, we're
	** interested in writing to the other socket than the
	** buffer owner. If the buffer is empty, we're interested
	** in reading from either one of the sockets.
	*/
	if( bptr < btop ) {
	    pfd[bown].events  = 0;
	    pfd[!bown].events = POLLOUT;
	}
	else {
	    pfd[0].events = POLLIN;
	    pfd[1].events = POLLIN;
	}

	/*
	** Wait for events for maximum 'con_idle_time' seconds
	*/
	while( (res=poll(pfd,2,con_idle_timeo*1000))<0 && errno==EINTR );
    
	/*
	** If poll() or any connection failed or the timeout 
	** expired, break and return an error.
	*/
	if( res<0 || (pfd[0].revents&POLLERR) || (pfd[1].revents&POLLERR) ) {
	    res = ERR_SKFATAL;
	    break;
	}
	if( !res ) {
	    res = ERR_CONIDLE;
	    break;
	}

	/*
	** If either one of the connections were closed, break with
	** no error.
	*/
	if( (pfd[0].revents & POLLHUP) || (pfd[1].revents & POLLHUP) ) {
	    res = ERR_NONE;
	    break;
	}

    
	/*
	** If the buffer is not empty, poll() has returned telling us 
	** that writting now will not block. Else, there is data to
	** be read from one of the sockets.
	*/
	if( bptr < btop ) {
	    while( (res=write(pfd[!bown].fd,buf+bptr,btop-bptr)) < 0 
		   && errno==EINTR );
	    if( res<0 ) {
		res = ERR_SKFATAL;
		break;
	    }
	    bptr += res;
	}
	else {
	    btop = 0;
	    bptr = 0;
	    bown = (pfd[0].revents & POLLIN)==0; 
	    while( (res=read(pfd[bown].fd,buf,buf_size))<0 && errno==EINTR );
	    if( res<0 ) {
		res = ERR_SKFATAL;
		break;
	    }
	    if( !res ) {
		res = ERR_NONE;
		break;
	    }
	    btop = res;
	}
    }

    /*
    ** Free buffer
    */
    free(buf);

    return res;
}
