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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include <string.h>
#include <stdio.h>
/*#include <unistd.h>*/
#include <sys/types.h>
#include <netdb.h>
#include "up_proxy.h"



int proxy_errno;




/*****************************************************************
 * Routine:
 *     proxy_new( type,addr,port,user,pwd )
 * Description:
 *     Allocate memory for a new proxy information structure.
 *     The returned object is to be used in subsequent calls
 *     to upstream proxy related functions. When the caller
 *     is done with the proxy object, it must call proxy_free().
 * Parameters:
 *     type  -> Proxy type. See up_proxy.h for a list of 
 *              supported proxy types.
 *     name  -> Domain name address of proxy to connect to.
 *     port  -> TCP port to connect to.
 *     flags -> Optional flags that affect the proxy negotiation.
 *              This parameter is constructed by bitwise ORing 
 *              the PROXY_FL_* values found in up_proxy.h.
 *     user  -> Optional user name for authenticating to upstream
 *              proxy. Only valid when PROXY_FL_AUTH is specified
 *              in flags.
 *     pwd   -> Optional password for authenticating to upstram
 *              proxy. Only valid when PROXY_FL_AUTH is specified
 *              in flags.
 * Return:
 *     Pointer to a t_proxy structure on success, or NULL on
 *     failure (proxy_errno is set accordingly in the latter 
 *     case).
 *****************************************************************/
struct t_proxy *proxy_new( 
    ushort   type, 
    char     *name,
    uint16_t port,
    ushort   flags,
    char     *user, 
    char     *pwd 
)
{

    struct t_proxy *res;
    struct hostent *he;


    /* 
     * Allocate memory for the information structure.
     */
    res = (struct t_proxy*) calloc(1,sizeof(struct t_proxy));

    /*
     * Check for allocation error.
     */
    if( !res ) {
	proxy_errno = PROXY_ERR_MEM;
	return NULL;
    }

    /*
     * If atuthentication is required, 
     * duplicate user and pwd strings.
     */
    if( flags & PROXY_FL_AUTH ) {
	if( !user || !pwd ) {
	    free(res);
	    proxy_errno = PROXY_ERR_PARAM;
	    return NULL;
	}

	res->user = strdup(user);
	if( !res->user ) {
	    free(res);
	    proxy_errno = PROXY_ERR_MEM;
	    return NULL;
	}

	res->pwd = strdup(pwd);
	if( !res->pwd ) {
	    free(res->user);
	    free(res);
	    proxy_errno = PROXY_ERR_MEM;
	    return NULL;
	}
    }

    /*
     * Reslove proxy name
     */
    he = gethostbyname(name);
    if( !he ) {
	proxy_errno = PROXY_ERR_RESOLV;
	return NULL;
    }
    memcpy(&res->ip,he->h_addr_list[0],4);

    /*
     * Fill the proxy structure with data.
     */
    res->type     = type;
    res->port     = port;
    res->flags    = flags;
    
    return res;

}


void proxy_delete( struct t_proxy *proxy )
{
    if( !proxy )
	return;

    if( proxy->user )
	free(proxy->user);
    if( proxy->pwd )
	free(proxy->pwd);
    free(proxy);
}


/**********************************************************
 * Routine: 
 *    proxy_connect( proxy,target_addr,proxy_cmd )
 **********************************************************/
struct t_proxy_connection *proxy_connect( 
    struct t_proxy *proxy, 
    char           *target_name, 
    ushort         target_port,
    int            proxy_cmd
)
{
    
    struct t_proxy_connection *pcon;
    int                       poll_res;


    pcon = proxy_connect_nb( proxy, target_name, target_port, proxy_cmd );
    if( !pcon ) 
	return NULL;

    for( ;; ) {
	poll_res = proxy_poll_connect(pcon,PROXY_CONPOLLTIMEO);
	if( poll_res < 0 ) {
	    proxy_end_connect(pcon);
	    return NULL;
	}
	if( poll_res > 0 ) 
	    return pcon;
    }

}


/**********************************************************
 * Routine: 
 *    proxy_connect_nb( proxy,target_addr,proxy_cmd )
 **********************************************************/
