/*
 *  $Id: f_d64.cc,v 1.1 1998/10/26 03:02:09 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include "misc/debug.h"
#include "misc/assert.h"
#include "libvfs/f_d64.h"
#include "libvfs/fs_d64.h"
#include "libvfs/common.h"

#define _MODULE_ "FileD64: "

#undef INTERNAL_ERROR
#define INTERNAL_ERROR(x)

FileD64::FileD64(FileSystemD64 *fs,
		 track_sector_t ts,
		 track_sector_t ts_dir,
		 int dir_slot,
		 const char *path,
		 const char *name = NULL,
		 file_mode_t mode = File_RO)
		 : File(path, name, mode)
{
	_fs = fs;
	_mode = mode;
	_ts = ts;
	_ts_dir = ts_dir;
	_dir_slot = dir_slot;

	_ok = true;
	_reg = true;

	*debug << _MODULE_ "CONSTRUCTOR" << endl;

	switch (_mode) {
	case File_RO:
		INTERNAL_ERROR("mode == File_RO");
		break;
	case File_WR:
		ASSERT(name != NULL);
		break;
	case File_RDWR:
		INTERNAL_ERROR("mode == File_RDWR");
		break;
	default:
		INTERNAL_ERROR("wrong mode");
		break;
	}

	_blocks = 1;
	_buf = new byte_t[256];
	_count = 2;
	memset(_buf, '\0', 256);
}

FileD64::~FileD64(void)
{
	_buf[0] = 0;
	_buf[1] = _count - 2;
	_fs->write_sector(_buf, _ts.track, _ts.sector);
	_fs->close_dir_slot(_ts_dir, _dir_slot, _blocks);
	delete _buf;

	*debug << _MODULE_ "DESTRUCTOR" << endl;
}

void FileD64::rewind(void)
{
	INTERNAL_ERROR("rewind");
}

bool FileD64::seek(long /* offset */)
{
	INTERNAL_ERROR("seek");
	return false;
}

long FileD64::read(void * /* buf */, long /* len */)
{
	INTERNAL_ERROR("read");
	return 0;
}

long FileD64::write(void * /* buf */, long /* len */)
{
	INTERNAL_ERROR("write");
	return 0;
}

int FileD64::getc(void)
{
	INTERNAL_ERROR("getc");
	return -1;
}

int FileD64::putc(int c)
{
	track_sector_t ts;

	if (_count < 256) {
		_buf[_count++] = c;
	} else {
		_blocks++;
		ts = _fs->allocate_block();
		if (IS_INVALID_TRACK_SECTOR(ts)) {
			return -1;
		}
		_buf[0] = ts.track;
		_buf[1] = ts.sector;
		_fs->write_sector(_buf, _ts.track, _ts.sector);
		_ts = ts;
		_count = 2;
		memset(_buf, '\0', 256);
		_buf[_count++] = c;
	}
	return c;
}

char * FileD64::fgets(void * /* buf */, long /* maxlen */)
{
	INTERNAL_ERROR("fgets");
	return NULL;
}

