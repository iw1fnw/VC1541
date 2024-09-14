/*
 *  $Id: fs_dos.h,v 1.3 1998/10/26 03:03:09 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_dos_h
#define __libvfs_fs_dos_h

#include <sys/stat.h>

#include "libvfs/fs.h"
#include "libvfs/file.h"

class FileSystemDOS : public FileSystem
{
protected:
	void init(File *file);

public:
	FileSystemDOS(File *file);
	virtual ~FileSystemDOS(void) {}

	virtual File * open(const char *path, DirectoryEntry *e);
	virtual File * open_write(const char *path, const char *name);

	virtual void update(void);

	static FileSystem * check(File *file);
};

#endif /* __libvfs_fs_dos_h */
