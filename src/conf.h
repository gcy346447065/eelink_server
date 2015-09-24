/*
 * conf.h
 *
 *  Created on: Jul 27, 2015
 *      Author: msj
 */

#ifndef SRC_CONF_H_
#define SRC_CONF_H_

gint conf_init(const gchar *conf_file);
gint conf_save_and_close(const gchar *conf_file);

gchar *conf_get_string(const gchar *group_name, const gchar *key);
gint conf_get_integer(const gchar *group_name, const gchar *key);

gint conf_set_string(const gchar *group_name, const gchar *key, const gchar *value);
gint conf_set_integer(const gchar *group_name, const gchar *key, gint value);

#endif /* SRC_CONF_H_*/
