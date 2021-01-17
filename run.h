/*
 * run.h - Process creation and removal
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#ifndef KUNAI_RUN_H
#define	KUNAI_RUN_H

#include <stdbool.h>

#include "daemon.h"


const char *now_string(void);
bool run(struct daemon *d, const char *cmd);
bool stop(struct daemon *d);
/* run "reap" in response to EOF, not before ! */
void reap(struct daemon *d);

#endif /* !KUNAI_RUN_H */
