/*
 *  $Id: fs_exec.cc,v 1.2 1998/10/26 03:02:15 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <string.h>
#include <stdlib.h>

#include "libvfs/fs_exec.h"

#define DEBUG_LEVEL 0
#define _MODULE_ "FS [EXEC]: "

FileSystemEXEC::FileSystemEXEC(File *file, const char *fstype) :
	FileSystem(file, fstype)
{
#if DEBUG_LEVEL > 9
        *debug << _MODULE_ << "CONSTRUCTOR" << endl;
#endif
}

FileSystemEXEC::~FileSystemEXEC(void)
{
#if DEBUG_LEVEL > 9
        *debug << _MODULE_ << "DESTRUCTOR" << endl;
#endif
}

bool FileSystemEXEC::check_type_exec(File *file, long offset, int len, const char *magic)
{
	int a;
	char buf[len];

	if (!file->is_regular()) return false;

        if (file->seek(offset)) {
        	a = file->read(buf, len);
                file->rewind();
		if (a == len) {
                	if (strncmp(buf, magic, len) == 0) return true;
                }
        } else {
        	file->rewind();
        }

	return false;
}

