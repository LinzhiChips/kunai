/*
 * cfg.c - Configuration file parser
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "linzhi/alloc.h"

#include "kunai.h"
#include "daemon.h"
#include "cfg.h"


/*
 * Syntax:
 * daemon-name start-command
 */


#define	MAX_LINE	1024
#define	WHITESPACE	" \t\r\n"


void cfg_parse(FILE *file)
{
	struct daemon *d;
	char buf[MAX_LINE];
	const char *cmd;
	char *end;
	size_t blanks, name_len;

	while (fgets(buf, sizeof(buf), file)) {
		end = strchr(buf, '\n');
		if (end)
			*end = 0;
		end = strchr(buf, '#');
		if (end)
			*end = 0;
		blanks = strspn(buf, WHITESPACE);
		if (blanks == strlen(buf))
			continue;
		if (blanks) {
			fprintf(stderr, "syntax error\n");
			exit(1);
		}
		name_len = strcspn(buf, WHITESPACE);
		buf[name_len] = 0;

		cmd = buf + name_len + 1;
		blanks = strspn(cmd, WHITESPACE);
		if (blanks == strlen(cmd)) {
			fprintf(stderr, "no command for %s\n", buf);
			exit(1);
		}
		cmd += blanks;

		d = daemon_new(stralloc(buf));
		if (*cmd == '!') {
			d->start = stralloc(cmd + 1);
			d->cycle = 0;
		} else {
			d->start = stralloc(cmd);
			d->cycle = 1;
		}
		if (verbose)
			fprintf(stderr, "added \"%s\" with \"%s\"\n",
			    d->name, d->start);
	}
}
