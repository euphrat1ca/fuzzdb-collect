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
#ifndef MISC_H
#define MISC_H


#define FALSE           0
#define TRUE            1
#define PROTO_TCP       6
#define SOCK_SIZE       sizeof(struct sockaddr)

#define SOCKS_VER       5
#define SOCKS_METHOD    0
#define SC_CONNECT      1
#define SC_BIND         2
#define ATYP_IP4        1
#define ATYP_DOMAIN     3
#define ATYP_IP6        4


typedef unsigned char uchar;


void get_host_ip( uchar *ip );
int  open_serv_sock( int *sk, struct sockaddr *ad );
int  readnb( int sk, void *buf, int cnt );
int  writenb( int sk, void *buf, int cnt );
void logstr( char *s, struct sockaddr *ad );
char *addr_to_ip( struct sockaddr *addr, char *szip );
void start_timer( int timeout );
void stop_timer();


#endif /* MISC_H */
