/*
 * objectID_leancloud.h
 *
 *  Created on: Mar 29, 2016
 *      Author: gcy
 */

#ifndef OBJECT_ID_LEANCLOUD_H
#define OBJECT_ID_LEANCLOUD_H

void objectID_table_display(void);
int objectID_table_initial(void);
void objectID_table_destruct(void);
int objectID_add_HashAndDb(const char *imei, const char *objectID);
char *objectID_get_hash(const char *imei);

#endif //OBJECT_ID_LEANCLOUD_H

