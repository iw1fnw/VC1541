/*
 *  $Id: fs_arj.cc,v 1.6 1998/10/26 03:02:11 tp Exp $
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
#include "libvfs/fs_arj.h"

void FileSystemARJ::parse(const char *filename)
{
	int a;
	File *f;
        int state;
        int blocks;
	char buf[1024];

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
                        buf[12] = '\0';
                        buf[23] = '\0';
                        for (a = 11;a > 0;a--) {
                        	if (buf[a] == ' ') {
                        		buf[a] = '\0';
                                        break;
                                }
                        }
                        blocks = atol(buf + 13);
                        if (blocks > 0) blocks = blocks / 254 + 1;
			_dir->add_sorted(buf, blocks, " PRG ");
			diremu_add_entry(buf, blocks, " PRG ");
                }
        }
}

FileSystemARJ::FileSystemARJ(File *file) :
 		FileSystemEXEC(file, "External Archiver (ARJ)")
{
	char buf[1024];
        char tmp_name[L_tmpnam];

	tmpnam(tmp_name);

#ifdef MSDOS
       	sprintf(buf,
		"arj l %s > %s",
		file->realpath(), tmp_name);
        util_system(buf, true);
#endif /* MSDOS */
#ifdef LINUX
        util_exec_wait("unarj", true, tmp_name, "l", file->realpath(), NULL);
#endif /* LINUX */
	parse(tmp_name);
}

FileSystemARJ::~FileSystemARJ(void)
{
}

File * FileSystemARJ::open(const char * /*path*/, DirectoryEntry *e)
{
	char buf[1024];
        char tmp_name[L_tmpnam];

#ifdef LINUX
	return NULL; /* fixme: implement arj decompression for linux */
#endif /* LINUX */

	tmpnam(tmp_name);

#ifdef MSDOS
       	sprintf(buf, "arj p -ja1 -i %s %s > %s",
               	_file->realpath(), e->name(), tmp_name);
        util_system(buf, true);
#endif /* MSDOS */


        return new FileDOS_Tmp(tmp_name);
}

FileSystem * FileSystemARJ::check(File *file)
{
        if (check_type_exec(file, 0, 2, "\x60\xea")) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemARJ::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemARJ(file);
        }
#ifdef DEBUG_CHECK
	*debug << "FileSystemARJ::check(): failed!\n";
#endif /* DEBUG_CHECK */
        return NULL;
}