struct t_proxy_connection *proxy_connect_nb( 
    struct t_proxy *proxy, 
    char           *target_name, 
    uint16_t       target_port,
    int            proxy_cmd
)
{

    struct t_proxy_connection *res;
    struct sockaddr_in        proxy_addr;
    int                       adam;


    /*
     * Check parameter validity.
     */
    if( !proxy || !target_name ){
	proxy_errno = PROXY_ERR_PARAM;
	return NULL;
    }

    switch( proxy_cmd ) {
      case PROXY_CMD_CONNECT:
	break;
      case PROXY_CMD_BIND:
	if( proxy->type != PROXY_SOCKS5 && proxy->type != PROXY_SOCKS4 ) {
	    proxy_errno = PROXY_ERR_PARAM;
	    return NULL;
	}
	break;
      case PROXY_CMD_UDP:
	if( proxy->type != PROXY_SOCKS5 ) {
	    proxy_errno = PROXY_ERR_PARAM;
	    return NULL;
	}
	break;
      default:
	proxy_errno = PROXY_ERR_PARAM;
	return NULL;
    }


    /* 
     * Create connection structure.
     */
    res = (struct t_proxy_connection *) 
      calloc(1,sizeof(struct t_proxy_connection));

    res->proxy       = proxy;
    res->proxy_cmd   = proxy_cmd;
    res->target_port = target_port;
    res->target_name   = strdup(target_name);

    if( !res->target_name) {
	free(res);
	proxy_errno = PROXY_ERR_MEM;
	return NULL;
    }


    /*
     * I hate this now as I always did.
     */
    memset(&proxy_addr,0,sizeof(struct sockaddr_in));
    proxy_addr.sin_family      = AF_INET;
    proxy_addr.sin_addr.s_addr = proxy->ip;
    proxy_addr.sin_port        = htons(proxy->port);
    
    
    /*
     * Create a socket to use for connecting
     * to the proxy.
     */
    res->sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if( res->sock < 0 ) {
	free(res->target_name);
	free(res);
	proxy_errno = errno;
	return NULL;
    }

    /*
     * Set the socket to non-blocking, so the 
     * caller won't block during the connection attempt.
     */
    if( fcntl(res->sock,F_SETFL,O_NONBLOCK) < 0 ) {
	close(res->sock);
	free(res->target_name);
	free(res);
	proxy_errno = errno;
	return NULL;
    }
	
    /*
     * Send connection request.
     */
    adam = connect( res->sock,
		    (struct sockaddr*)&proxy_addr,
		    sizeof(struct sockaddr_in) );
    
    /*
     * Check connection status.
     */
    if( adam  &&  errno != EINPROGRESS ) {
	close(res->sock);
	free(res->target_name);
	free(res);
	proxy_errno = errno;
	return NULL;
    }
    
    /*
     * t_proxy_connection::status will hold the negotiation
     * progress. Its value will depend upon the other
     * members of t_proxy_connection. See up_proxy.h for a
     * full list of valid sate values (PROXY_CS_*).
     */
    res->flags |= PROXY_CF_WAITWRITE;
    res->status = PROXY_CS_CONNECTING;
    
    return res;

}


/* Returns: 
 *   1  -> Negotiation successfully completed
 *   0  -> Negotiation still in progress
 *   -1 -> Negotiation failed
 */
