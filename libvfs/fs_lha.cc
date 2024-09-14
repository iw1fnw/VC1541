/*
 *  $Id: fs_lha.cc,v 1.6 1998/10/26 03:02:17 tp Exp $
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
#include "libvfs/fs_lha.h"

void FileSystemLHA::parse(const char *filename)
{
	int a;
	File *f;
        int state;
        int blocks;
	char buf[1024];
#ifdef LINUX
	char *p;
#endif /* LINUX */

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
#ifdef MSDOS
                        buf[14] = '\0';
                        buf[26] = '\0';
                        for (a = 13;a > 0;a--) {
                        	if (buf[a] == ' ') {
                        		buf[a] = '\0';
                                        break;
                                }
                        }
                        blocks = atol(buf + 15);
                        if (blocks > 0) blocks = blocks / 254 + 1;
			_dir->add_sorted(buf + 2, blocks, " PRG ");
			diremu_add_entry(buf + 2, blocks, " PRG ");
#endif /* MSDOS */
#ifdef LINUX
			if ((p = strchr(buf, '\n')) != NULL) *p = '\0';
			if ((p = strchr(buf, '\r')) != NULL) *p = '\0';
			buf[17] = '\0';
			buf[25] = '\0';
			blocks = atol(buf + 18);
			if (blocks > 0) blocks = blocks / 254 + 1;
			_dir->add_sorted(buf + 46, blocks, " PRG ");
			diremu_add_entry(buf + 46, blocks, " PRG ");
#endif /* LINUX */
                }
        }
}

FileSystemLHA::FileSystemLHA(File *file) :
 		FileSystemEXEC(file, "External Archiver (LHA)")
{
	char buf[1024];
        char tmp_name[L_tmpnam];

	tmpnam(tmp_name);

#ifdef MSDOS
       	sprintf(buf,
               	"lha l %s > %s",
       		file->realpath(), tmp_name);
        util_system(buf, false);
#endif /* MSDOS */
#ifdef LINUX
        util_exec_wait("lha", false, tmp_name, "l", file->realpath(), NULL);
#endif /* LINUX */

        parse(tmp_name);
}


FileSystemLHA::~FileSystemLHA()
{
}

File * FileSystemLHA::open(const char * /*path*/, DirectoryEntry *e)
{
	char buf[1024];
        char tmp_name[L_tmpnam];

	tmpnam(tmp_name);

#ifdef MSDOS
       	sprintf(buf, "lha p /m /n %s %s > %s",
               	_file->realpath(), e->name(), tmp_name);
        util_system(buf, false);
#endif /* MSDOS */

#ifdef LINUX
        util_exec_wait("lha", false, tmp_name, "-pq", _file->realpath(),
                       e->name(), NULL);
#endif /* LINUX */


        return new FileDOS_Tmp(tmp_name);
}

FileSystem * FileSystemLHA::check(File *file)
{
        if (check_type_exec(file, 2, 3, "-lh")) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemLHA::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemLHA(file);
        }
#ifdef DEBUG_CHECK
	*debug << "FileSystemLHA::check(): failed!\n";
#endif /* DEBUG_CHECK */
        return NULL;
}

