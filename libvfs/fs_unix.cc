/*
 *  $Id: fs_unix.cc,v 1.6 1998/10/26 03:02:21 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifdef LINUX

#include <ctype.h>
#include <dirent.h>
#include <sys/vfs.h>

#include "misc/debug.h"
#include "libvfs/fs_unix.h"

FileSystemUNIX::FileSystemUNIX(File *file) : FileSystem(file, "UNIX Directory")
{
	DIR *d;
	struct stat st;
        struct statfs stfs;
	const char *path;
	char *type, *buf;
	struct dirent *ent;
	int a, len, blocks;
	char p[1024];
	dword_t free;

	path = file->path();
        if (statfs(path, &stfs) != 0) return;
	if ((d = opendir(path)) == NULL) return;

	diremu_init();

	free = ((dword_t)stfs.f_bfree * (dword_t)stfs.f_bsize) / 254;
	_dir->set_title(file->name(), free);
	diremu_set_title(file->name(), free);

	while ((ent = readdir(d)) != NULL) {
		a = strlen(path);
		if (a > 0 && path[a - 1] == '/') {
			sprintf(p, "%s%s", path, ent->d_name);
		} else {
			sprintf(p, "%s/%s", path, ent->d_name);
		}
		if (stat(p, &st) == -1) {
			type = " ??? ";
		} else {
			if (st.st_mode & S_IFDIR) {
				type = " DIR ";
			} else {
				type = " PRG ";
			}
		}
		blocks = (st.st_size + 253) / 254;

		len = strlen(ent->d_name);
		buf = new char[len + 1];
		strcpy(buf, ent->d_name);
		_dir->add_sorted(buf, blocks, type);
		diremu_add_entry(buf, blocks, type);

		delete buf;
	}
	closedir(d);
}

FileSystemUNIX::~FileSystemUNIX(void)
{
}

FileSystem * FileSystemUNIX::check(File *file)
{
	if (file->is_directory()) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemUNIX::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemUNIX(file);
	}
#ifdef DEBUG_CHECK
	*debug << "FileSystemUNIX::check(): failed!\n";
#endif /* DEBUG_CHECK */
	return NULL;
}

#endif /* LINUX */

