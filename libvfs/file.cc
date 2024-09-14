/*
 *  $Id: file.cc,v 1.6 1998/10/26 03:02:09 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <unistd.h>
#include <sys/stat.h>

#include "misc/util.h"
#include "misc/debug.h"
#include "misc/assert.h"
#include "libvfs/file.h"

#define DEBUG_LEVEL 0
#define _MODULE_ "File: "

File::File(const char *path, const char *name, file_mode_t mode)
{
	int len;

#if DEBUG_LEVEL > 1
	*debug << _MODULE_ "CONSTRUCTOR File: " << this << endl;
#endif
	ASSERT(path != NULL);
	_path = util_strdup(path);
        len = strlen(_path);
        if (name == NULL) {
		_name = util_strdup("<noname>");
        } else {
		len += strlen(name);
		_name = util_strdup(name);
        }
        _buf = new char[len + 2];
        strcpy(_buf, _path);
        if (name != NULL) strcat(_buf, _name); /* yes, we check for name not _name! */

	_mode = mode;
	_size = 0;
	_ok = _dir = _reg = false;
}

File::~File(void)
{
	delete _path;
	delete _name;
	delete _buf;
#if DEBUG_LEVEL > 1
	*debug << _MODULE_ "DESTRUCTOR File: " << this << endl;
#endif
}

const char * File::dir(void)
{
	return _path;
}

const char * File::name(void)
{
	return _name;
}

const char * File::path(void)
{
	return _buf;
}

const char * File::realpath(void)
{
        return _buf;
}

long File::size(void)
{
	return _size;
}

bool File::ok(void)
{
	return _ok;
}

bool File::is_regular(void)
{
	return _reg;
}

bool File::is_directory(void)
{
	return _dir;
}

FileDOS::FileDOS(const char *path, const char *name, file_mode_t mode) :
		File(path, name, mode)
{
#ifdef MSDOS
	int a;
        char *tmp;
#endif
	struct stat statbuf;
	int len;

#if DEBUG_LEVEL > 1
	*debug << _MODULE_ "CONSTRUCTOR FileDOS: " << this << endl;
#endif

	_realpath = NULL;
#ifdef MSDOS
	/*
	 *  convert / to \
	 */
	len = strlen(_buf);
        for (a = 0;a < len;a++) {
        	if (_buf[a] == '/') _buf[a] = '\\';
        }
#endif /* MSDOS */

#ifdef MSDOS
	/*
	 *  check for c: -> c:\
	 */
	tmp = strchr(_buf, ':');
	ASSERT(tmp != NULL);
	if (((tmp - _buf) + 1) == len) {
		strcat(_buf, "\\");
	}
#endif /* MSDOS */

	_f = NULL;
	_size = 0;
        _ok = _dir = _reg = false;

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "open:  " << _buf << endl;
#endif
        if (stat(_buf, &statbuf) == 0) {
                if (S_ISDIR(statbuf.st_mode)) _ok = _dir = true;
                if (S_ISREG(statbuf.st_mode)) _ok = _reg = true;
		_size = statbuf.st_size;
	}

	if (_dir) return;

	switch (mode) {
	case File_RO:
	case File_RDWR:
	case File_C_RO:
	case File_C_WR:
	case File_C_RDWR:
		break;
	default:
		/*
		 *  don't overwrite
		 */
		if (_ok) {
			_ok = false;
			return;
		}
		break;
	}

        switch (mode) {
        case File_RO:
	case File_C_RO:
        	_mode_str = "rb";
        	break;
        case File_WR:
	case File_C_WR:
        	_mode_str = "wb";
        	break;
        case File_RDWR:
        	_mode_str = "rb+";
        	break;
	case File_C_RDWR:
        	_mode_str = "wb+";
        	break;
	case File_NONE:
		INTERNAL_ERROR("File_NONE not allowed here");
		break;
        }

        if ((_f = fopen(_buf, _mode_str)) == NULL) {
                _ok = _reg = false;
        } else {
                _ok = _reg = true;
                // if (mode == File_RO) check_gzip();
                check_gzip(); /* FIXME: */
        }
}

