//
// Created by jk on 15-10-30.
//

#ifndef ELECTROMBILE_MSG_PROC_H
#define ELECTROMBILE_MSG_PROC_H

#include <stddef.h>

int handle_incoming_msg(const char *m, size_t msgLen, void *arg);

#endif //ELECTROMBILE_MSG_PROC_H
