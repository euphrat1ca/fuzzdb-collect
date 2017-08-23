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
#ifndef ERROR_H
#define ERROR_H


#define MAX_ERRMSGLEN  64

/*
** mocks specific error codes
*/
#define ERR_NONE         0
#define ERR_NEGFAIL      1
#define ERR_NEGTIME      2
#define ERR_BADVER       3
#define ERR_READ         4
#define ERR_WRITE        5
#define ERR_NOMETHOD     6
#define ERR_CMDNOTSUP    7
#define ERR_ATYPNOTSUP   8
#define ERR_RESOLV       9
#define ERR_CONREFUSE    10
#define ERR_NETUNREACH   11
#define ERR_CREATESK     12
#define ERR_BIND         13
#define ERR_LISTEN       14
#define ERR_CONIDLE	 15
#define ERR_SKFATAL 	 16
#define ERR_LASTMSG      ERR_SKFATAL



/*
** SOCKSv5 protocol error codes
*/
#define SE_NONE          0
#define SE_GENERAL       1
#define SE_NETUNREACH    3
#define SE_CONREFUSE     5
#define SE_CMDNOTSUP     7
#define SE_ATYPNOTSUP    8


extern char sz_error[ERR_LASTMSG+1][MAX_ERRMSGLEN];


int err_to_se( int err_code );       


#endif /* ERROR_H */
