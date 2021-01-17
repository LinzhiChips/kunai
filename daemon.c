/*
 * daemon.c - Daemon operation
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#define	_GNU_SOURCE	/* for asprintf, vasprintf */
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>

#include "linzhi/alloc.h"
#include "linzhi/mqtt.h"

#include "kunai.h"
#include "mqtt.h"
#include "run.h"
#include "daemon.h"


struct daemon *daemons = NULL;


void daemon_log(struct daemon *d, const char *s, unsigned len)
{
	assert(len <= d->log_limit);
	if (d->log_size + len > d->log_limit) {
		unsigned diff = d->log_size + len - d->log_limit;

		d->log_size -= diff;
		d->log_buf =
		    memmove(d->log_buf, d->log_buf + diff, d->log_size);
	}
	d->log_buf = realloc_size(d->log_buf, d->log_size + len);
	memcpy(d->log_buf + d->log_size, s, len);
	d->log_size += len;
	mqtt_printf_arg(MQTT_TOPIC_LOG, qos_once, 0, d->name, "%.*s", len, s);
}


void daemon_err(struct daemon *d, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	vasprintf(&s, fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s: %s\n", d->name, s);
	daemon_log(d, s, strlen(s));
	free(s);
}


void daemon_current_get(struct daemon *d)
{
	mqtt_printf_arg(MQTT_TOPIC_CURRENT, qos_once, 0, d->name, "%.*s",
	    (int) d->log_size, d->log_buf);
}


void daemon_eof(struct daemon *d)
{
	reap(d);
	mqtt_printf_arg(MQTT_TOPIC_TIME, qos_ack, 1, d->name, "0");
}


void daemon_in(struct daemon *d)
{
	ssize_t got;

	while (1) {
		got = read(d->out, d->buf + d->buf_offset,
		    BUF_SIZE - d->buf_offset);
		if (got < 0) {
			if (errno != EAGAIN)
				perror("read");
			break;
		}
		if (!got) {
			daemon_eof(d);
			return;
		}
		d->buf_offset += got;
		if (d->buf_offset == BUF_SIZE ||
		    memchr(d->buf, '\n', d->buf_offset)) {
			daemon_log(d, d->buf, d->buf_offset);
			d->buf_offset = 0;
		}
	}
}


void daemon_start(struct daemon *d)
{
	struct timespec ts;

	if (d->pid) {
		fprintf(stderr, "%s: already running (PID %d)\n",
		    d->name, (int) d->pid);
		return;
	}
	if (clock_gettime(CLOCK_BOOTTIME, &ts) < 0) {
		perror("clock_gettime");
		exit(1);
	}
	d->cycle = 0;
	daemon_err(d, "starting \"%s\", %s (%lu)",
	    d->start, now_string(), (unsigned long) ts.tv_sec);
	if (run(d, d->start))
		mqtt_printf_arg(MQTT_TOPIC_TIME, qos_ack, 1, d->name, "%lu",
		    (unsigned long) ts.tv_sec);
}


void daemon_stop(struct daemon *d)
{
	d->cycle = 0;
	if (!d->pid) {
		fprintf(stderr, "%s: not running\n", d->name);
		return;
	}
	if (!stop(d))
		fprintf(stderr, "warning: could not stop \"%s\"\n", d->name);
}


void daemon_cycle(struct daemon *d)
{
	if (d->pid) {
		daemon_stop(d);
		d->cycle = 1;
	}
}


void daemon_restart_if_cycling(struct daemon *d)
{
	if (d->cycle)
		daemon_start(d);
}


struct daemon *daemon_new(const char *name)
{
	struct daemon *d;
	struct daemon **anchor;

	d = alloc_type(struct daemon);
	d->name = stralloc(name);
	d->start = NULL;
	d->pid = 0;
	d->out = -1;
	d->cycle = 0;
	d->buf_offset = 0;
	d->log_buf = NULL;
	d->log_limit = LOG_LIMIT;
	d->log_size = 0;
	d->next = NULL;
	for (anchor = &daemons; *anchor; anchor = &(*anchor)->next);
	*anchor = d;
	return d;
}
