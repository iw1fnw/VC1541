/*
 *  $Id: fs_dos.cc,v 1.4 1998/10/26 03:02:13 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifdef MSDOS

#include <ctype.h>
#include <dirent.h>
#include <sys/vfs.h>

#include "misc/debug.h"
#include "libvfs/fs_dos.h"

#define _MODULE_ "FS [DOS]: "

FileSystemDOS::FileSystemDOS(File *file) : FileSystem(file, "DOS Directory")
{
	init(file);
}

void FileSystemDOS::init(File *file)
{
	DIR *d;
	struct stat st;
        struct statfs stfs;
	const char *path;
	char *type, *buf;
	struct dirent *ent;
	int a, len, blocks;
	char p[1024];

	path = file->path();
        if (statfs(path, &stfs) != 0) return;
	if ((d = opendir(path)) == NULL) return;

	diremu_init();

	_dir->clear();
	_dir->set_title(file->name(),
			(stfs.f_bfree * stfs.f_bsize) / 254);
	diremu_set_title(file->name(),
			(stfs.f_bfree * stfs.f_bsize) / 254);

	while ((ent = readdir(d)) != NULL) {
        	if (strcmp(ent->d_name, ".") == 0) continue;
                if (strcmp(ent->d_name, "..") == 0) continue;
		a = strlen(path);
		if (a > 0 && path[a - 1] == '\\') {
			sprintf(p, "%s%s", path, ent->d_name);
		} else {
			sprintf(p, "%s\\%s", path, ent->d_name);
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
		for (a = 0;a < len;a++) {
			buf[a] = toupper(ent->d_name[a]);
		}
		buf[a] = '\0';
		_dir->add_sorted(buf, blocks, type);
		diremu_add_entry(buf, blocks, type);
		delete buf;
	}
	closedir(d);
}

FileSystem * FileSystemDOS::check(File *file)
{
	if (file->is_directory()) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemDOS::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemDOS(file);
	}
#ifdef DEBUG_CHECK
	*debug << "FileSystemDOS::check(): failed!\n";
#endif /* DEBUG_CHECK */
	return NULL;
}

File * FileSystemDOS::open_write(const char *path, const char *name)
{
	return new FileDOS(path, name, File_WR);
}

File * FileSystemDOS::open(const char *path, DirectoryEntry *e)
{
	File *f;

	f = new FileDOS(path, e->name(), File_RDWR);
	if (!f->ok()) {
		*debug << _MODULE_ "can't open file read/write" << endl;
		delete f;
		f = new FileDOS(path, e->name(), File_RO);
	}
	return f;
}

void FileSystemDOS::update(void)
{
	*debug << "FileSystemDOS::update()" << endl;
	init(_file);
}

#endif /* MSDOS */

