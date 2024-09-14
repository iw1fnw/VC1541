/*
 *  $Id: fs_t64.h,v 1.3 1998/10/26 03:03:14 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_t64_h
#define __libvfs_fs_t64_h

#include "libvfs/fs.h"
#include "libvfs/file.h"

class FileSystemT64 : public FileSystem
{
private:
	static const int CHECK_LEN = 20;
	static bool check_type_t64(File *file);
public:
	FileSystemT64(File *file);
	virtual ~FileSystemT64(void);

	virtual File * open(const char *path, DirectoryEntry *e);

	static FileSystem * check(File *file);
};

struct t64_tape {
	char  desc[32];
	char  minor;
	char  major;
	short entries;
	short used;
	short _unused;
	char  user_desc[24];
};

struct t64_file {
	unsigned char  entry_type;
	unsigned char  file_type;
	unsigned short start;
	unsigned short end;
	short          _unused1;
	unsigned long  offset;
	long           _unused2;
	char           name[16];
};

typedef struct t64_tape t64_tape_t;
typedef struct t64_file t64_file_t;

#endif /* __libvfs_fs_t64_h */
