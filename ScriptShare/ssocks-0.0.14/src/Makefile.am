AM_CFLAGS=-Wall
AM_LDFLAGS = -lsocks -Llibsocks/

SUBDIRS = libsocks

bin_PROGRAMS = ssocksd nsocks ssocks  rcsocks rssocks

rcsocks_SOURCES = rcsocks.c

rssocks_SOURCES = rssocks.c

ssocks_SOURCES = ssocks.c

nsocks_SOURCES = nsocks.c

ssocksd_SOURCES =  auth-util.c configd-util.c ssocksd.c \
auth-util.h  configd-util.h \
ssocksd.conf ssocksd.auth


sysconf_DATA = ssocksd.conf ssocksd.auth
