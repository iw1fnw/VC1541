/*
 *  $Id: fs.cc,v 1.4 1998/10/26 03:02:10 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <string.h>

#include "misc/util.h"
#include "misc/assert.h"
#include "libvfs/fs.h"

#define DEBUG_LEVEL 0
#define _MODULE_ "FS [---]: "

FileSystem::FileSystem(File *file, const char *fstype)
{
	int a;

        _refcount = 0;
	_file = file;
	_dir = new Directory;
	_cur_idx = 0;
	_cur_dirent = NULL;
	_fstype = new char[strlen(fstype) + 1];
	strcpy(_fstype, fstype);
	_dir_len = 0;
	for (a = 0;a < 19;a++) _dir_sector[a] = NULL;
}

FileSystem::~FileSystem(void)
{
	int a;

        ASSERT(_refcount == 0);

	delete _dir;
	delete _fstype;
	delete _file;
	for (a = 0;a < 19;a++) {
		if (_dir_sector[a] != NULL) delete _dir_sector[a];
	}
}

void FileSystem::diremu_init(void)
{
	int a;

	_dir_len = 0;
	_dir_sector[0] = new byte_t[256];
	_dir_sector[1] = new byte_t[256];
	memset(_dir_sector[0], 0, 256);
	memset(_dir_sector[1], 0, 256);

	/*
	 *  setup sector 18/ 0
	 */
	_dir_sector[0][0] = 18;
	_dir_sector[0][1] = 1;
	_dir_sector[0][2] = 'A';
	_dir_sector[0][3] = 0;
	for (a = 0;a < 18;a++) {
		_dir_sector[0][0x90 + a] = 0xa0;
	}
	_dir_sector[0][0xa2] = 'I';
	_dir_sector[0][0xa3] = 'D';
	_dir_sector[0][0xa4] = 0xa0;
	_dir_sector[0][0xa5] = '2';
	_dir_sector[0][0xa6] = 'A';
	_dir_sector[0][0xa7] = 0xa0;
	_dir_sector[0][0xa8] = 0xa0;

	/*
	 *  setup sector 18/ 1
	 */
	_dir_sector[1][0] = 0;
	_dir_sector[1][1] = 0xff;
}

void FileSystem::diremu_set_title(const char *title, dword_t /* free */)
{
	int a;
	int state;

	state = 0;
	for (a = 0;a < 18;a++) {
		switch (state) {
		case 0:
			if (title[a] != '\0') {
				_dir_sector[0][0x90 + a] = title[a];
				break;
			} else {
				state = 1;
			}
			/* fall through */
		case 1:
			_dir_sector[0][0x90 + a] = 0xa0;
			break;
		default:
			INTERNAL_ERROR("FileSystem::diremu_set_title");
			break;
		}
	}
}

void FileSystem::diremu_add_entry(const char *name, dword_t /* blocks */,
                                  const char * /* type */)
{
	int a;
	int state;
	int s = _dir_len / 8 + 1;
	int o = (_dir_len % 8) * 32;
	static bool too_long = true;

	if (s > 18) {
		if (too_long) {
			*debug << _MODULE_ "directory too long for disk "
					   "emulation!" << endl;
		}
		too_long = false;
		return;
	}

	if (_dir_sector[s] == NULL) {
		_dir_sector[s] = new byte_t[256];
		memset(_dir_sector[s], 0, 256);
		_dir_sector[s][0] = 0;
		_dir_sector[s][1] = 0xff;
		_dir_sector[s - 1][0] = 18;
		_dir_sector[s - 1][1] = s;
	}

	_dir_sector[s][o + 2] = 0x82;
	_dir_sector[s][o + 3] = 0;
	_dir_sector[s][o + 4] = _dir_len;

	state = 0;
	for (a = 0;a < 16;a++) {
		switch (state) {
		case 0:
			if (name[a] != '\0') {
				_dir_sector[s][o + a + 5] = name[a];
				break;
			} else {
				state = 1;
			}
			/* fall through */
		case 1:
			_dir_sector[s][o + a + 5] = 0xa0;
			break;
		default:
			INTERNAL_ERROR("FileSystem::diremu_add_entry");
			break;
		}
	}
	_dir_len++;
}

Directory * FileSystem::get_dir(void)
{
	return _dir;
}

const char * FileSystem::get_path(void)
{
	if (_cur_dirent == NULL) return NULL;
        return util_strdup(_cur_dirent->name());
}

File * FileSystem::file(void)
{
	return _file;
}

const char * FileSystem::fstype(void)
{
	return _fstype;
}

int FileSystem::chdir(DirectoryEntry *e, int idx)
{
	int tmp = _cur_idx;

	_cur_idx = idx;
	_cur_dirent = e;

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ << "chdir: " << (e ? e->name() : "(null)")
	       << " " << idx << " -> " << tmp << endl;
#endif

	return tmp;
}

/*
 *  generally we don't need internal chdir handling
 *  this is mainly useful for archivers that can handle directory trees
 */
FileSystem * FileSystem::chdir_internal(DirectoryEntry *e, int idx)
{
#if DEBUG_LEVEL > 0
	*debug << _MODULE_ << "chdir_internal: " << (e ? e->name() : "(null)")
	       << " " << idx << " -> (null)" << endl;
#endif
	return NULL;
}

int FileSystem::chdir_internal_up(void)
{
        return -1;
}

File * FileSystem::open(const char *path, DirectoryEntry *e)
{
	return new FileDOS(path, e->name());
}

File * FileSystem::open_write(const char * /* path */, const char * /* name */)
{
	return NULL;
}

bool FileSystem::command_supported(DeviceCommand /* cmd */)
{
	return false;
}

char * FileSystem::command(DeviceCommand /* cmd */, char * /* arg */, int /* arglen */)
{
	return NULL;
}

bool FileSystem::read_sector(byte_t *buf, int track, int sector)
{
	bool ok = true;

	if (track != 18) ok = false;
	if ((sector < 0) || (sector > 18)) ok = false;
	if (_dir_sector[sector] == NULL) ok = false;

	if (ok) {
		*debug << _MODULE_ "diremu: read sector " << track << "/" << sector << endl;
		memcpy(buf, _dir_sector[sector], 256);
		return true;
	}
	*debug << _MODULE_ "diremu: can't read sector " << track << "/" << sector << endl;
	return false;
}

bool FileSystem::write_sector(byte_t * /* buf */, int /* track */, int /* sector */)
{
	return false;
}