FileDOS::~FileDOS(void)
{
#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "close: " << _buf << endl;
#endif
	if (_f != NULL) fclose(_f);
        if (_realpath != NULL) {
#if DEBUG_LEVEL > 0
		*debug << _MODULE_ "GZIP del:   " << _realpath << endl;
#endif
                unlink(_realpath);
        	delete _realpath;
        }
#if DEBUG_LEVEL > 1
	*debug << _MODULE_ "DESTRUCTOR FileDOS: " << this << endl;
#endif

}

void FileDOS::check_gzip(void)
{
	int a;
        char buf[2];
        char cmd[1024];
        char tmp_name[L_tmpnam];
	struct stat statbuf;

        a = fread(buf, 1, 2, _f);
        ::rewind(_f);

        if (a != 2) return;
	if (strncmp(buf, "\x1f\x8b", 2) != 0) return;

       	fclose(_f);

	tmpnam(tmp_name);
#ifdef MSDOS
	sprintf(cmd, "gzip -dc \"%s\" > %s", _buf, tmp_name);
	if (util_system(cmd, true) == 0) {
#endif /* MSDOS */
#ifdef LINUX
        if (util_exec_wait("gzip", true, tmp_name, "-dc", _buf, NULL) == 0) {
#endif /* LINUX */
	        if (stat(tmp_name, &statbuf) != 0) {
			_ok = _reg = false;
                        return;
                }
		_size = statbuf.st_size;
                if ((_f = fopen(tmp_name, "rb")) != NULL) {
                	_realpath = new char [strlen(tmp_name) + 1];
                        strcpy(_realpath, tmp_name);
#if DEBUG_LEVEL > 0
			*debug << _MODULE_ "GZIP ok (name: " << _realpath
 			       << ")" << endl;
#endif
	                return;
                }
        }

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "GZIP failed" << endl;
#endif
        _ok = false;
        _reg = false;
}

void FileDOS::rewind(void)
{
	struct stat statbuf;

	if (_f != NULL) {
		::rewind(_f);
		fstat(fileno(_f), &statbuf);
		_size = statbuf.st_size;
	}
}

bool FileDOS::seek(long offset)
{
	ASSERT(_f != NULL);
	ASSERT(offset >= 0);
	if (fseek(_f, offset, SEEK_SET) == 0) return true;
	return false;
}

long FileDOS::read(void *buf, long len)
{
	ASSERT(_f != NULL);
	ASSERT(buf != NULL);
	ASSERT(len > 0);
	return fread(buf, 1, len, _f);
}

long FileDOS::write(void *buf, long len)
{
	ASSERT(_f != NULL);
        ASSERT(buf != NULL);
        ASSERT(len > 0);
        return ::fwrite(buf, 1, len, _f);
}

int FileDOS::getc(void)
{
	ASSERT(_f != NULL);
	return ::fgetc(_f);
}

int FileDOS::putc(int c)
{
	ASSERT(_f != NULL);
	return ::fputc(c, _f);
}

char * FileDOS::fgets(void *buf, long maxlen)
{
	ASSERT(_f != NULL);
        ASSERT(buf != NULL);
        ASSERT(maxlen > 0);
        return ::fgets((char *)buf, maxlen, _f);
}

const char * FileDOS::realpath(void)
{
	if (_realpath) return _realpath;
        return _buf;
}

FileDOS_Tmp::FileDOS_Tmp(const char *path, const char *name,
                         file_mode_t mode)
        : FileDOS(path, name, mode)
{
#if DEBUG_LEVEL > 1
	*debug << _MODULE_ "CONSTRUCTOR FileDOS_Tmp: " << this << endl;
#endif
}

FileDOS_Tmp::~FileDOS_Tmp(void)
{
#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "del:   " << _buf << endl;
#endif
	if (_f != NULL) {
        	fclose(_f);
		unlink(_buf);
                _f = NULL;
        }
#if DEBUG_LEVEL > 1
	*debug << _MODULE_ "DESTRUCTOR FileDOS_Tmp: " << this << endl;
#endif
}