int proxy_poll_connect( struct t_proxy_connection *pcon, int timeout )
{
    struct pollfd  pfd;
    struct hostent *he;
    int            so_err;
    int            so_err_len;
    int            adam;
    uint8_t        buf[PROXY_NEGBUFLEN];
    int            buf_len;
    uint8_t        auth_buf[PROXY_AUTHBUFLEN];

    
    /* 
     * I'll do lots of polling for pcon->sock, so
     * I'll just initalize the polldf struct here.
     */
    pfd.fd      = pcon->sock;
    pfd.revents = 0;

    /*
     * For most of the status values I will have to
     * wait for socket readability or writability.
     * A flag in the connection structure specifies
     * what I'll have to wait for next.
     */
    if( pcon->flags & PROXY_CF_WAITREAD ) {
	pfd.events = POLLIN;
	pcon->flags &= ~PROXY_CF_WAITREAD;
    }
    else
	if( pcon->flags & PROXY_CF_WAITWRITE ) {
	    pfd.events = POLLOUT;
	    pcon->flags &= ~PROXY_CF_WAITWRITE;
	}

    switch( poll(&pfd,1,timeout) ) {
      case 0: /* No event occured and timeout expired */
	return 0; 
      case -1: /* An error occured */
	proxy_errno = errno;
	pcon->status = PROXY_CS_ERROR;
	return -1;
    }



    switch( pcon->status ) {

	/*
	 * The connection to the proxy sever is not 
	 * yet completed.
	 */
      case PROXY_CS_CONNECTING:
	
	/*
	 * Check whether the connection to proxy completed
	 * as described in the connect() man page: by polling
	 * for writability and checking the value of SO_ERROR at
	 * SOL_SOCKET level afterwards.
	 */

	so_err_len = sizeof(so_err);
	adam = getsockopt(pcon->sock,SOL_SOCKET,SO_ERROR,&so_err,&so_err_len);

	if( adam<0 || so_err ) {
	    pcon->status = PROXY_CS_ERROR;
	    proxy_errno = errno;
	    return -1;
	}

	/*
	 * Set the socket back to blocking, from now on
	 * I'll just poll for readability/writability.
	 */
	if( fcntl(pcon->sock,F_SETFL,0) < 0 ) {
	    pcon->status = PROXY_CS_ERROR;
	    proxy_errno = errno;
	    return -1;
	}

	/*
	 * All ok till now, time to consider
	 * the proxy type.
	 */
	pcon->status = pcon->proxy->type * 100;

	break;


	/*
	 * The SOCKS5 server is waiting for a list of
	 * supported authentication methods from me.
	 */
      case PROXY_CS_S5_SENDMET:
	
	/*
	 * Set up the SOCKS request buffer.
	 */
	buf[0] = 5; /* SOCKS version */
	buf[2] = 0; /* SOCKS method 0 - no auth */
	
	/*
	 * If I can authenticate to this server, inform it
	 * that I support authentication.
	 */
	if( pcon->proxy->flags & PROXY_FL_AUTH ) {
	    buf[1]  = 2; /* Number of acceptable methods */
	    buf[3]  = 2; /* SOCKS method 2 - User/Passwd */
	    buf_len = 4;
	}
	else {
	    buf[1]  = 1; /* Number of acceptable methods */
	    buf_len = 3;
	}

	/*
	 * Send SOCKS request to proxy server.
	 */
	if( write(pcon->sock,buf,buf_len) < buf_len ) {
	    pcon->status = PROXY_CS_ERROR;
	    proxy_errno = errno;
	    return -1;
	}
	
	/*
	 * All OK, jump to the next status
	 */
	pcon->status = PROXY_CS_S5_WAITMET;
	pcon->flags |= PROXY_CF_WAITREAD;

	break;


	/*
	 * I am waiting for the SOCKS5 server to 
	 * select an authentication method.
	 */
      case PROXY_CS_S5_WAITMET:

	/*
	 * Read the server selected method.
	 */
	buf_len = read(pcon->sock,buf,2);
	if( buf_len < 2 ) {
	    proxy_errno = errno;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}
	
	/* 
	 * Check SOCKS version returned by server.
	 */
	if( buf[0] != 5 ) {
	    proxy_errno = PROXY_ERR_SOCKSVER;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	/*
	 * Check SOCKS5 method. For now, we support 
	 * method 0 (no auth) and method 2 (usr/pwd).
	 */
	switch( buf[1] ) {
	  case 0:
	    pcon->status = PROXY_CS_S5_SENDREQ;
	    break;
	  case 2:
	    pcon->status = PROXY_CS_S5_SENDAUTH;
	    break;
	  default:
	    proxy_errno  = PROXY_ERR_SOCKSMETHOD;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	pcon->flags |= PROXY_CF_WAITWRITE;

	break;


	/*
	 * Authenticate to the SOCKS5 server by sending
	 * username/password.
	 */
      case PROXY_CS_S5_SENDAUTH:
	
	/*
	 * Set up SOCKS5 authetication buffer.
	 */
	buf[0]        = 1; /* Authentication version 1 */
	buf[1]        = (char)strlen(pcon->proxy->user); /* Username length */
	strcpy(buf+2,pcon->proxy->user);    /* Add username to buffer */
	buf[2+buf[1]] = (char)strlen(pcon->proxy->pwd); /* Password length */
	strcpy(buf+2+buf[1]+1,pcon->proxy->pwd);  /* Add password */
	buf_len       = buf[1] + buf[2+buf[1]] + 3; /* Set buffer length */
	
	
	/* 
	 * Send authentication data to SOCKS server.
	 */
	if( write(pcon->sock,buf,buf_len) < buf_len  ) {
	    proxy_errno = errno;
	    pcon->status |= PROXY_CS_ERROR;
	    return -1;
	}
	
	pcon->status = PROXY_CS_S5_WAITAUTH;
	pcon->flags |= PROXY_CF_WAITREAD;

	break;


	/*
	 * I am waiting for an authentication response
	 * from the SOCKS5 server. 
	 */
      case PROXY_CS_S5_WAITAUTH:
	
	/*
	 * Read authentication response (2 bytes).
	 */
	buf_len = read(pcon->sock,buf,2);
	if( buf_len < 2 ) {
	    if( buf_len < 0 )
		proxy_errno = errno;
	    else
		proxy_errno = PROXY_ERR_AUTH; 
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}
	
	/*
	 * Check the authentication status received
	 * from server. This should be 0 for a
	 * successfull operation.
	 */
	if( buf[1] ) {
	    proxy_errno  = PROXY_ERR_AUTH;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	/*
	 * Authentication went OK, time to send
	 * SOCKS5 request.
	 */
	pcon->status  = PROXY_CS_S5_SENDREQ;
	pcon->flags |= PROXY_CF_WAITWRITE;

	break;

	
	/*
	 * The SOCKS5 server is waiting for the SOCKS
	 * request from me.
	 */
      case PROXY_CS_S5_SENDREQ:
	
	/*
	 * Set up SOCKS request buffer. I will specify
	 * the target address through a domain name
	 * because I might not be able to directly 
	 * access a DNS server, so the proxy should do that.
	 */
	buf[0] = 5; /* SOCKS version */
	buf[1] = pcon->proxy_cmd; /* SOCKS command */
	buf[2] = 0; /* Reserved  */
	buf[3] = 3; /* ATYP 3: domain name */
	buf[4] = strlen(pcon->target_name); /* target address length */
	strcpy(buf+5,pcon->target_name);    /* target address */
	*(uint16_t*)(buf+5+buf[4]) = htons(pcon->target_port); /*target port*/
	buf_len = buf[4]+7; /* buffer length */

	/*
	 * Send SOCKS request to server.
	 */
	if( write(pcon->sock,buf,buf_len) < buf_len ) {
	    proxy_errno  = errno;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	/*
	 * All ok, now wait for server reply.
	 */
	pcon->status  = PROXY_CS_S5_WAITREQ;
	pcon->flags |= PROXY_CF_WAITREAD;

	break;


	/*
	 * I am waiting for a reply to my SOCKS5
	 * request.
	 */
      case PROXY_CS_S5_WAITREQ:

	/*
	 * Read request reply.
	 */
	buf_len = read(pcon->sock,buf,PROXY_NEGBUFLEN);

	if( buf_len < 2 ) {
	    if( buf_len < 0 )
		proxy_errno = errno;
	    else
		proxy_errno = PROXY_ERR_NEGOT;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}
	
	
	/*
	 * Check reply status.
	 */
	if( buf[1] ) {
	    if( buf[1] > 8 )
		buf[1] = 1;
	    proxy_errno  = PROXY_ERR_S5_BASE + buf[1];
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	/*
	 * Set remote address
	 */
	switch( buf[3] ) { /* ATYP */
	  case 1: /* IPv4 */
	    pcon->remote_name = (char*)malloc(16);
	    if( !pcon->remote_name )
		break;
	    sprintf( pcon->remote_name,"%d.%d.%d.%d",
		     buf[4],buf[5],buf[6],buf[7] );
	    pcon->remote_port = ntohs(*(uint16_t*)(buf+8));
	    break;
	  case 3: /* Domain name */
	    pcon->remote_name = (char*)calloc(1,buf[4]+1);
	    if( !pcon->remote_name )
		break;
	    memcpy(pcon->remote_name,buf+5,buf[4]);
	    pcon->remote_port = ntohs(*(uint16_t*)(buf+5+buf[4]));
	    break;
	}

	/*
	 * If I'm handling a CONNECT or an UPD request then
	 * this is it, I can notify the caller the connection
	 * through the proxy succeded.
	 */
	if( pcon->proxy_cmd != PROXY_CMD_BIND ) {
	    pcon->status = PROXY_CS_CONNECTED;
	    return 1;
	}

	/*
	 * If this is a BIND command, I must wait for
	 * a second reply from the proxy server telling 
	 * me a connection arrived for my bound socket.
	 */
	pcon->flags |= PROXY_CF_WAITREAD;
	pcon->status = PROXY_CS_S5_WAITREQ2;
	    
	break;


	/*
	 * I've issued a BIND request and now I'm
	 * waiting for the second reply from the 
	 * SOCKS5 server.
	 */
      case PROXY_CS_S5_WAITREQ2:

	
	/*
	 * Read second reply (for BIND only).
	 */
	buf_len = read(pcon->sock,buf,PROXY_NEGBUFLEN);

	if( buf_len < 2 ) {
	    if( buf_len < 0 )
		proxy_errno = errno;
	    else
		proxy_errno = PROXY_ERR_NEGOT;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	/*
	 * Check reply status.
	 */
	if( buf[1] ) {
	    if( buf[1] > 8 )
		buf[1] = 1;
	    proxy_errno  = PROXY_ERR_S5_BASE + buf[1];
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}
	
	/*
	 * I'm not interested in whatever else
	 * the server sends me.
	 */
	pcon->status = PROXY_CS_CONNECTED;

	return 1;

	
	/*
	 * I must send the SOCKS4 request to 
	 * the proxy server.
	 */
      case PROXY_CS_S4_SENDREQ:
	
	/*
	 * SOCKS4 doesn't support remote resolving
	 * so I'll have to do it here.
	 */
	while( !(he = gethostbyname(pcon->target_name)) && errno==TRY_AGAIN )
	       ;
	if( !he ) {
	    proxy_errno  = errno;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}
	
	/*
	 * Set up the request buffer
	 */
	buf[0] = 4; /* SOCKS version 4 */
	buf[1] = pcon->proxy_cmd; /* SOCKS4 command */
	*(uint16_t*)(buf+2) = htons(pcon->target_port); /* target port */
	memcpy(buf+4,he->h_addr_list[0],4); /* target IPv4 address */
	sprintf(buf+8,"%u",getuid()); /* userid */
	
	buf_len = 8 + strlen(buf+8);

	/*
	 * Send SOCKS4 request to server.
	 */
	if( write(pcon->sock,buf,buf_len) < buf_len ) {
	    proxy_errno  = errno;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	pcon->flags |= PROXY_CF_WAITREAD;
	pcon->status = PROXY_CS_S4_WAITREQ;

	break;


	/*
	 * I'm waiting for a reply to my SOCKS4 request.
	 */
      case PROXY_CS_S4_WAITREQ:

	/*
	 * Read SOCKS4 reply.
	 */
	buf_len = read(pcon->sock,buf,PROXY_NEGBUFLEN);
	
	if( buf_len < 2 ) {
	    proxy_errno = errno;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	/*
	 * Check SOCKS4 reply status 
	 * (should be 90 if al went well).
	 */
	if( buf[1] != 90 ) {
	    proxy_errno = PROXY_ERR_S4_BASE + buf[1] - 90;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	/*
	 * Setup the remote address.
	 */
	pcon->remote_port = ntohs(*(uint16_t*)(buf+2));
	pcon->remote_name   = (char*)malloc(16);
	if( pcon->remote_name ) 
	    sprintf( pcon->remote_name,"%d.%d.%d.%d",
		     buf[4],buf[5],buf[6],buf[7] );
	

	/*
	 * That's all for a CONNECT command.
	 */
	if( pcon->proxy_cmd == PROXY_CMD_CONNECT ) {
	    pcon->status = PROXY_CS_CONNECTED;
	    return 1;
	}

	/* 
	 * For a BIND command, I must wait for the
	 * second reply.
	 */
	pcon->flags |= PROXY_CF_WAITREAD;
	pcon->status = PROXY_CS_S4_WAITREQ2;

	break;


	/*
	 * I'm waiting for the second reply to my 
	 * SOCKS4 BIND command (telling me a connection
	 * has arrived for my listening socket).
	 */
      case PROXY_CS_S4_WAITREQ2:

	/*
	 * Read SOCKS4 reply.
	 */
	buf_len = read(pcon->sock,buf,PROXY_NEGBUFLEN);
	
	if( buf_len < 2 ) {
	    proxy_errno = errno;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	/*
	 * Check SOCKS4 reply status 
	 * (should be 90 if al went well).
	 */
	if( buf[1] != 90 ) {
	    proxy_errno = PROXY_ERR_S4_BASE + buf[1] - 90;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	return 1;


	/*
	 * I am to send the CONNECT request to the 
	 * HTTP proxy.
	 */
      case PROXY_CS_HC_SENDREQ:
	
	sprintf( buf,"CONNECT %s:%d HTTP/1.1\n"
		 "Host: %s:%d\n",
		 pcon->target_name, pcon->target_port, 
                 pcon->target_name, pcon->target_port );
	buf_len = strlen(buf);

	/*
	 * If this is the case, add the authorization data
	 * to the request buffer in the form usr:pwd, 
	 * base-64 encoded.
	 */
	if( pcon->proxy->flags & PROXY_FL_AUTH ) {
	    sprintf(auth_buf,"%s:%s",pcon->proxy->user,pcon->proxy->pwd);
	    strcat(buf,"Authorization: Basic ");
	    proxy_encode_base64(auth_buf,strlen(auth_buf),buf+strlen(buf));
	    strcat(buf,"\n");
	    strcat(buf,"Proxy-Authorization: Basic ");
	    proxy_encode_base64(auth_buf,strlen(auth_buf),buf+strlen(buf));
	    strcat(buf,"\n");
	}
	
	/*
	 * End of request.
	 */
	strcat(buf,"\n");
	buf_len = strlen(buf);
	
	/*
	 * Send the CONNECT command to the
	 * HTTP proxy.
	 */
	/*printf(buf);*/
	if( write(pcon->sock,buf,buf_len) < buf_len ) {
	    proxy_errno = errno;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	pcon->status = PROXY_CS_HC_WAITREQ;
	pcon->flags |= PROXY_CF_WAITREAD;

	break;


	/*
	 * I am waiting for the server response
	 * to my CONNECT request.
	 */
      case PROXY_CS_HC_WAITREQ:
	
	/*
	 * Read HTTP server response.
	 */
	buf_len = read(pcon->sock,buf,buf_len);

	if( buf_len <= 0 ) {
	    proxy_errno = errno;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}

	buf[buf_len] = 0;

	/*
	 * Read HTTP response code. This should be 200
	 * or slightly above 200 if everything went OK.
	 */
	sscanf(buf,"HTTP/1.1 %d",&adam);

	/*
	 * Error.
	 */
	if( adam/100 != 2 ) {
	    proxy_errno = PROXY_ERR_HC_GENERAL;
	    pcon->status = PROXY_CS_ERROR;
	    return -1;
	}
	    
	/*
	 * Success.
	 */
	pcon->status = PROXY_CS_CONNECTED;

	return 1;

	    
	/*
	 * Negotiation completed,
	 * nothing to do.
	 */
      case PROXY_CS_CONNECTED:
	return 1;


	/*
	 * The status information in the pcon struct
	 * is corrupted.
	 */
      default:

	proxy_errno = PROXY_ERR_PARAM;

	return -1;

    }

    return 0;

}


void proxy_end_connect( struct t_proxy_connection *pcon )
{
    
    if( !pcon )
	return;

    if( pcon->remote_name )
	free(pcon->remote_name);

    if( pcon->target_name )
	free(pcon->target_name);

    close(pcon->sock);

    free(pcon);
    
}


int proxy_read(
    struct t_proxy_connection *pcon,
    void                      *buf,
    int                       buf_len
)
{
    return read(pcon->sock,buf,buf_len);
}


int proxy_write(
    struct t_proxy_connection *pcon,
    void                      *buf,
    int                        buf_len
)
{
    return write(pcon->sock,buf,buf_len);
}


char *proxy_encode_base64( char *src, int src_len, char *dest ) 
{
    
    int     i,j;
    char    b64[64];
    char    b1,b2;
    

    if( !(src && dest) )
        return NULL;

    /*
     * Create the base-64 encoding table.
     */
    for( i=0; i<26; i++ ) {
        b64[i]      = 'A'+i;
        b64[26+i]   = 'a'+i;
        if( i<10 ) {
            b64[52+i] = '0'+i;
        }
    }
    b64[62] = '+';
    b64[63] = '/';


    j = 0;

    for( i=0; i<src_len; i+=3 ) {

        /*
	 * Pad with zeros to a string length dividable by 3
	 */
        b1 = b2 = 0;
        if( i+1 < src_len ) 
            b1 = src[i+1];
        if( i+2 < src_len ) 
            b2 = src[i+2];

        dest[j++] = b64[src[i] >> 2];
        dest[j++] = b64[((src[i] & 0x03) << 4) | (b1 >> 4)];
        dest[j++] = b64[((b1 & 0x0F) << 2) | (b2>> 6)];
        dest[j++] = b64[b2 & 0x3F];

    }

    if( i >= src_len+1 )
        dest[j-1] = '=';
    if( i >= src_len+2 )
        dest[j-2] = '=';

    dest[j] = 0;


    return dest;

}

