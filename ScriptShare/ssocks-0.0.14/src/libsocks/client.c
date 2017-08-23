/*
 *      client.c
 *
 *      Created on: 2011-04-11
 *      Author:     Hugo Caron
 *      Email:      <h.caron@codsec.com>
 *
 * Copyright (C) 2011 by Hugo Caron
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "client.h"

#include "bor-util.h"

void init_client (s_client *c, int id, int mode, s_socks_conf *conf)
{
	c->id = id;
	/*c->mode = mode;*/
	c->conf = conf;

	init_socket(&c->soc);
	init_socket(&c->soc_stream);
	init_socket(&c->soc_bind);

	init_socks(&c->socks, c->id, mode);
	init_socks(&c->socks_stream, c->id, (mode == M_DYNAMIC) ? M_DYNAMIC_CLIENT : mode);

	init_buffer(&c->buf);
	init_buffer(&c->stream_buf);
}

void disconnection(s_client *c)
{
	if ( c->soc_stream.soc != -1 || c->soc_bind.soc != -1 || c->soc.soc != -1 )
		TRACE(L_VERBOSE, "server [%d]: disconnected client ...", c->id);

	close_socket(&c->soc_stream);
	close_socket(&c->soc_bind);
	close_socket(&c->soc);

	init_client(c, c->id, c->socks.mode, c->conf);
}

int new_connection(int soc_ec, s_client *tc, int ssl)
{
    int nc, soc_tmp;
    struct sockaddr_in adrC_tmp;

    TRACE(L_DEBUG, "server: connection in progress ...");
    soc_tmp = bor_accept_in (soc_ec, &adrC_tmp);
    if (soc_tmp < 0) { return -1; }

    /* Search free space in tc[].soc */
    for (nc = 0; nc < MAXCLI; nc++)
        if (tc[nc].soc.soc == -1) break;

    if (nc < MAXCLI) {
    	init_client(&tc[nc], tc[nc].id, tc[nc].socks.mode, tc[nc].conf);
        tc[nc].soc.soc = soc_tmp;
        tc[nc].soc.con = 1;
        memcpy (&tc[nc].soc.adrC, &adrC_tmp, sizeof(struct sockaddr_in));
        TRACE(L_VERBOSE, "server [%d]: established connection with %s",
            nc, bor_adrtoa_in(&adrC_tmp));

#ifdef HAVE_LIBSSL
		/* Init SSL here
		 */
		if ( ssl == 1 ){
			TRACE(L_DEBUG, "server [%d]: socks5 enable ssl  ...", nc);
			tc[nc].soc.ssl = ssl_neogiciate_server(tc[nc].soc.soc);
			if ( tc[nc].soc.ssl == NULL ){
				ERROR(L_VERBOSE, "server [%d]: ssl error", nc);
				disconnection(&tc[nc]);
				return -1;
			}
			TRACE(L_DEBUG, "server [%d]: ssl ok.", nc);
			set_non_blocking(tc[nc].soc.soc);
		}
#endif /* HAVE_LIBSSL */

		return nc;
        //append_log_client(&tc[nc], "%s", bor_adrtoa_in(&adrC_tmp));
		//set_non_blocking(tc[nc].soc.soc);
    } else {
        close (soc_tmp);
        ERROR (L_NOTICE, "server: %s connection refused : too many clients!",
            bor_adrtoa_in(&adrC_tmp));
        return -1;
    }
}


