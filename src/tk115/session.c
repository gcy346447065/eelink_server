//
// Created by jk on 15-7-28.
//

#include <glib.h>

#include "session.h"
#include "log.h"
#include "object.h"

static GHashTable *session_table = NULL;

void session_freeKey(gpointer key)
{
	LOG_DEBUG("free key IMEI:%s of session_table", key);
    g_free(key);
}

void session_freeValue(gpointer value)
{
	SESSION * session = (SESSION *)value;

    LOG_DEBUG("free value IMEI:%s of session_table", ((OBJECT *)session->obj)->IMEI);

    g_free(session);
}

void session_table_initial()
{
	session_table = g_hash_table_new_full(g_str_hash, g_str_equal, session_freeKey, session_freeValue);
}

void session_table_destruct()
{
	g_hash_table_destroy(session_table);
}

int session_add(SESSION *session)
{
    const char *strIMEI = ((OBJECT *)session->obj)->IMEI;
	g_hash_table_insert(session_table, g_strdup(strIMEI), session);
    LOG_INFO("session %s added", strIMEI);
    return 0;
}

int session_del(SESSION *session)
{
	OBJECT *t_obj = (OBJECT *)session->obj;
	SESSION *t_session = session_get(t_obj->IMEI);
	if(NULL != t_session)
	{
		g_hash_table_remove(session_table, t_obj->IMEI);
	}
    return 0;
}

SESSION *session_get(const char *imei)
{
    return g_hash_table_lookup(session_table, imei);
}
