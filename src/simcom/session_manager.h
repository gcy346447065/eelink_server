/*
 * session_manager.h
 *
 *  Created on: Mar 17, 2016
 *      Author: gcy
 */

#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

typedef void (*MSG_SEND)(struct bufferevent* bev, const void* buf, size_t n);

typedef struct
{
    struct event_base *base;
    struct bufferevent *bev;

    MSG_SEND pSendMsg;
    int sequence;
}SESSION_MANAGER;

void sessionManager_table_initial(void);
void sessionManager_table_destruct(void);
int sessionManager_add(SESSION_MANAGER *sessionManager);
int sessionManager_del(SESSION_MANAGER *sessionManager);
SESSION_MANAGER *sessionManager_get(int seq);

#endif //SESSION_MANAGER_H

