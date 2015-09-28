/*
 * slb.c
 *
 *  Created on: 2015Äê6ÔÂ5ÈÕ
 *      Author: jk
 */

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "log.h"

#include "slb.h"


static void read_cb(struct bufferevent *bev, void *ctx)
{
	char buf[1024] = {0};
	size_t n = 0;

	LOG_DEBUG("Receive message");

    while ((n = bufferevent_read(bev, buf, sizeof(buf))) > 0)
    {
    	LOG_HEX(buf, n);
    }
}

static void write_cb(struct bufferevent* bev, void *ctx)
{
	return;
}

static void event_cb(struct bufferevent *bev, short events, void *arg)
{
	if (events & BEV_EVENT_CONNECTED)
	{
		LOG_DEBUG("Connect okay.\n");
	}
	else if (events & BEV_EVENT_TIMEOUT)
	{
		LOG_INFO("SLB connection timeout!");

		bufferevent_free(bev);
		evutil_socket_t socket = bufferevent_getfd(bev);
		EVUTIL_CLOSESOCKET(socket);
	}
	else if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		if (events & BEV_EVENT_ERROR)
		{
			 int err = bufferevent_socket_get_dns_error(bev);
			 if (err)
			 {
				 LOG_ERROR("DNS error: %s\n", evutil_gai_strerror(err));
			 }
			LOG_ERROR("BEV_EVENT_ERROR:%s", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		}
		LOG_INFO("Closing the SLB connection");

		evutil_socket_t socket = bufferevent_getfd(bev);
		EVUTIL_CLOSESOCKET(socket);

		bufferevent_free(bev);
	}
}


static void accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen, void *arg)
{
	struct sockaddr_in* p = address;
	//TODO: just handle the IPv4, no IPv6
	char addr[INET_ADDRSTRLEN] = {0};
	inet_ntop(address->sa_family, &p->sin_addr, addr, sizeof addr);

	/* We got a new connection! Set up a bufferevent for it. */
	LOG_INFO("SLB check from %s:%d", addr, ntohs(p->sin_port));
	struct event_base *base = evconnlistener_get_base(listener);
	struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev)
	{
		LOG_FATAL("accept SLB's connection failed!");
		return;
	}


	//TODO: set the water-mark and timeout
	bufferevent_setcb(bev, read_cb, write_cb, event_cb, arg);

	bufferevent_enable(bev, EV_READ|EV_WRITE);

	//set the timeout for the connection, when timeout close the connectiont
	struct timeval tm = {300, 0};

	bufferevent_set_timeouts(bev, &tm, &tm);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	LOG_ERROR("Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

struct evconnlistener* slb_start(struct event_base* base, int port)
{
    struct evconnlistener *listener;
    struct sockaddr_in sin;

    /* Clear the sockaddr before using it, in case there are extra
     * platform-specific fields that can mess us up. */
    memset(&sin, 0, sizeof(sin));
    /* This is an INET address */
    sin.sin_family = AF_INET;
    /* Listen on 0.0.0.0 */
    sin.sin_addr.s_addr = htonl(0);
    /* Listen on the given port. */
    sin.sin_port = htons(port);

    listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
            LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
            (struct sockaddr*)&sin, sizeof(sin));
    if (!listener)
    {
		LOG_FATAL("Couldn't create listener");
		return NULL;
    }
    evconnlistener_set_error_cb(listener, accept_error_cb);

    return listener;
}
