/*
 *  $Id$
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_f_d64_h
#define __libvfs_f_d64_h

#include "libvfs/file.h"
#include "libvfs/fs_d64.h"

class FileD64 : public File
{
protected:
	FileSystemD64 *_fs;
	file_mode_t _mode;
	track_sector_t _ts;
	track_sector_t _ts_dir;
	int _dir_slot;
	int _count;
	int _blocks;
	byte_t *_buf;

public:
	FileD64(FileSystemD64 *fs,
		track_sector_t ts,		/* track/sector for data */
		track_sector_t ts_dir,		/* track/sector of directory entry */
		int dir_slot,			/* index in directory block */
		const char *path,
		const char *name = NULL,
		file_mode_t mode = File_RO);
        virtual ~FileD64(void);

	virtual void   rewind(void);
	virtual bool   seek(long offset);
	virtual long   read(void *buf, long len);
        virtual long   write(void *buf, long len);
	virtual int    getc(void);
	virtual int    putc(int c);
        virtual char * fgets(void *buf, long maxlen);
};

#endif /*  __libvfs_f_d64_h */
