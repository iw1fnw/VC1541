/*
 *  $Id: fs_arj.h,v 1.3 1998/10/26 03:03:07 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_arj_h
#define __libvfs_fs_arj_h

#include "libvfs/fs_exec.h"

class FileSystemARJ : public FileSystemEXEC
{
private:
        virtual void parse(const char *filename);
public:
	FileSystemARJ(File *file);
	virtual ~FileSystemARJ(void);
	virtual File * open(const char *path, DirectoryEntry *e);

	static FileSystem * check(File *file);
};

#endif /* __libvfs_fs_arj_h */
