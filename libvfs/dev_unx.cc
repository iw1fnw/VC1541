/*
 *  $Id: dev_unx.cc,v 1.1 1998/10/26 03:02:08 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifdef LINUX

#include <iostream>
#include <iomanip>

#include "misc/debug.h"
#include "libvfs/dev_unx.h"

FileSystemDevUNIX::FileSystemDevUNIX(void) : FileSystem(NULL, "UNIX Device")
{
	diremu_init();
	_dir->set_title("UNIX Devices", 0);
	diremu_set_title("UNIX Devices", 0);
	_dir->add("/", 0, " DEV ");
	diremu_add_entry("/", 0, " DEV ");
}

FileSystemDevUNIX::~FileSystemDevUNIX(void)
{
}

#endif /* LINUX */
