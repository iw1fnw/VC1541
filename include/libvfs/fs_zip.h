/*
 *  $Id: fs_zip.h,v 1.3 1998/10/26 03:03:17 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_zip_h
#define __libvfs_fs_zip_h

#include <list.h>
#include <stack.h>

#include "libvfs/fs_exec.h"


#include "misc/util.h"
class dirbuf
{
private:
        char *_val;
        long _len;
        dirbuf *_next;
        dirbuf *_last;
        
public:
        dirbuf(char *name = NULL, long len = 0) {
                _val = util_strdup(name);
                _len = len;
                _next = NULL;
                _last = this;
        }

        ~dirbuf(void) {
                if (_val != NULL) delete _val;
        }

        void add(char *name, long len) {
                if (_val == NULL) {
                        _val = util_strdup(name);
                        _len = len;
                } else {
                        _last->_next = new dirbuf(name, len);
                        _last = _last->_next;
                }
        }

        char * value(void) { return _val; }
        long length(void) { return _len; }
        dirbuf * next(void) { return _next; }
};

class FileSystemZIP : public FileSystemEXEC
{
private:
        class X {
                typedef char * value_type;
                typedef int size_type;
        };
        
        dirbuf *_db;
	bool _is_external;

#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 8)
        stack < list <char *> > _internal_path;
        stack < list <int> > _internal_idx;
#else
	/*
	 *  this is for GCC 2.8.0 and above; seems that there were
	 *  some changes in the STL headers too :-(
	 *  the code looks cleaner now, through...
	 */
	stack <char *> _internal_path;
	stack <int> _internal_idx;
#endif
        virtual void parse(const char *filename);
        virtual void update(void);

public:
        FileSystemZIP(File *file);
        FileSystemZIP(File *file, const char *internal_path);
        virtual ~FileSystemZIP(void);

	virtual const char * get_path(void);
	virtual int chdir(DirectoryEntry *e, int idx);
	virtual FileSystem * chdir_internal(DirectoryEntry *e, int idx);
        virtual int chdir_internal_up(void);
	virtual File * open(const char *path, DirectoryEntry *e);

	static FileSystem * check(File *file);
};

#endif /* __libvfs_fs_zip_h */

