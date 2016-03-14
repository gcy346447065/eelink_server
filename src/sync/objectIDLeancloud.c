/*
 * objectIDLeancloud.c
 *
 *  Created on: Mar 11, 2016
 *      Author: gcy
 */
#include <glib.h>

#include "objectIDLeancloud.h"

/* global hash table */
static GHashTable *objectID_table = NULL;

static void objectID_freeKey(gpointer key)
{
    LOG_DEBUG("free key imei:%s of objectID_table", (char *)key);
    g_free(key);

    return;
}

static void objectID_freeValue(gpointer value)
{
    LOG_DEBUG("free value objectID:%s of objectID_table", (char *)value);
    g_free(value);

    return;
}

void objectID_table_initial(void)
{
    /* create hash table */
    objectID_table = g_hash_table_new_full(g_str_hash, g_str_equal, objectID_freeKey, objectID_freeValue);

    return;
}

void objectID_table_destruct(void)
{
    g_hash_table_destroy(objectID_table);

    return;
}

void objectID_add_hash(const char *imei, const char *objectID)
{
    g_hash_table_insert(objectID_table, imei, objectID);
    LOG_INFO("imei(%s), objectID(%s) added to hashtable", imei, objectID);

    return;
}

void objectID_del_hash(const char *imei)
{
    if(g_hash_table_lookup(objectID_table, imei))
    {
        g_hash_table_remove(objectID_table, imei);
    }

    return;
}
