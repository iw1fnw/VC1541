/*
 *  $Id$
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_zoo_h
#define __libvfs_fs_zoo_h

#include "libvfs/fs_exec.h"

class FileSystemZOO : public FileSystemEXEC
{
private:
        virtual void parse(const char *filename);
public:
	FileSystemZOO(File *file);
	virtual ~FileSystemZOO(void);
	virtual File * open(const char *path, DirectoryEntry *e);

	static FileSystem * check(File *file);
};

#endif /* __libvfs_fs_zoo_h */
