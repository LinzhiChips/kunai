/*
 * mqtt.c - MQTT setup and input processing
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#include <stddef.h>
#include <poll.h>

#include "linzhi/alloc.h"
#include "linzhi/mqtt.h"

#include "kunai.h"
#include "daemon.h"
#include "mqtt.h"


#define	POLL_MS		1000


/* ----- Event loop -------------------------------------------------------- */


void mqtt_loop(void)
{
	struct daemon *d;
	struct pollfd *fds;
	unsigned n = 0;
	unsigned i;

	for (d = daemons; d; d = d->next)
		n++;
	fds = alloc_type_n(struct pollfd, n + 1);
	fds[0].fd = mqtt_fd();

	while (1) {
		int got;

		fds[0].events = mqtt_events();
		i = 1;
		for (d = daemons; d; d = d->next)
			if (d->pid) {
				fds[i].fd = d->out;
				fds[i].events = POLLIN;
				i++;
			}

		got = poll(fds, i, POLL_MS);
		if (got < 0) {
			perror("poll");
			exit(1);
		}
		if (got) {
			i = 1;
			for (d = daemons; d; d = d->next)
				if (d->pid) {
					if (fds[i].revents & POLLIN)
						daemon_in(d);
					else if (fds[i].revents &
					    (POLLHUP | POLLERR))
						daemon_eof(d);
					i++;
				}
		}

		mqtt_poll(fds[0].revents);
	}
}

/* ----- Setup and subscriptions ------------------------------------------- */


static void cb_current_get(void *user, const char *topic, const char *msg)
{
	daemon_current_get(user);
}


static void cb_start(void *user, const char *topic, const char *msg)
{
	daemon_start(user);
}


static void cb_stop(void *user, const char *topic, const char *msg)
{
	daemon_stop(user);
}


static void cb_cycle(void *user, const char *topic, const char *msg)
{
	daemon_cycle(user);
}


void mqtt_setup(void)
{
	struct daemon *d;

	for (d = daemons; d; d = d->next) {
		mqtt_subscribe(MQTT_TOPIC_CURRENT_GET, qos_ack, cb_current_get,
		    d, d->name);
		mqtt_subscribe(MQTT_TOPIC_START, qos_ack, cb_start, d, d->name);
		mqtt_subscribe(MQTT_TOPIC_STOP, qos_ack, cb_stop, d, d->name);
		mqtt_subscribe(MQTT_TOPIC_CYCLE, qos_ack, cb_cycle, d, d->name);
	}
	/* MQTT_TOPIC_SHUTDOWN */

	mqtt_init(NULL, 0);
}
