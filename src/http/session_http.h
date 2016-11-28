/*
 * session_http.h
 *
 *  Created on: 2016/11/28
 *      Author: lc
 */

#ifndef SRC_SESSION_HTTP_H_
#define SRC_SESSION_HTTP_H_

#include <evhttp.h>

typedef struct
{
    long int key;
    struct evhttp_request *req;
}SESSION_HTTP;

SESSION_HTTP *session_new_hash(long int key);
SESSION_HTTP *session_get_hash(long int key);
void session_add_hash(SESSION_HTTP *session);
void session_del_hash(SESSION_HTTP *session);
void session_table_initial(void);
void session_table_destruct(void);

#endif /* SRC_SESSION_HTTP_H_ */

