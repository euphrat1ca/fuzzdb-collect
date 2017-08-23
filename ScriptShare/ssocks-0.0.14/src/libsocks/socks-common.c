#include "socks-common.h"

int buf_empty(s_buffer *buf){
	return ( (buf->b - buf->a) == 0 );
}

int buf_size(s_buffer *buf){
	return (buf->b - buf->a);
}

int buf_free(s_buffer *buf){
	return sizeof(buf->data) - buf->b - 1;
}

/* Return -1 error
 * Return 0 need to write again
 * Reutrn 1 all data was write
 */
int write_socks(s_socket *s, s_buffer *buf){
	int k;
#ifdef HAVE_LIBSSL
	if ( s->ssl != NULL ){
		k = SSL_write(s->ssl, buf->data + buf->a, buf_size(buf));
		if (k < 0){ perror("write socks"); return -1; }
		buf->a += k;
		return buf_empty(buf);
	}
#endif
	k = write(s->soc, buf->data + buf->a, buf_size(buf));
	if (k < 0){ perror("write socks"); return -1; }
	buf->a += k;
	return buf_empty(buf);
}

/* Return -1 error
 * Return 0 need to read again
 * Reutrn 1 have read size >= minsize
 */
int read_socks(s_socket *s, s_buffer *buf, size_t minsize){
	int k;
#ifdef HAVE_LIBSSL
	if ( s->ssl != NULL ){
		k = SSL_read(s->ssl, buf->data + buf->b, buf_free(buf));
		if (k < 0){ perror("read socks"); return -1; }
		if (k == 0){ return -1; }
		buf->b += k;
		return (buf->b >= minsize);
	}
#endif
	k = read(s->soc, buf->data + buf->b, buf_free(buf));
	if (k < 0){ perror("read socks"); return -2; }
	if (k == 0){ return -1; }
	buf->b += k;
	return (buf->b >= minsize);
}

void init_buffer(s_buffer *buf){
	buf->data[0] = 0;
	buf->a = 0;
	buf->b = 0;
}

void init_socket(s_socket *s){
	s->soc = -1;
	s->con = 0;
#ifdef HAVE_LIBSSL
	s->ssl= NULL;
	s->want_ssl= -1;
#endif
}

void init_socks(s_socks *s, int id, int mode){
	s->id = id;
	s->mode = mode;	 	/* Socks mode */
	switch(s->mode){
		case M_CLIENT:
			s->state = S_W_VER;
			break;
		case M_SERVER:
			s->state = S_R_VER;
			break;
		case M_DYNAMIC:
			s->state = S_R_VER;
			break;
		case M_DYNAMIC_CLIENT:
			s->state = S_W_VER;
			break;
		default:
			s->state = -1;   	/* Socks state */
			break;
	}

	s->version = -1; 	/* Version choose, default -1 */
	s->method = -1;	 	/* Authentication method choose, default -1 */
	s->auth = 0; 	 	/* Authenticate flag, default 0 */
	s->connected = 0;	/* Connected flag, default 0 */
	s->listen = 0;		/* Listen flag in bind mode, default 0,
					 * if -1 error when accept */
	s->cmd = 0;		/* Socks command request */

	s->uname[0] = 0;
}


void close_socket(s_socket *s){
	if ( s->soc != -1 ) close(s->soc);
	s->soc = -1;

#ifdef HAVE_LIBSSL
	if ( s->ssl != NULL ) ssl_close(s->ssl);
	s->ssl= NULL;
#endif
}
