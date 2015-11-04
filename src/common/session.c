//
// Created by jk on 15-7-28.
//

#include <glib.h>

#include "session.h"
#include "log.h"
#include "object.h"

static GHashTable *session_table = NULL;

/*
void session_freeKey(gpointer key)
{
    LOG_DEBUG("free key IMEI:%s of session_table", (char *)key);
    g_free(key);
}
*/

void session_freeValue(gpointer value)
{
    SESSION * session = (SESSION *)value;

    LOG_DEBUG("free value IMEI:%s of session_table", ((OBJECT *)session->obj)->IMEI);

    g_free(session);
}

void session_table_initial()
{
    session_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, session_freeValue);
}

void session_table_destruct()
{
    g_hash_table_destroy(session_table);
}

int session_add(SESSION *session)
{
    OBJECT *t_obj = (OBJECT *)(session->obj);
    t_obj->bev = (void *)(session->bev);
    g_hash_table_insert(session_table, (gconstpointer)(session->bev), session);
    LOG_INFO("session(%s) added", t_obj->IMEI);
    return 0;
}

int session_del(SESSION *session)
{
    if(!session)
    {
        LOG_ERROR("session not exist");
        return -1;
    }

    OBJECT *t_obj = (OBJECT *)(session->obj);
    if (!t_obj)
    {
        LOG_WARN("object not login before timeout");
        return 0;
    }

    if((void *)(session->bev) == t_obj->bev)
    {
        t_obj->bev = NULL;
    }
    g_hash_table_remove(session_table, (gconstpointer)(session->bev));
    LOG_INFO("session(%s) deleted", t_obj->IMEI);
    return 0;
}


SESSION *session_get(gconstpointer p)
{
    return g_hash_table_lookup(session_table, p);
}
