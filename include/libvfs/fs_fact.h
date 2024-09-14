/*
 *  $Id: fs_fact.h,v 1.3 1998/10/26 03:03:11 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_fact_h
#define __libvfs_fs_fact_h

#include "misc/vector.h"

#include "libvfs/fs.h"
#include "libvfs/dir.h"
#include "libvfs/file.h"
#include "libvfs/device.h"

typedef FileSystem * (*check_func)(File *);

class FSFactory
{
private:
        Vector<check_func> _check;
public:
	FSFactory(void);
	~FSFactory(void);
	void register_fs(check_func f);
	FileSystem * get_fs(Device *dev, FileSystem *fs, DirectoryEntry *e);
};

#endif /* __libvfs_fs_fact_h */
