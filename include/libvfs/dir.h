/*
 *  $Id: dir.h,v 1.5 1998/10/26 03:03:04 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_dir_h
#define __libvfs_dir_h

#include <stdio.h>
#include <string.h>

#include <list.h>

#include "misc/debug.h"
#include "libvfs/common.h"

class DirectoryEntry
{
protected:
	int             _idx;
	char           *_name;
	char           *_type;
	dword_t         _blocks;

public:
	DirectoryEntry(const char *name, dword_t blocks,
                       const char *type, int idx)
	{
		// fprintf(stderr, "+ DirectoryEntry::DirectoryEntry()\n");
		_idx = idx;
		_name = new char[strlen(name) + 1];
		_type = new char[strlen(type) + 1];
		strcpy(_name, name);
		strcpy(_type, type);
		_blocks = blocks;
	}

	~DirectoryEntry(void)
	{
		delete _name;
		delete _type;
		// fprintf(stderr, "- DirectoryEntry::~DirectoryEntry()\n");
	}

	const char * name(void) { return _name; }
	const char * type(void) { return _type; }
	dword_t blocks(void) { return _blocks; }
	int    index(void) { return _idx; }
};

class Directory : public list<DirectoryEntry *>
{
private:
	char *_title;
	dword_t _free;

protected:

public:
	Directory(void)
	{
		// fprintf(stderr, "+ Directory::Directory()\n");
		_free = 0;
		_title = new char[1];
		_title[0] = '\0';
	}

	virtual ~Directory(void)
	{
		delete _title;
		while (!empty()) {
			iterator it = begin();
			delete (*it);
			erase(it);
		}
		// fprintf(stderr, "- Directory::~Directory()\n");
	}

	void set_title(const char *title, dword_t free)
	{
		if (_title != NULL) delete _title;
		_title = new char[strlen(title) + 1];
		strcpy(_title, title);
		_free = free;
	}

	const char * title(void) { return _title; }
	dword_t free(void) { return _free; }

        void clear(void) {
                erase(begin(), end());
        }
        
	void add(const char *name, dword_t blocks, const char *type)
	{
		insert(end(), new DirectoryEntry(name, blocks, type, 0));
	}

	void add_sorted(const char *name, dword_t blocks, const char *type)
	{
		// *debug << endl;
		// *debug << "add + : " << name << ", " << type << endl;
		iterator it = begin();
		while (1) {
			if (it == end()) break;
			// *debug << "add - : " << (*it)->name() << ", " << (*it)->type() << endl;
			if (strcmp(type, (*it)->type()) < 0) break;
			if (strcmp(type, (*it)->type()) == 0) {
				if (strcmp(name, (*it)->name()) < 0) break;
			}
			it++;
		}
		// *debug << "*" << endl;
		insert(it, new DirectoryEntry(name, blocks, type, 0));

		for (it = begin();it != end();it++) {
			// *debug << "add * : " << (*it)->name() << ", " << (*it)->type() << endl;
		}
	}
};

#endif /* __libvfs_dir_h */

