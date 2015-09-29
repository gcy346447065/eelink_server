/*
 * conf.h
 *
 *  Created on: Jul 27, 2015
 *      Author: msj
 */

#include <stdio.h>
#include <glib.h>
#include "log.h"

static GKeyFile *conf;
static GError *conf_error;

gint conf_initial(const gchar *conf_file)
{
 	conf = g_key_file_new();
 	GKeyFileFlags flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

 	if(!g_key_file_load_from_file(conf, conf_file, flags, &conf_error))
 	{
 		LOG_FATAL("can't load the configuration file from %s :%s",conf_file, conf_error->message);
 		return -1;
 	}
 	return 0;
}

gchar *conf_get_string(const gchar *group_name, const gchar *key)
{
	gchar *result = NULL;
	result = g_key_file_get_string(conf, group_name, key, &conf_error);
	if(NULL == result)
	{
		LOG_ERROR("can't get [%s]:%s from the configuration file :%s", group_name, key, conf_error->message);
		return NULL;
	}
	return result;
}

gint conf_get_integer(const gchar *group_name, const gchar *key)
{
	gint result;
	result = g_key_file_get_integer(conf, group_name, key, &conf_error);
	if(!result && conf_error && (conf_error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND || conf_error->code == G_KEY_FILE_ERROR_INVALID_VALUE))
	{
		LOG_ERROR("can't get [%s]:%s from the configuration file :%s", group_name, key, conf_error->message);
		return 0;
	}
	return result;
}


gint conf_set_string(const gchar *group_name, const gchar *key, const gchar *value)
{
	g_key_file_set_string(conf, group_name, key, value);
	return 0;
}

gint conf_set_integer(const gchar *group_name, const gchar *key, gint value)
{
	g_key_file_set_integer(conf, group_name, key, value);
	return 0;
}

gint conf_save_and_close(const gchar *conf_file)
{
	gsize length;
	gchar *conf_update = g_key_file_to_data(conf, &length, NULL);

	FILE *fp = fopen(conf_file, "w");
	if(NULL == fp)
	{
		LOG_ERROR("can't open the configuration file: %s", conf_file);
		return -1;
	}
	fwrite(conf_update, 1, length, fp);
	fclose(fp);
	g_key_file_free(conf);
	return 0;
}
