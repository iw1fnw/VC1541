/*
 *  $Id$
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_dev_unx_h
#define __libvfs_dev_unx_h

#include "libvfs/fs.h"

class FileSystemDevUNIX : public FileSystem
{
public:
	FileSystemDevUNIX(void);
	virtual ~FileSystemDevUNIX(void);
};

#endif /* __libvfs_dev_unx_h */
