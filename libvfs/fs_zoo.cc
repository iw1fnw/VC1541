/*
 *  $Id: fs_zoo.cc,v 1.1 1998/10/26 03:02:24 tp Exp $
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
#include "libvfs/fs_zoo.h"

void FileSystemZOO::parse(const char *filename)
{
	File *f;
        int state;
        int blocks;
	char buf[1024];
	char *p;

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
                		return;
                        }
			if ((p = strchr(buf, '\n')) != NULL) *p = '\0';
			if ((p = strchr(buf, '\r')) != NULL) *p = '\0';
			buf[8] = '\0';
			blocks = atol(buf);
			if (blocks > 0) blocks = blocks / 254 + 1;
#ifdef MSDOS
			_dir->add_sorted(buf + 45, blocks, " PRG ");
			diremu_add_entry(buf + 45, blocks, " PRG ");
#endif /* MSDOS */
#ifdef LINUX
			_dir->add_sorted(buf + 48, blocks, " PRG ");
			diremu_add_entry(buf + 48, blocks, " PRG ");
#endif /* LINUX */
                }
        }
}

FileSystemZOO::FileSystemZOO(File *file) :
 		FileSystemEXEC(file, "External Archiver (ZOO)")
{
	char buf[1024];
        char tmp_name[L_tmpnam];

	tmpnam(tmp_name);

#ifdef MSDOS
       	sprintf(buf,
               	"zoo l %s > %s",
       		file->realpath(), tmp_name);

        util_system(buf, true);
#endif /* MSDOS */
#ifdef LINUX
        util_exec_wait("zoo", true, tmp_name, "l", file->realpath(), NULL);
#endif /* LINUX */

        parse(tmp_name);
}


FileSystemZOO::~FileSystemZOO()
{
}

File * FileSystemZOO::open(const char * /*path*/, DirectoryEntry *e)
{
	char buf[1024];
        char tmp_name[L_tmpnam];

	tmpnam(tmp_name);

#ifdef MSDOS
       	sprintf(buf, "zoo xpq %s %s > %s",
               	_file->realpath(), e->name(), tmp_name);
        util_system(buf, true);
#endif /* MSDOS */

#ifdef LINUX
        util_exec_wait("zoo", true, tmp_name, "xpq", _file->realpath(),
                       e->name(), NULL);
#endif /* LINUX */

        return new FileDOS_Tmp(tmp_name);
}

FileSystem * FileSystemZOO::check(File *file)
{
        if (check_type_exec(file, 0, 17, "ZOO 2.10 Archive.")) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemZOO::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemZOO(file);
        }
#ifdef DEBUG_CHECK
	*debug << "FileSystemZOO::check(): failed!\n";
#endif /* DEBUG_CHECK */
        return NULL;
}

