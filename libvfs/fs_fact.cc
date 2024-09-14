/*
 *  $Id: fs_fact.cc,v 1.4 1998/10/26 03:02:16 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <malloc.h>

#include "misc/assert.h"
#include "libvfs/fs_fact.h"

#include "libvfs/fs_dos.h"
#include "libvfs/fs_unix.h"
#include "libvfs/fs_d64.h"
#include "libvfs/fs_t64.h"
#include "libvfs/fs_lnx.h"
#include "libvfs/fs_arj.h"
#include "libvfs/fs_lha.h"
#include "libvfs/fs_tar.h"
#include "libvfs/fs_zip.h"
#include "libvfs/fs_zoo.h"
#include "libvfs/fs_zipc.h"
#ifdef MSDOS
#include "libvfs/dev_dos.h"
#endif /* MSDOS */
#ifdef LINUX
#include "libvfs/dev_unx.h"
#endif /* LINUX */

FSFactory::FSFactory(void)
{
#ifdef MSDOS
	register_fs(FileSystemDOS::check);
#endif /* MSDOS */
#ifdef LINUX
	register_fs(FileSystemUNIX::check);
#endif /* LINUX */
	register_fs(FileSystemT64::check);
	register_fs(FileSystemLNX::check);
        register_fs(FileSystemARJ::check);
        register_fs(FileSystemLHA::check);
        register_fs(FileSystemTAR::check);
	register_fs(FileSystemZIP::check);
	register_fs(FileSystemZOO::check);
	register_fs(FileSystemZIPC::check);
	register_fs(FileSystemD64::check); /* this should go last */
}

FSFactory::~FSFactory(void)
{
}

void FSFactory::register_fs(check_func f)
{
	_check.add(f);
}

FileSystem * FSFactory::get_fs(Device *dev, FileSystem *fs, DirectoryEntry *e)
{
        File *f;
	int a, size;
        const char *path;
        FileSystem *new_fs;

        if ((dev == NULL) && (fs == NULL) && (e == NULL)) {
#ifdef MSDOS
		return new FileSystemDevDOS();
#endif /* MSDOS */
#ifdef LINUX
		return new FileSystemDevUNIX();
#endif /* LINUX */
        }

        ASSERT(dev != NULL);
        ASSERT(fs != NULL);
        ASSERT(e != NULL);

        path = dev->path();
	f = fs->open(path, e);
	delete path;

       	if (f == NULL) return NULL;

	new_fs = NULL;
        size = _check.size();
	for (a = 0;a < size;a++) {
		f->rewind();
        	if ((new_fs = _check[a](f)) != NULL) {
			/*
			 *  do not delete f!
			 */
        		return new_fs;
		}
	}

        delete f;
        return NULL;
}
