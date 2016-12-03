/*
 * session_http.h
 *
 *  Created on: 2016/11/28
 *      Author: lc
 */
#include <glib.h>
#include <string.h>

#include "log.h"
#include "http_connect_simcom.h"

static GHashTable *session_table = NULL;

HTTP_CONNECTION *session_new_hash(long int key)
{
	HTTP_CONNECTION *session = g_malloc(sizeof(HTTP_CONNECTION));
	memset(session, 0, sizeof(HTTP_CONNECTION));
    session->key = key;

	return session;
}

HTTP_CONNECTION *session_get_hash(long int key)
{
    return g_hash_table_lookup(session_table, key);
}

void session_add_hash(HTTP_CONNECTION *session)
{
	g_hash_table_insert(session_table, session->key, session);
    LOG_INFO("session added to hashtable: %d", session->key);
}

void session_del_hash(HTTP_CONNECTION *session)
{
    HTTP_CONNECTION * obj = session_get_hash(session->key);
    if(NULL != obj)
    {
        g_hash_table_remove(session_table, obj->key);
    }
}

static void session_freeValue(gpointer value)
{
    HTTP_CONNECTION *session = (HTTP_CONNECTION *)value;

    LOG_DEBUG("free value of session_table :%d", session->key);

    g_free(session);
}

void session_table_initial(void)
{
    session_table = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, session_freeValue);
}

void session_table_destruct(void)
{
    g_hash_table_destroy(session_table);
}

