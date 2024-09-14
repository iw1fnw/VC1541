/*
 *  $Id: fs_zip.cc,v 1.5 1998/10/26 03:02:22 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "misc/util.h"
#include "misc/debug.h"
#include "misc/assert.h"
#include "libvfs/fs_zip.h"

#define DEBUG_LEVEL 1
#define _MODULE_ "FS [ZIP]: "

void FileSystemZIP::parse(const char *filename)
{
        File *f;
        int state;
        int blocks;
	char *p, buf[1024];
	char *last = NULL;

	f = new FileDOS_Tmp(filename);
        if (!f->ok()) {
        	delete f;
                return;
        }

	diremu_init();
	_dir->set_title(f->name(), 0);
	diremu_set_title(f->name(), 0);

        state = 0;
	while (1) {
		if (f->fgets(buf, 1024) == NULL) {
                	delete f;
			return;
                }
                switch (state) {
                case 0:
                	if (buf[1] == '-') state = 1;
                        break;
		case 1:
                	if (buf[1] == '-') {
                                delete f;
				if (last != NULL) delete last;
                		return;
                        }
                        buf[7] = '\0';
                        blocks = atol(buf);
                        if (blocks > 0) blocks = blocks / 254 + 1;
                        if ((p = strchr(buf + 27, 0x0d))) *p = '\0';
                        if ((p = strchr(buf + 27, 0x0a))) *p = '\0';
                        _db->add(buf + 27, blocks);
/*
                        buf[7] = '\0';
                        blocks = atol(buf);
			type = " PRG ";
                        if ((p = strchr(buf + 27, 0x0d))) *p = '\0';
                        if ((p = strchr(buf + 27, 0x0a))) *p = '\0';
                        db.add(buf + 27);
                        if ((p = strchr(buf + 27, '/'))) {
                                blocks = 0;
                                type = " DIR ";
                                *p = '\0';
                        }
                        if (blocks > 0) blocks = blocks / 254 + 1;

			if ((last == NULL) || (strcmp(last, buf + 27) != 0)) {
				if (last != NULL) delete last;
				_dir->add_sorted(buf + 27, blocks, type);
				diremu_add_entry(buf + 27, blocks, type);
				last = new char[strlen(buf + 27) + 1];
				strcpy(last, buf + 27);
			}
                        */
                }
        }
}

void FileSystemZIP::update(void)
{
        long blocks;
        dirbuf *ptr = _db;
        char *p, *name, *last, *type;

        if (ptr->value() == NULL) return;

        _dir->clear();
        
        last = NULL;
        do {
                int l = 0;
                
                if (!_internal_path.empty()) {
                        l = strlen(_internal_path.top());
                        if (strncmp(_internal_path.top(), ptr->value(), l) != 0) {
                                ptr = ptr->next();
                                continue;
                        }
                }
                
                p = ptr->value() + l;
                while (*p == '/') p++;
                if (*p == '\0') {
                        ptr = ptr->next();
                        continue;
                }
                name = util_strdup(p);

                p = strchr(name, '/');
                if (!p) {
                        blocks = ptr->length();
                        type = " PRG ";
                } else {
                        *p = '\0';
                        blocks = 0;
                        type = " DIR ";
                }
                if ((last == NULL) || (strcmp(last, name) != 0)) {
                        _dir->add_sorted(name, blocks, type);
                        if (last != NULL) delete last;
                        last = name;
                } else {
                        delete name;
                }
                ptr = ptr->next();
        } while (ptr != NULL);
}

FileSystemZIP::FileSystemZIP(File *file) :
 		FileSystemEXEC(file, "External Archiver (ZIP)")
{
	char buf[1024];
        char tmp_name[L_tmpnam];

#if DEBUG_LEVEL > 9
        *debug << _MODULE_ << "CONSTRUCTOR" << endl;
#endif

	_is_external = false;
        _db = new dirbuf();

	tmpnam(tmp_name);

#ifdef MSDOS
        sprintf(buf,
		"unzip386 -l -U %s > %s",
		file->realpath(), tmp_name);
        util_system(buf, true);
#endif /* MSDOS */
#ifdef LINUX
        util_exec_wait("unzip", true, tmp_name, "-l", "-U", file->realpath(),
                       NULL);
#endif /* LINUX */

        parse(tmp_name);
        update();
}

FileSystemZIP::~FileSystemZIP(void)
{
        delete _db;
#if DEBUG_LEVEL > 9
        *debug << _MODULE_ << "DESTRUCTOR" << endl;
#endif
}

