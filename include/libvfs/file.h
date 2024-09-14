/*
 *  $Id: file.h,v 1.4 1998/10/26 03:03:05 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_file_h
#define __libvfs_file_h

#include <stdio.h>
#include <string.h>

#undef getc
#undef putc

/* #define DEBUG_FILE */

typedef enum {
	File_NONE,
	File_RO,
	File_WR,
	File_RDWR,
	File_C_RO,
	File_C_WR,
	File_C_RDWR
} file_mode_t;

class File
{
protected:
	char *_path;
	char *_name;
	char *_buf;
	file_mode_t _mode;
	long _size;
	bool _ok, _dir, _reg;

public:
	File(const char *path, const char *name = NULL,
             file_mode_t mode = File_RO);
        virtual ~File(void);

	virtual void   rewind(void)			= 0;
	virtual bool   seek(long offset)		= 0;
	virtual long   read(void *buf, long len)	= 0;
        virtual long   write(void *buf, long len)	= 0;
	virtual int    getc(void)			= 0;
	virtual int    putc(int c)			= 0;
        virtual char * fgets(void *buf, long maxlen)	= 0;

	virtual const char * dir(void);
        virtual const char * name(void);
	virtual const char * path(void);
	virtual const char * realpath(void);
	virtual long size(void);
        virtual bool ok(void);
        virtual bool is_regular(void);
        virtual bool is_directory(void);
};

class FileDOS : public File
{
protected:
	FILE *_f;
	char *_mode_str;
	char *_realpath;

	void check_gzip(void);
public:
	FileDOS(const char *path, const char *name = NULL,
                file_mode_t mode = File_RO);
        virtual ~FileDOS(void);

	virtual void   rewind(void);
	virtual bool   seek(long offset);
	virtual long   read(void *buf, long len);
        virtual long   write(void *buf, long len);
	virtual int    getc(void);
	virtual int    putc(int c);
        virtual char * fgets(void *buf, long maxlen);

	virtual const char * realpath(void);
};

class FileDOS_Tmp : public FileDOS
{
public:
	FileDOS_Tmp(const char *path, const char *name = NULL,
                    file_mode_t mode = File_RO);
        virtual ~FileDOS_Tmp(void);
};

#endif /* __libvfs_file_h */

