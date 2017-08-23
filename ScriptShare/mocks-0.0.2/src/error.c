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
#include "error.h"

char sz_error[ERR_LASTMSG+1][MAX_ERRMSGLEN] = {
         "no error",
	 "negotiation failed",
	 "negotiation timed out",
	 "bad SOCKS protocol version",
	 "read error",
	 "write error",
	 "no acceptable method",
	 "command not supported",
	 "address type not supported",
	 "unable to resolve target name",
	 "connection to target refused",
	 "target network unreachable",
	 "socket creation failed",
	 "bind() failed",
	 "listen() failed",
	 "connection idle for too long",
	 "fatal socket error"
     };


int err_to_se( int err_code )
{
    switch( err_code ) {
      case ERR_NONE:
	return SE_NONE;
      case ERR_CMDNOTSUP:
	return SE_CMDNOTSUP;
      case ERR_ATYPNOTSUP:
	return SE_ATYPNOTSUP;
      case ERR_NETUNREACH:
	return SE_NETUNREACH;
      case ERR_CONREFUSE:
	return SE_CONREFUSE;
    }
    return SE_GENERAL;
}
