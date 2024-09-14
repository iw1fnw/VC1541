/*
 *  $Id: util.h,v 1.3 1998/10/26 03:03:26 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __misc_util_h
#define __misc_util_h

char * util_strdup(const char *s);
int util_system(const char *cmd, bool close_stdin);
int util_exec_wait(const char *cmd, bool close_stdin, const char *ofile, ...);
int util_glob(const char *f1, const char *f2, bool ignorecase = false);

#endif /* __misc_util_h */
