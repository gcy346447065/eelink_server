/*
 * http.h
 *
 *  Created on: 2016/12/06
 *      Author: lc
 */

#ifndef SRC_EVREQ_LIST_H_
#define SRC_EVREQ_LIST_H_

#include <evhttp.h>
#include <glib.h>

GHashTable *request_initial(void);
void request_destruct(GHashTable *request_table);
int request_add(GHashTable *request_table, struct evhttp_request *req, unsigned char seq);
int request_del(GHashTable *request_table, unsigned char seq);
struct evhttp_request *request_get(GHashTable *request_table, unsigned char seq);

#endif/*    SRC_EVREQ_LIST_H_   */

