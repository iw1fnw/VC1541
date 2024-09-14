/*
 *  $Id: fs_zipc.h,v 1.3 1998/10/26 03:03:18 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_zipc_h
#define __libvfs_fs_zipc_h

#include "libvfs/fs.h"
#include "libvfs/file.h"

class FileSystemZIPC : public FileSystem
{
private:
	static const int CHECK_LEN = 2;
protected:
	static bool check_type_zipc(File *file);
public:
	FileSystemZIPC(File *file);
	virtual ~FileSystemZIPC(void);

	File * open(const char *path, DirectoryEntry *e);

	static FileSystem * check(File *file);
};

#endif /* __libvfs_fs_zipc_h */
