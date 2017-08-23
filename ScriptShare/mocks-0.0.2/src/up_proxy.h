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
#ifndef UP_PROXY_H
#define UP_PROXY_H

#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


/* Proxy types */
#define PROXY_SOCKS5         1 /* SOCKS protocol, version 5 */
#define PROXY_SOCKS4         2 /* SOCKS protocol, version 4 */
#define PROXY_HTTPCONNECT    3 /* HTTP CONNECT proxy */
#define PROXY_HTTP           4 /* HTTP proxy */

/* Proxy commands */
#define PROXY_CMD_CONNECT    1
#define PROXY_CMD_BIND       2
#define PROXY_CMD_UDP        3

/* Proxy flags */
#define PROXY_FL_AUTH        1

/* Proxy connection flags */
#define PROXY_CF_WAITREAD    1 /* I should wait for readability */
#define PROXY_CF_WAITWRITE   2 /* I should wait for writability */

/* Proxy connection status */
#define PROXY_CS_CONNECTING   1 /* Connecting to proxy server */
#define PROXY_CS_CONNECTED    2 /* Negotiatin succeded */
#define PROXY_CS_ERROR        3 /* Negotiation failed */
#define PROXY_CS_S5_SENDMET   (PROXY_SOCKS5*100)
#define PROXY_CS_S5_WAITMET   (PROXY_SOCKS5*100+1)
#define PROXY_CS_S5_SENDAUTH  (PROXY_SOCKS5*100+2)
#define PROXY_CS_S5_WAITAUTH  (PROXY_SOCKS5*100+3)
#define PROXY_CS_S5_SENDREQ   (PROXY_SOCKS5*100+4)
#define PROXY_CS_S5_WAITREQ   (PROXY_SOCKS5*100+5)
#define PROXY_CS_S5_WAITREQ2  (PROXY_SOCKS5*100+6)
#define PROXY_CS_S4_SENDREQ   (PROXY_SOCKS4*100)
#define PROXY_CS_S4_WAITREQ   (PROXY_SOCKS4*100+1)
#define PROXY_CS_S4_WAITREQ2  (PROXY_SOCKS4*100+2)
#define PROXY_CS_HC_SENDREQ   (PROXY_HTTPCONNECT*100)
#define PROXY_CS_HC_WAITREQ   (PROXY_HTTPCONNECT*100+1)


/* Proxy errors */
#define PROXY_ERR_MEM          1 /* Memory allocation error */
#define PROXY_ERR_PARAM        2 /* Invalid parameter to function */
#define PROXY_ERR_SOCKSVER     3 /* Bad SOCKS version received from server */
#define PROXY_ERR_SOCKSMETHOD  4 /* Bad SOCKS method received from server */
#define PROXY_ERR_AUTH         5 /* Authentication to proxy failed */
#define PROXY_ERR_NEGOT        6 /* General negotiation error */
#define PROXY_ERR_RESOLV       7 /* DNS error */
#define PROXY_ERR_S5_BASE      (PROXY_SOCKS5*100)
#define PROXY_ERR_S5_GENERAL   (PROXY_ERR_S5_BASE+1)
#define PROXY_ERR_S5_RULESET   (PROXY_SRR_S5_BASE+2)
#define PROXY_ERR_S5_NETREACH  (PROXY_ERR_S5_BASE+3)
#define PROXY_ERR_S5_HOSTREACH (PROXY_ERR_S5_BASE+4)
#define PROXY_ERR_S5_CONREF    (PROXY_ERR_S5_BASE+5)
#define PROXY_ERR_S5_TTL       (PROXY_ERR_S5_BASE+6)
#define PROXY_ERR_S5_CMD       (PROXY_ERR_S5_BASE+7)
#define PROXY_ERR_S5_ATYP      (PROXY_ERR_S5_BASE+8)
#define PROXY_ERR_S4_BASE      (PROXY_SOCKS4*100)
#define PROXY_ERR_S4_GENERAL   (PROXY_ERR_S4_BASE+1)
#define PROXY_ERR_S4_IDENTCON  (PROXY_ERR_S4_BASE+2)
#define PROXY_ERR_S4_IDENTDIF  (PROXY_ERR_S4_BASE+3)
#define PROXY_ERR_HC_GENERAL   (PROXY_HTTPCONNECT*100+1)

/* Miscellaneous */
#define PROXY_NEGBUFLEN      256 /* Negotiation buffer length */
#define PROXY_AUTHBUFLEN     128 /* usr+':'+pwd for HTTP CONNECT */
#define PROXY_CONPOLLTIMEO   10  /* milliseconds */



/* Error number used by this module */
extern int proxy_errno;


struct t_proxy {
  ushort   type;
  uint32_t ip;
  uint16_t port;
  ushort   flags;
  char     *user;
  char     *pwd;
};

struct t_proxy_connection {
  struct t_proxy *proxy;
  int            sock;
  int            status;
  ushort         proxy_cmd;
  ushort         flags;
  uint16_t       remote_port;
  char           *remote_name;
  uint16_t       target_port;
  char           *target_name;
};


/* Create a new proxy */
struct t_proxy *proxy_new( 
    ushort   type,
    char     *name,
    uint16_t port,
    ushort   flags,
    char     *user, 
    char     *pwd 
);

/* Delete an existing proxy */
void proxy_delete( 
    struct t_proxy *proxy 
);

/* Connect using a proxy and blocking until connection completes */
struct t_proxy_connection *proxy_connect( 
    struct t_proxy *proxy,
    char           *target_name,
    ushort         target_port,
    int            proxy_cmd
);

/* Nonblocking proxy connect */
struct t_proxy_connection *proxy_connect_nb( 
    struct t_proxy *proxy, 
    char           *target_name,
    ushort         target_port,   
    int            proxy_cmd
);

/* Check a nonblocking connection for completion */
int  proxy_poll_connect( 
    struct t_proxy_connection *pcon, 
    int                       timeout 
);

/* Free any resources associated with a proxy connection */
void proxy_end_connect( 
    struct t_proxy_connection *pcon 
);

/* Read data from a proxy connection */
int proxy_read(
    struct t_proxy_connection *pcon,
    void                      *buf,
    int                       buf_len
);

/* Write data to a proxy connection */
int proxy_write(
    struct t_proxy_connection *pcon,
    void                      *buf,
    int                       buf_len
);


/* Encode a character string using the Base-64 algorithm */
char *proxy_encode_base64(
    char *src, 
    int  src_len, 
    char *dest
);


#endif /* UP_PROXY_H */
