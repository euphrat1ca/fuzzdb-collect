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
#ifndef SOCKSD_H
#define SOCKSD_H

#include "up_proxy.h"


#define cmdln_arg_in(i,s1,s2) (!strcmp(argv[i],s1) || !strcmp(argv[i],s2))
#define ACT_START             1
#define ACT_SHUTDOWN          2
#define PROG_VERSION          "0.0.2"
#define PROG_NAME             "mocks"
#define PROG_DATE             "25 Nov 2004"

#define CFG_VARS_CNT          18
#define MAX_FILTER_EXCEPTS    256

#define FP_ALLOW              1
#define FP_DENY               2


extern int  socks_port;
extern char sz_logfile[256];
extern char sz_pidfile[256];
extern int  buf_size;
extern int  backlog;
extern int  neg_timeo;
extern int  con_idle_timeo;
extern int  bind_timeo;
extern int  shutd_timeo;
extern int  max_con_cnt;
extern struct t_proxy *up_proxy;
extern int  filter_policy;
extern int  filter_except_cnt;
extern uint32_t filter_excepts[MAX_FILTER_EXCEPTS];
extern int  filter_except_masks[MAX_FILTER_EXCEPTS];

extern char            sz_cfgfile[256];
extern int             con_cnt;
extern int  	       child;
extern int             sk_socks;
extern struct sockaddr ad_socks;


void handle_sig( int sig );
void print_help( char *exe_path );
void print_ver();
int  load_config();


#endif /* SOCKSD_H */
