/*
 *  $Id: fs_exec.h,v 1.3 1998/10/26 03:03:10 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_exec_h
#define __libvfs_fs_exec_h

#include "libvfs/fs.h"
#include "libvfs/file.h"

class FileSystemEXEC : public FileSystem
{
private:
        virtual void parse(const char *filename) = 0;
protected:
	static bool check_type_exec(File *file, long offset, int len,
				const char *magic);
public:
	FileSystemEXEC(File *file, const char *fstype);
	virtual ~FileSystemEXEC(void);

	virtual File * open(const char *path, DirectoryEntry *e) = 0;
};

#endif /* __libvfs_fs_exec_h */

