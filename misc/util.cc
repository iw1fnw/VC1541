/*
 *  $Id: util.cc,v 1.3 1998/10/26 03:03:39 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <cstring>

#include "misc/util.h"
#include "misc/debug.h"

#define DEBUG_LEVEL 1
#define _MODULE_ "util: "

char * util_strdup(const char *s)
{
	if (s == NULL) return NULL;
	char *tmp = new char[strlen(s) + 1];
	strcpy(tmp, s);
	return tmp;
}

int util_system(const char *cmd, bool close_stdin)
{
	int a, fd, stdin_fd, stderr_fd;

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "system: " << cmd << "\n";
#endif

	stdin_fd = 1;
	fd = open("/dev/null", O_RDWR);

        stderr_fd = dup(2); 		/* save stderr */
        close(2);        		/* close stderr */
        dup2(fd, 2);			/* new stderr */

        if (close_stdin) {
	        stdin_fd  = dup(0);	/* save stdin */
	        close(0);		/* close stdin */
	        dup2(fd, 0);		/* new stdin */
        }

        a = system(cmd);		/* run command */
        
        dup2(stderr_fd, 2);		/* restore stderr */
        close(stderr_fd);

        if (a != 0) {
                *debug << _MODULE_ "system: exit code = " << a << endl;
                *debug << _MODULE_ "system: " << strerror(errno)
                       << endl;
        }

        if (close_stdin) {
	        dup2(stdin_fd, 0);	/* restore stdin */
	        close(stdin_fd);	/* close temporary files */
        }

        close(fd);

        return a;
}

int util_exec_wait(const char *prg, bool close_stdin, const char *ofile, ...)
{
        va_list ap;
        char **argv;
	int a, status, len, pid, fd, out_fd;

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "exec_wait: " << prg << " -> "
               << ofile << endl;
#endif


        va_start(ap, ofile);
        for (a = 0;va_arg(ap, char *) != NULL;a++); /* count arguments */
        len = a + 1;
        va_end(ap);

        argv = new char * [len + 1];
        argv[0] = util_strdup(prg);
        va_start(ap, ofile);
        for (a = 1;a < len;a++) {
                argv[a] = util_strdup(va_arg(ap, char *));
        }
        argv[a] = NULL;
        va_end(ap);

#if DEBUG_LEVEL > 1
        a = -1;
        do {
                a++;
                *debug << _MODULE_ "exec_wait: arg = " << a << ", val = "
                       << argv[a] << endl;
        } while (argv[a] != NULL);
#endif
        
        pid = fork();
        if (pid < 0) return -1;
        
        if (pid == 0) {
                fd = open("/dev/null", O_RDWR);
                if (fd < 0) exit(-1);
                out_fd = open(ofile, O_RDWR | O_CREAT | O_EXCL,
                              S_IRUSR | S_IWUSR);
                if (out_fd < 0) exit(-1);

                close(2);        		/* close stderr */
                dup2(fd, 2);		        /* new stderr (/dev/null) */
                if (close_stdin) {
                    close(0);	                /* close stdin */
                    dup2(fd, 0);		/* new stdin (/dev/null) */
                }
                close(1);                       /* close stdout */
                dup2(out_fd, 1);                /* new stdout (ofile) */
                
                execvp(prg, argv);
                exit(-1);
        } else {
#if DEBUG_LEVEL > 1
                *debug << _MODULE_ "exec_wait: waiting for "
                       << pid << endl;
#endif
                while (waitpid(pid, &status, 0) != pid);
#if DEBUG_LEVEL > 1
                *debug << _MODULE_ "exec_wait: child exited" << endl;
#endif
        }

        if (WIFSIGNALED(status)) {
                *debug << _MODULE_ "exec_wait: got signal "
                       << status << endl;
        }
        if (WEXITSTATUS(status) != 0) {
                *debug << _MODULE_ "exec_wait: exit code "
                       << WEXITSTATUS(status) << endl;
        }

        for (a = 1;a < len;a++) {
                delete argv[a];
        }
        delete argv;

        return WEXITSTATUS(status);
}

int util_glob(const char *f1, const char *f2, const bool ignorecase)
{
	while (1) {
		if (*f1 == '\0' && *f2 == '\0') return 1;
		if (*f1 == '\0') return 0;
		if (*f2 == '\0') {
			if (*f1 == '*') return 1;
			return 0;
		}
		if (ignorecase) {
			if (toupper(*f1) == toupper(*f2)) {
				f1++;
				f2++;
				continue;
			}
		} else {
			if (*f1 == *f2) {
				f1++;
				f2++;
				continue;
			}
		}
		if (*f1 == '*') return 1;
		return 0;
	}
}
