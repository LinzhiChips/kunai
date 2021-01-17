/*
 * kunai.c - Daemon supervisor ("daemon lord")
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "linzhi/mqtt.h"

#include "daemon.h"
#include "mqtt.h"
#include "cfg.h"
#include "kunai.h"


bool verbose = 0;


static void start(void)
{
	struct daemon *d;

	for (d = daemons; d; d = d->next)
		daemon_start(d);
	mqtt_loop();
}


static void usage(const char *name)
{
	fprintf(stderr, "usage: %s [-c config_file] [-v ...]\n", name);
	exit(1);
}


int main(int argc, char **argv)
{
	const char *config_file = NULL;
	int c;

	while ((c = getopt(argc, argv, "c:v")) != EOF)
		switch (c) {
		case 'c':
			config_file = optarg;
			break;
		case 'v':
			verbose = 1;
			mqtt_verbose++;
			break;
		default:
			usage(*argv);
		}

	switch (argc - optind) {
	case 0:
		break;
	default:
		usage(*argv);
	}

	if (config_file) {
		FILE *file = fopen(config_file, "r");

		if (!file) {
			perror(config_file);
			exit(1);
		}
		cfg_parse(file);
	} else {
		cfg_parse(stdin);
	}

	mqtt_setup();
	start();

	return 0;
}
