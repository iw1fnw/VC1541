/*
 *  $Id: fs_lha.h,v 1.3 1998/10/26 03:03:12 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_lha_h
#define __libvfs_fs_lha_h

#include "libvfs/fs_exec.h"

class FileSystemLHA : public FileSystemEXEC
{
private:
        virtual void parse(const char *filename);
public:
	FileSystemLHA(File *file);
	virtual ~FileSystemLHA(void);
	virtual File * open(const char *path, DirectoryEntry *e);

	static FileSystem * check(File *file);
};

#endif /* __libvfs_fs_lha_h */
