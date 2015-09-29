//
// Created by jk on 15-7-28.
//

#ifndef ELECTROMBILE_SESSION_H
#define ELECTROMBILE_SESSION_H

#include <stddef.h>
#include <event2/bufferevent.h>

typedef void (*MSG_SEND)(struct bufferevent* bev, const void* buf, size_t n);

typedef struct
{
    struct event_base* base;
    struct bufferevent* bev;

    void* obj;
    MSG_SEND pSendMsg;
}SESSION;

void session_table_initial();
void session_table_destruct();

int session_add(SESSION *);
int session_del(SESSION *);
SESSION* session_get(const char* imei);

#endif //ELECTROMBILE_SESSION_H