const char * FileSystemZIP::get_path(void)
{
	int len;
	char *buf;
	const char *p1, *p2;

	len = 0;
	p1 = NULL;
	p2 = NULL;
	if (!_internal_path.empty()) {
		p1 = _internal_path.top();
		len += strlen(p1);
	}
	if (_cur_dirent != NULL) {
		p2 = _cur_dirent->name();
		len += strlen(p2);
	}
	if (len == 0) return NULL;
	buf = new char[len + 2];
	if (p1 != NULL) {
		strcpy(buf, p1);
		if (p2 != NULL) {
			strcat(buf, "/");
			strcat(buf, p2);
		}
	} else if (p2 != NULL) {
		strcpy(buf, p2);
	} else {
		INTERNAL_ERROR("FileSystemZIP::get_path(void)");
	}

	return buf;
}

int FileSystemZIP::chdir(DirectoryEntry * e, int idx)
{
        int tmp = _cur_idx;

        _cur_idx = idx;
        if (_is_external) _cur_dirent = e;

#if DEBUG_LEVEL > 2
        *debug << _MODULE_ << "chdir: " << (e ? e->name() : "(null)")
               << " " << idx << " -> " << tmp << endl;
#endif

        return tmp;
}

int FileSystemZIP::chdir_internal_up(void)
{
        int idx;
        
        if (_internal_idx.empty()) return -1;
        
	_is_external = false;
        idx = _internal_idx.top();
        _internal_idx.pop();
	_cur_idx = 0;
	_cur_dirent = NULL;
        
        if (!_internal_path.empty()) {
                _internal_path.pop();
                dec_refcount();
        }
        
        update();
        
        return idx;
}

FileSystem * FileSystemZIP::chdir_internal(DirectoryEntry *e, int idx)
{
        char *buf;
        int a, len;

        /*
         *  find entry to change to and...
         */
        if (idx >= _dir->size()) return NULL;
        Directory::iterator it = _dir->begin();
        for (a = 0;a < idx;a++) it++;

        /*
         *  handle chdir internally
         */
        len = 2;
        len += strlen(e->name());
        if (!_internal_path.empty()) len += strlen(_internal_path.top());
        
        buf = new char[len];
        if (!_internal_path.empty()) {
                strcpy(buf, _internal_path.top());
                strcat(buf, "/");
                strcat(buf, e->name());
        } else {
                strcpy(buf, e->name());
        }

        /*
         *  ...check if it's an internal directory entry
         *
         *  FIXME: hmm, we could do better; should introduce symbolic
         *         filetypes instead of strings!
         */
        if (strcmp((*it)->type(), " DIR ") != 0) {
		_cur_idx = idx;
		_cur_dirent = e;
		_is_external = true;
		return NULL;
	}

#if DEBUG_LEVEL > 2
	*debug << _MODULE_ << "chdir_internal: " << buf << endl;
#endif

        _internal_path.push(buf);
        _internal_idx.push(idx);

        inc_refcount();

        update();
        
        return this;
}

File * FileSystemZIP::open(const char * /*path*/, DirectoryEntry *e)
{
	char buf[1024];
        char tmp_name[L_tmpnam];

	tmpnam(tmp_name);

#ifdef MSDOS
	if (_internal_path.empty()) {
	       	sprintf(buf, "unzip386 -p %s %s > %s",
			_file->realpath(),
			e->name(),
			tmp_name);
	} else {
	       	sprintf(buf, "unzip386 -p %s %s/%s > %s",
			_file->realpath(),
			_internal_path.top(),
			e->name(),
			tmp_name);
	}
        util_system(buf, true);
#endif /* MSDOS */
#ifdef LINUX
        if (_internal_path.empty()) {
                sprintf(buf, "%s", e->name());
        } else {
                sprintf(buf, "%s/%s", _internal_path.top(), e->name());
        }
        util_exec_wait("unzip", true, tmp_name, "-p", _file->realpath(),
                       buf, NULL);
#endif /* LINUX */


        return new FileDOS_Tmp(tmp_name);
}

FileSystem * FileSystemZIP::check(File *file)
{
        if (check_type_exec(file, 0, 2, "PK")) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemZIP::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemZIP(file);
        }
#ifdef DEBUG_CHECK
	*debug << "FileSystemZIP::check(): failed!\n";
#endif /* DEBUG_CHECK */
        return NULL;
}

