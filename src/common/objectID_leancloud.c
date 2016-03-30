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

#include "log.h"
#include "macro.h"
#include "objectID_leancloud.h"

/* global hash table */
static GHashTable *objectID_table = NULL;

static int objectID_add_hash(const char *imei, const char *objectID)
{
    if(strlen(imei) != IMEI_LENGTH || strlen(objectID) != OBJECT_ID_LENGTH)
    {
        LOG_ERROR("imei or objectID string length error");
        return -1;
    }

    g_hash_table_insert(objectID_table, imei, objectID);
    LOG_INFO("add hash imei(%s)->objectID(%s)", imei, objectID);

    return 0;
}

static int objectID_add_db(const char *imei, const char *objectID)
{
    if(strlen(imei) != IMEI_LENGTH || strlen(objectID) != OBJECT_ID_LENGTH)
    {
        LOG_ERROR("imei or objectID string length error");
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

    return;
}

char *objectID_get_hash(const char *imei)
{
    if(strlen(imei) != IMEI_LENGTH)
    {
        LOG_ERROR("imei string length error");
        return -1;
    }

    char *objectID = g_hash_table_lookup(objectID_table, imei);
    if(strlen(objectID) != OBJECT_ID_LENGTH)
    {
        LOG_ERROR("get objectID string length error");
        return NULL;
    }
    else
    {
        LOG_INFO("get objectID(%s) with imei(%s)", objectID, imei);
        return objectID;
    }
}

int objectID_table_initial(void)
{
    /* create hash table */
    objectID_table = g_hash_table_new(g_str_hash, g_str_equal);
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

