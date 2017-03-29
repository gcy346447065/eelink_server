/*
 * object.c
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib.h>

#include "log.h"
#include "itinerary_object.h"

static GHashTable *object_table = NULL;

static void itineraryObj_ItieraryJudge(gpointer key, gpointer value, gpointer user_data)
{
    char *imei = (char *)key;
    ITINERARY_OBJECT *obj = (ITINERARY_OBJECT *)value;
    ITINERARY_PROC fun = (ITINERARY_PROC)user_data;

    if(++obj->timecount >= 5)
    {
        if(obj->starttime < obj->timestamp && obj->isStarted)
        {
            fun(obj->IMEI, obj->starttime, obj->startlat, obj->startlon, obj->timestamp, obj->lat, obj->lon, obj->itineray);
        }
        itineraryObj_del(obj);
    }

    return;
}

void itineraryObj_table_ItieraryJudge(void *arg)
{
    g_hash_table_foreach(object_table, (GHFunc)itineraryObj_ItieraryJudge, arg);
}

static void itineraryObj_freeKey(gpointer key)
{
    LOG_DEBUG("free key IMEI:%s of object_table", (char *)key);
    g_free(key);
}

static void itineraryObj_freeValue(gpointer value)
{
    ITINERARY_OBJECT *obj = (ITINERARY_OBJECT *)value;

    LOG_DEBUG("free value IMEI:%s of object_table", obj->IMEI);

    g_free(obj);
}

void itineraryObj_table_initial()
{
    object_table = g_hash_table_new_full(g_str_hash, g_str_equal, itineraryObj_freeKey, itineraryObj_freeValue);
}

void itineraryObj_table_destruct()
{
    g_hash_table_destroy(object_table);
}

void itineraryObj_add(ITINERARY_OBJECT *obj)
{
	const char* strIMEI = obj->IMEI;
	g_hash_table_insert(object_table, g_strdup(strIMEI), obj);
    LOG_INFO("obj %s added to hashtable", strIMEI);
}

ITINERARY_OBJECT *itineraryObj_new()
{
	ITINERARY_OBJECT *obj = g_malloc(sizeof(ITINERARY_OBJECT));
    if(!obj)
    {
        return NULL;
    }
    memset(obj, 0, sizeof(ITINERARY_OBJECT));
	obj->isStarted = 0;
    obj->itineray = 0;
    obj->timecount = 0;
	return obj;
}

void itineraryObj_del(ITINERARY_OBJECT *obj)
{
    ITINERARY_OBJECT *t_obj = itineraryObj_get(obj->IMEI);
    if(t_obj)
    {
        g_hash_table_remove(object_table, obj->IMEI);
    }
}

ITINERARY_OBJECT *itineraryObj_get(const char *IMEI)
{
    return g_hash_table_lookup(object_table, IMEI);
}

