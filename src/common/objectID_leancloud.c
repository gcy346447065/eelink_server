/*
 * objectID_leancloud.c
 *
 *  Created on: Mar 29, 2016
 *      Author: gcy
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "db.h"
#include "log.h"
#include "macro.h"
#include "objectID_leancloud.h"

/* global hash table */
static GHashTable *objectID_table = NULL;

static void print_key_value(gpointer key, gpointer value, gpointer user_data)
{
    user_data = user_data;
    LOG_INFO("%s----->%s", (char *)key, (char *)value);
}

void objectID_table_display(void)
{
    g_hash_table_foreach(objectID_table, print_key_value, NULL);
}

int objectID_add_hash(const char *imei, const char *objectID)
{
    if(strlen(imei) != IMEI_LENGTH || strlen(objectID) != OBJECT_ID_LENGTH)
    {
        LOG_ERROR("imei(%s) or objectID(%s) string length error", imei, objectID);
        return -1;
    }

    int rc = g_hash_table_insert(objectID_table, g_strdup(imei), g_strdup(objectID));
    LOG_INFO("rc(%d), add hash imei(%s)->objectID(%s)",rc , imei, objectID);

    return 0;
}

int objectID_add_db(const char *imei, const char *objectID)
{
    if(strlen(imei) != IMEI_LENGTH || strlen(objectID) != OBJECT_ID_LENGTH)
    {
        LOG_ERROR("imei(%s) or objectID(%s) string length error", imei, objectID);
        return -1;
    }

    db_add_ObjectID(imei, objectID);
    LOG_INFO("add db imei(%s)->objectID(%s)", imei, objectID);

    return 0;
}

int objectID_add_HashAndDb(const char *imei, const char *objectID)
{
    int rc = 0;
    rc |= objectID_add_hash(imei, objectID);
    rc |= objectID_add_db(imei, objectID);

    return rc;
}

char *objectID_get_hash(const char *imei)
{
    if(strlen(imei) != IMEI_LENGTH)
    {
        LOG_ERROR("imei string length error");
        return NULL;
    }

    char *objectID = g_hash_table_lookup(objectID_table, imei);
    if(!objectID)
    {
        LOG_ERROR("can't get objectID from hash table");
        return NULL;
    }
    else
    {
        LOG_INFO("get objectID(%s) with imei(%s)", objectID, imei);
        return objectID;
    }
}

void objectID_freeKey(gpointer key)
{
    LOG_DEBUG("free key IMEI(%s) of objectID_table", (char *)key);
    g_free(key);
}

void objectID_freeValue(gpointer value)
{
    LOG_DEBUG("free value objectID(%s) of objectID_table", (char *)value);
    g_free(value);
}

int objectID_table_initial(void)
{
    /* create hash table */
    objectID_table = g_hash_table_new_full(g_str_hash, g_str_equal, objectID_freeKey, objectID_freeValue);
    if(objectID_table == NULL)
    {
        LOG_ERROR("create imei->objectID hash table failed");
        return -1;
    }

    /* read imei data from db*/
    db_doWithObjectID(objectID_add_hash);

    return 0;
}

void objectID_table_destruct(void)
{
    g_hash_table_destroy(objectID_table);

    return;
}



