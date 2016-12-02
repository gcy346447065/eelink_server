/*
 * session_http.h
 *
 *  Created on: 2016/11/28
 *      Author: lc
 */
#include <glib.h>
#include <string.h>

#include "log.h"
#include "session_http.h"

static GHashTable *session_table = NULL;

SESSION_HTTP *session_new_hash(long int key)
{
	SESSION_HTTP *session = g_malloc(sizeof(SESSION_HTTP));
	memset(session, 0, sizeof(SESSION_HTTP));
    session->key = key;

	return session;
}

SESSION_HTTP *session_get_hash(long int key)
{
    return g_hash_table_lookup(session_table, key);
}

void session_add_hash(SESSION_HTTP *session)
{
	g_hash_table_insert(session_table, session->key, session);
    LOG_INFO("session added to hashtable: %d", session->key);
}

void session_del_hash(SESSION_HTTP *session)
{
    SESSION_HTTP * obj = session_get_hash(session->key);
    if(NULL != obj)
    {
        g_hash_table_remove(session_table, obj->key);
    }
}

static void session_freeValue(gpointer value)
{
    SESSION_HTTP *session = (SESSION_HTTP *)value;

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

