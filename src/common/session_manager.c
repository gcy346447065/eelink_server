/*
 * session_manager.c
 *
 *  Created on: Mar 17, 2016
 *      Author: gcy
 */

#include <glib.h>

#include "log.h"
#include "session_manager.h"

static GHashTable *sessionManager_table = NULL;
static int gSequence = 0;

void sessionManager_freeValue(gpointer value)
{
    SESSION_MANAGER *sessionManager = (SESSION_MANAGER *)value;

    LOG_DEBUG("free value sessionManager->sequence(%d) of sessionManager_table", sessionManager->sequence);

    g_free(sessionManager);
}

void sessionManager_table_initial(void)
{
    sessionManager_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, sessionManager_freeValue);
}

void sessionManager_table_destruct(void)
{
    g_hash_table_destroy(sessionManager_table);
}

int sessionManager_add(SESSION_MANAGER *sessionManager)
{
    sessionManager->sequence = ++gSequence;

    g_hash_table_insert(sessionManager_table, sessionManager->sequence, sessionManager);
    LOG_INFO("sessionManager->sequence(%d) added", sessionManager->sequence);
    return 0;
}

int sessionManager_del(SESSION_MANAGER *sessionManager)
{
    if(!sessionManager)
    {
        LOG_ERROR("sessionManager not exist");
        return -1;
    }

    g_hash_table_remove(sessionManager_table, (gconstpointer)(sessionManager->sequence));
    LOG_INFO("sessionManager->sequence(%d) deleted", sessionManager->sequence);
    return 0;
}

SESSION_MANAGER *sessionManager_get(int seq)
{
    return g_hash_table_lookup(sessionManager_table, (gconstpointer)seq);
}
