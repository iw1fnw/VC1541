/*
 *  $Id: fs_tar.cc,v 1.6 1998/10/26 03:02:20 tp Exp $
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
#include "libvfs/fs_tar.h"

void FileSystemTAR::parse(const char *filename)
{
	File *f;
	char *p, buf[1024];

	f = new FileDOS_Tmp(filename);
        if (!f->ok()) {
        	delete f;
                return;
        }

	diremu_init();
	_dir->set_title(f->name(), 0);
	diremu_set_title(f->name(), 0);

	while (1) {
		if (f->fgets(buf, 1024) == NULL) {
                        delete f;
			return;
                }
                if ((p = strchr(buf, 0x0d))) *p = '\0';
                if ((p = strchr(buf, 0x0a))) *p = '\0';
		_dir->add_sorted(buf, 0, " PRG ");
		diremu_add_entry(buf, 0, " PRG ");
        }
}

FileSystemTAR::FileSystemTAR(File *file) :
 		FileSystemEXEC(file, "External Archiver (TAR)")
{
	char buf[1024];
        char tmp_name[L_tmpnam];

	tmpnam(tmp_name);

#ifdef MSDOS
       	sprintf(buf,
               	"gtar tf %s > %s",
       		file->realpath(), tmp_name);
	util_system(buf, true);
#endif /* MSDOS */
#ifdef LINUX
        util_exec_wait("tar", true, tmp_name, "tf", file->realpath(), NULL);
#endif /* LINUX */

	parse(tmp_name);
}

FileSystemTAR::~FileSystemTAR(void)
{
}

File * FileSystemTAR::open(const char * /*path*/, DirectoryEntry *e)
{
	char buf[1024];
        char tmp_name[L_tmpnam];

	tmpnam(tmp_name);

#ifdef MSDOS
       	sprintf(buf, "tar xOf %s %s > %s",
               	_file->realpath(), e->name(), tmp_name);
        util_system(buf, true);
#endif /* MSDOS */

#ifdef LINUX
        util_exec_wait("tar", true, tmp_name, "xOf", _file->realpath(),
                       e->name(), NULL);
#endif /* LINUX */

        return new FileDOS_Tmp(tmp_name);
}

FileSystem * FileSystemTAR::check(File *file)
{
        if (check_type_exec(file, 257, 5, "ustar")) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemTAR::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemTAR(file);
        }
#ifdef DEBUG_CHECK
	*debug << "FileSystemTAR::check(): failed!\n";
#endif /* DEBUG_CHECK */
        return NULL;
}

