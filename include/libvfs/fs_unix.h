/*
 *  $Id: fs_unix.h,v 1.3 1998/10/26 03:03:16 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_unix_h
#define __libvfs_fs_unix_h

#include <sys/stat.h>

#include "libvfs/fs.h"
#include "libvfs/file.h"

class FileSystemUNIX : public FileSystem
{
public:
	FileSystemUNIX(File *file);
	virtual ~FileSystemUNIX(void);

	static FileSystem * check(File *file);
};

#endif /* __libvfs_fs_unix_h */

