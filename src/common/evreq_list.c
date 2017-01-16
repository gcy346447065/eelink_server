/*
 * evreq_list.c
 *
 *  Created on: 2016/12/06
 *      Author: lc
 */
#include <string.h>
#include <malloc.h>

#include "evreq_list.h"
#include "msg_http.h"
#include "log.h"

static void request_freeValue(gpointer value)
{
    struct evhttp_request *req = (struct evhttp_request *)value;
}

GHashTable *request_initial(void)
{
    return g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, request_freeValue);
}

void request_destruct(GHashTable *request_table)
{
    g_hash_table_destroy(request_table);
}

int request_add(GHashTable *request_table, struct evhttp_request *req, unsigned char seq)
{
    g_hash_table_insert(request_table, (gconstpointer)seq, req);
    return 0;
}

int request_del(GHashTable *request_table, unsigned char seq)
{
    g_hash_table_remove(request_table, (gconstpointer)seq);
    return 0;
}

struct evhttp_request *request_get(GHashTable *request_table, unsigned char seq)
{
    return g_hash_table_lookup(request_table, (gconstpointer)seq);
}

