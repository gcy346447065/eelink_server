/*
 * server_simcom.c
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>

#include "db.h"
#include "log.h"
#include "object.h"
#include "session.h"
#include "server_simcom.h"
#include "msg_proc_simcom.h"

static void send_msg(struct bufferevent* bev, const void* buf, size_t n)
{
    bufferevent_write(bev, buf, n);

    return;
}

static void read_cb(struct bufferevent *bev, void *arg)
{
    char buf[1024] = {0};
    size_t n = 0;

    LOG_DEBUG("Receive message");

    while ((n = bufferevent_read(bev, buf, sizeof(buf))) > 0)
    {
        LOG_HEX(buf, n);

        int rc = handle_simcom_msg(buf, n, arg);
        if (rc)
        {
            LOG_ERROR("handle simcom message error!");
        }
    }

    return;
}

static void write_cb(struct bufferevent* bev __attribute__((unused)), void *arg __attribute__((unused)))
{
    return;
}

static void event_cb(struct bufferevent *bev, short events, void *arg)
{
    SESSION *session = (SESSION *)arg;
    if(!session)
    {
        LOG_FATAL("session ptr null");
        return;
    }

    OBJECT *obj = (OBJECT *)session->obj;
    if(!obj)
    {
        LOG_WARN("obj ptr null");
        return;
    }

    if (events & BEV_EVENT_CONNECTED)
    {
        LOG_DEBUG("Connect okay.\n");
    }
    else if (events & BEV_EVENT_TIMEOUT)
    {
        LOG_INFO("imei(%s), simcom connection timeout!", obj->IMEI);

        //add timeout log in db
        db_add_log(obj->IMEI, "timeout");

        session_del(session);
        evutil_socket_t socket = bufferevent_getfd(bev);
        EVUTIL_CLOSESOCKET(socket);
        bufferevent_free(bev);
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

        LOG_INFO("imei(%s), closing the simcom connection", obj->IMEI);

        //add logout log in db
        db_add_log(obj->IMEI, "logout");

        session_del(session);
        evutil_socket_t socket = bufferevent_getfd(bev);
        EVUTIL_CLOSESOCKET(socket);
        bufferevent_free(bev);
    }

    return;
}

static void accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen __attribute__((unused)), void *arg __attribute__((unused)))
{
    struct sockaddr_in* p = (struct sockaddr_in *)address;
    //TODO: just handle the IPv4, no IPv6
    char addr[INET_ADDRSTRLEN] = {0};
    inet_ntop(address->sa_family, &p->sin_addr, addr, sizeof addr);

    /* We got a new connection! Set up a bufferevent for it. */
    LOG_INFO("simcom connect from %s:%d", addr, ntohs(p->sin_port));
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev)
    {
        LOG_FATAL("accept simcom connection failed!");
        return;
    }

    SESSION *session = malloc(sizeof(SESSION));
    if (!session)
    {
        LOG_FATAL("memory alloc failed");
        return;
    }
    session->base = base;
    session->bev = bev;
    session->obj = NULL;
    session->pSendMsg = send_msg;

    //TODO: set the water-mark and timeout
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, session);

    bufferevent_enable(bev, EV_READ | EV_WRITE);

    //set the timeout for the connection, when timeout close the connection
    struct timeval tm = {600, 0};

    bufferevent_set_timeouts(bev, &tm, &tm);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx __attribute__((unused)))
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    LOG_ERROR("Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

struct evconnlistener* server_simcom(struct event_base* base, int port)
{
    struct evconnlistener *listener;
    struct sockaddr_in sin;

    /* Clear the sockaddr before using it, in case there are extra
     * platform-specific fields that can mess us up. */
    memset(&sin, 0, sizeof(sin));
    
    sin.sin_family = AF_INET; /* This is an INET address */
    sin.sin_addr.s_addr = INADDR_ANY; /* Listen on 0.0.0.0 */
    sin.sin_port = htons(port); /* Listen on the given port. */

    listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
            (struct sockaddr*)&sin, sizeof(sin));
    if (!listener)
    {
        LOG_FATAL("Couldn't create listener");
        return NULL;
    }

    evconnlistener_set_error_cb(listener, accept_error_cb);
    return listener;
}
