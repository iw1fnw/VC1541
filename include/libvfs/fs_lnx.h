/*
 *  $Id: fs_lnx.h,v 1.3 1998/10/26 03:03:13 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_lnx_h
#define __libvfs_fs_lnx_h

#include "libvfs/fs.h"
#include "libvfs/file.h"

class FileSystemLNX : public FileSystem
{
private:
	static const int BLOCK_LEN = 254;
	static const int CHECK_LEN = 254;
	int lnx_get_header(char *buf, word_t *dirlen, word_t *direntries);
	char * lnx_getstr(word_t *a, char *dirmem, word_t dirlen);
	static bool check_type_lnx(File *file);
public:
	FileSystemLNX(File *file);
	virtual ~FileSystemLNX(void);
	File * open(const char *path, DirectoryEntry *e);

	static FileSystem * check(File *file);
};

#endif /* __libvfs_fs_lnx_h */

