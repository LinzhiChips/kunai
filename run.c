/*
 * run.c - Process creation and removal
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


#include "daemon.h"
#include "run.h"


const char *now_string(void)
{
	static char buf[30];	/* 21 should be enough */
	struct tm *tm;
	time_t t;

	if (time(&t) == (time_t) -1) {
		perror("time");
		exit(1);
	}
	tm = gmtime(&t);
	if (!strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%SZ", tm))
		abort();
	return buf;
}


bool run(struct daemon *d, const char *cmd)
{
	int pipefd[2];

	d->pid = 0;
	if (pipe(pipefd) < 0) {
		perror("pipe");
		return 0;
	}
	d->pid = fork();
	if (d->pid < 0) {
		perror("fork");
		(void) close(pipefd[0]);
		(void) close(pipefd[1]);
		return 0;
	}
	if (d->pid) {
		d->out = pipefd[0];
		(void) close(pipefd[1]);
		if (fcntl(d->out, F_SETFL, O_NONBLOCK) < 0)
			perror("fcntl(O_NONBLOCK");
		return 1;
	}

	/* in child process */
	if (setsid() < 0) {
		perror("setsid");
		_exit(1);
	}
	(void) close(0);
	(void) close(pipefd[0]);
	if (dup2(pipefd[1], 1) < 0) {
		fprintf(stderr, "dup2(%d, 1): %s\n",
		    pipefd[1], strerror(errno));
		_exit(1);
	}
	if (dup2(pipefd[1], 2) < 0) {
		fprintf(stderr, "dup2(%d, 2): %s\n",
		    pipefd[1], strerror(errno));
		_exit(1);
	}
	(void) close(pipefd[1]);

	execl("/bin/sh", "sh", "-c", cmd, NULL);
	perror(cmd);
	_exit(1);
}


bool stop(struct daemon *d)
{
	if (kill(-d->pid, SIGTERM) < 0) {
		daemon_err(d, "kill(%d): %s\n", (int) -d->pid, strerror(errno));
		return 0;
	}
	return 1;
}


void reap(struct daemon *d)
{
	pid_t pid = d->pid;
	int res, status;
	const char *t;

	d->pid = 0;
	(void) close(d->out);
	d->out = -1;
	res = waitpid(pid, &status, WNOHANG);
	if (res < 0) {
		daemon_err(d, "waitpid(%d, WNOHANG): %s\n",
		    (int) pid, strerror(errno));
		return;
	}
	if (!res) {
		fprintf(stderr, "%s: waiting to reap %d\n", d->name, pid);
		res = waitpid(pid, &status, 0);
		if (res < 0) {
			daemon_err(d, "waitpid(%d, 0): %s\n",
			    (int) pid, strerror(errno));
			return;
		}
	}
	t = now_string();
	if (WIFEXITED(status))
		daemon_err(d, "exit status %d, %s\n",
		    WEXITSTATUS(status), t);
	 else if (WIFSIGNALED(status))
		daemon_err(d, "signal %s (%d), %s\n",
		    strsignal(WTERMSIG(status)), WTERMSIG(status), t);
	else
		daemon_err(d, "exit status (undecoded) %d, %s\n", status, t);
	daemon_restart_if_cycling(d);
}
