/*
 * cell.c
 *
 *  Created on: Oct 1, 2015
 *      Author: jk
 */


#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>

static struct bufferevent *bev = NULL;

static void eventcb(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED)
    {
         /* We're connected to the server.   Ordinarily we'd do
            something here, like start reading or writing. */
    	//TODO:
    }
    else if (events & BEV_EVENT_ERROR)
    {
    	//TODO
    }
}

static void read_cb(struct bufferevent *bev, void *ctx)
{
	char buf[1024] = {0};
	size_t n = 0;

    while ((n = bufferevent_read(bev, buf, sizeof(buf))) > 0)
    {
    	//TODO
    }
}

static void write_cb(struct bufferevent* bev, void *ctx)
{
	//TODO
	return;
}


int main_loop(struct event_base *base)
{
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
    sin.sin_port = htons(40713); /* Port 8080 */

    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, read_cb, write_cb, eventcb, NULL);
    bufferevent_enable(bev, EV_READ|EV_WRITE);


    if (bufferevent_socket_connect(bev,
        (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        /* Error starting connection */
        bufferevent_free(bev);
        return -1;
    }

    return 0;
}

void sendData(const void* data, size_t len)
{
	bufferevent_write(bev, data, len);
}



