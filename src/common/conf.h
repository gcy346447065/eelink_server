/*
 * conf.h
 *
 *  Created on: Jul 27, 2015
 *      Author: msj
 */

#ifndef CONF_H_
#define CONF_H_

//Func conf_initial and Func conf_save_and_close must be called in pairs.
//Nestification is wrong.
gint conf_initial(const gchar *conf_file);
gint conf_save_and_close(const gchar *conf_file);

//The space returned by Func conf_get_string is allocated in heap.The caller of this func should free it.
gchar *conf_get_string(const gchar *group_name, const gchar *key);
gint conf_get_integer(const gchar *group_name, const gchar *key);

gint conf_set_string(const gchar *group_name, const gchar *key, const gchar *value);
gint conf_set_integer(const gchar *group_name, const gchar *key, gint value);

#endif /* SRC_CONF_H_*/
