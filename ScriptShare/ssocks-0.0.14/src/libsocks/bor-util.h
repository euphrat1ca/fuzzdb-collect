/* bor-util.h : Boite a Outil Reseau
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 22/01/2009 - V 2.0
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#ifndef BOR_UTIL__H
#define BOR_UTIL__H

#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     /* exit */
#include <unistd.h>     /* close */
#include <string.h>     /* strlen */
#include <sys/types.h>  /* open, socket, bind, sendto, recvfrom, wait */
#include <sys/wait.h>   /* wait */
#include <sys/stat.h>   /* open */
#include <fcntl.h>      /* open */
#include <sys/socket.h> /* socket, bind, sendto, recvfrom, getsockname */
#include <sys/un.h>     /* socket domaine AF_UNIX */
#include <netinet/ip.h> /* socket domaine AF_INET */
#include <arpa/inet.h>  /* inet_ntoa */
#include <netdb.h>      /* gethostbyname */
#include <signal.h>     /* sigaction */
#include <sys/time.h>   /* gettimeofday */
#include <time.h>       /* time, gettimeofday */
#include <errno.h>      /* errno */


/* Compatibilite avec C++ */
#if defined(c_plusplus) && !defined(__cplusplus)
#define __cplusplus
#endif
#ifdef __cplusplus
extern "C" {
#endif


/* UNIX_PATH_MAX pas toujours definie */
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

/* Prototypes */
void bor_perror (const char *funcname);
int bor_signal (int sig, void (*h)(int), int options);

int bor_bind_un (int soc, struct sockaddr_un *adr);
int bor_sendto_un (int soc, void *buf, size_t len, struct sockaddr_un *adr);
int bor_recvfrom_un (int soc, void *buf, size_t len, struct sockaddr_un *adr);
int bor_connect_un (int soc, struct sockaddr_un *adr);
int bor_accept_un (int soc, struct sockaddr_un *adr);

int bor_bind_in (int soc, struct sockaddr_in *adr);
int bor_sendto_in (int soc, void *buf, size_t len, struct sockaddr_in *adr);
int bor_recvfrom_in (int soc, void *buf, size_t len, struct sockaddr_in *adr);
int bor_connect_in (int soc, struct sockaddr_in *adr);
int bor_accept_in (int soc, struct sockaddr_in *adr);
int bor_getsockname_in (int soc, struct sockaddr_in *adr);
char *bor_adrtoa_in (struct sockaddr_in *adr);


/* Solution de repli pour __func__ */
#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

/* Compatibilite avec C++ */
#ifdef __cplusplus
}
#endif

#endif /* BOR_UTIL__H */

