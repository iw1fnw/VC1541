/*
 *  $Id: dev_dos.h,v 1.2 1998/10/26 03:03:01 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_dev_dos_h
#define __libvfs_dev_dos_h

#include "libvfs/fs.h"

class FileSystemDevDOS : public FileSystem
{
public:
	FileSystemDevDOS(void);
	virtual ~FileSystemDevDOS(void);

	int checkDevice(int device);
};

#endif /* __libvfs_dev_dos_h */
