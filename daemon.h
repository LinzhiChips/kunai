/*
 * daemon.h - Daemon operation
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#ifndef KUNAI_DAEMON_H
#define	KUNAI_DAEMON_H

#define	BUF_SIZE	4096


struct logfile {
	const char	*path;
	struct logfile	*next;
};

struct daemon {
	/* configuration */
	const char	*name;
// @@@ keep is simple for now
//	const char	*setup;
	const char	*start;
//	const char	*stop;
//	const char	*restart;

	pid_t		pid;		/* 0 if not running */
	int		out;		/* stdout and stderr */
	bool		cycle;		/* restart when kill is confirmed;
					   on start: start this daemon */

	char		buf[BUF_SIZE];	/* input line buffer */
	unsigned	buf_offset;

//	struct logfile	*log_files;
//	struct logfile	*curr_log;	/* current log file, NULL if none */
//	int		log_fd;		/* undefined if curr_log == NULL */
	char		*log_buf;	/* log buffer */

	unsigned	log_limit;	/* log size limit */
	unsigned	log_size;	/* current log size */

	struct daemon	*next;
};


extern struct daemon *daemons;


void daemon_log(struct daemon *d, const char *s, unsigned len);
void daemon_err(struct daemon *d, const char *fmt, ...);

void daemon_current_get(struct daemon *d);
void daemon_stopped(struct daemon *d);
void daemon_eof(struct daemon *d);
void daemon_in(struct daemon *d);

void daemon_start(struct daemon *d);
void daemon_stop(struct daemon *d);
void daemon_cycle(struct daemon *d);
void daemon_restart_if_cycling(struct daemon *d);

struct daemon *daemon_new(const char *name);

#endif /* !KUNAI_DAEMON_H */
