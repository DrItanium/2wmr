/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "2wm.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* extern */

void *
emallocz(unsigned int size) {
	void *res = calloc(1, size);

    ENTER_FUNC;
	if(!res)
		eprint("fatal: could not malloc() %u bytes\n", size);
    EXIT_FUNC;
	return res;
}

void
eprint(const char *errstr, ...) {
	va_list ap;

    ENTER_FUNC;
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
    EXIT_FUNC;
}

void
spawn(Arg *arg) {
	static char *shell = NULL;

    ENTER_FUNC;
	if(!shell && !(shell = getenv("SHELL")))
		shell = "/bin/sh";
	if(!arg->cmd) {
        EXIT_FUNC;
		return;
    }
	/* The double-fork construct avoids zombie processes and keeps the code
	 * clean from stupid signal handlers. */
	if(fork() == 0) {
		if(fork() == 0) {
			if(dpy)
				close(ConnectionNumber(dpy));
			setsid();
			execl(shell, shell, "-c", arg->cmd, (char *)NULL);
			fprintf(stderr, "2wm: execl '%s -c %s'", shell, arg->cmd);
			perror(" failed");
		}
		exit(0);
	}
	wait(0);
    EXIT_FUNC;
}

void
enterFunction(HERE_DECL__) {
    fprintf(stderr, "%s, %ld: Entered function %s\n", file, line, func);
}

void
exitFunction(HERE_DECL__) {
    fprintf(stderr, "%s, %ld: Exiting function %s\n", file, line, func);
}
