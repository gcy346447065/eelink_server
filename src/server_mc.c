#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "log.h"
#include "fsm.h"
#include "curl.h"
#include "server_mc.h"

#include "msg_sch_mc.h"

#ifdef WITH_CATEGORY
#define LOG_DEBUG(...) \
	zlog(cat[MOD_SERVER_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_INFO(...) \
	zlog(cat[MOD_SERVER_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define LOG_WARNNING(...) \
	zlog(cat[MOD_SERVER_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_WARNNING, __VA_ARGS__)

#define LOG_ERROR(...) \
	zlog(cat[MOD_SERVER_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_FATAL(...) \
	zlog(cat[MOD_SERVER_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)
#endif

static void send_msg(struct bufferevent* bev, const void* buf, size_t n)
{
	LOG_INFO("Send msg to TK115 %p(len=%zu)", buf, n);
	bufferevent_write(bev, buf, n);
}


static void read_cb(struct bufferevent *bev, void *ctx)
{
	char buf[1024] = {0};
	size_t n = 0;

	LOG_DEBUG("Receive message");

    while ((n = bufferevent_read(bev, buf, sizeof(buf))) > 0)
    {
    	LOG_HEX(buf, n);
    	if (handle_mc_msg(buf, n, ctx))
    	{
    		LOG_ERROR("handle incoming message error!");
    	}
    }
}

static void write_cb(struct bufferevent* bev, void *ctx)
{
	return;
}

static void event_cb(struct bufferevent *bev, short events, void *arg)
{
	CB_CTX* ctx = arg;
	if (events & BEV_EVENT_CONNECTED)
	{
		LOG_DEBUG("Connect okay.\n");
	}
	else if (events & BEV_EVENT_TIMEOUT)
	{
//		LOG_INFO("%s connection timeout!", get_IMEI_STRING(ctx->obj));
		LOG_INFO("connection timeout!");
		cleanupLeancloudCurlHandle(ctx->curlOfLeancloud);
		cleanupYeelinkCurlHandle(ctx->curlOfYeelink);
		free(ctx);

		bufferevent_free(bev);
	}
	else if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		if (events & BEV_EVENT_ERROR)
		{
//			 int err = bufferevent_socket_get_dns_error(bev);
//			 if (err)
//			 {
//				 LOG_ERROR("DNS error: %s\n", evutil_gai_strerror(err));
//			 }
			LOG_ERROR("BEV_EVENT_ERROR:%s", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		}
		LOG_ERROR("Closing the connection");
		//TODO: cleanup the mosquitto
		cleanupLeancloudCurlHandle(ctx->curlOfLeancloud);
		cleanupYeelinkCurlHandle(ctx->curlOfYeelink);
		free(ctx);

		bufferevent_free(bev);
	}
}


static void accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen, void *arg)
{
	CB_CTX* ctx = arg;

	struct sockaddr_in* p = address;
	//TODO: just handle the IPv4, no IPv6
	char addr[INET_ADDRSTRLEN] = {0};
	inet_ntop(address->sa_family, &p->sin_addr, addr, sizeof addr);

	/* We got a new connection! Set up a bufferevent for it. */
	LOG_INFO("TK115 connect from %s:%d", addr, ntohs(p->sin_port));
	struct event_base *base = evconnlistener_get_base(listener);
	struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev)
	{
		LOG_FATAL("accept TK115's connection failed!");
		return;
	}

	CB_CTX* cb_ctx = malloc(sizeof(CB_CTX));
	cb_ctx->base = base;
	cb_ctx->bev = bev;
	cb_ctx->curlOfLeancloud = initCurlHandleOfLeancloud();
	cb_ctx->curlOfYeelink = initCurlHandleOfYeelink();
	cb_ctx->obj = 0;
	cb_ctx->pSendMsg = send_msg;

	//TODO: set the water-mark and timeout
	bufferevent_setcb(bev, read_cb, write_cb, event_cb, cb_ctx);

	bufferevent_enable(bev, EV_READ|EV_WRITE);

	//set the timeout
	struct timeval tm = {600, 0};

	bufferevent_set_timeouts(bev, &tm, &tm);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

struct evconnlistener* server_mc_start(struct event_base* base)
{
    struct evconnlistener *listener;
    struct sockaddr_in sin;
    int port = 9876;

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

