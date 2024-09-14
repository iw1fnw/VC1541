/*
 *  $Id: dev_dos.cc,v 1.5 1998/10/26 03:02:07 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifdef MSDOS

#include <iostream.h>
#include <iomanip.h>

#include <dir.h>
#include <bios.h>

#include "misc/debug.h"
#include "libvfs/dev_dos.h"

FileSystemDevDOS::FileSystemDevDOS(void) : FileSystem(NULL, "DOS Device")
{
	int a, c;
        char buf[3];

	diremu_init();
	_dir->set_title("DOS Devices", 0);
	diremu_set_title("DOS Devices", 0);

        c = 0;
       	*debug << "- checking devices..." << flush;
        for (a = 0;a < 26;a++) {
        	if (checkDevice(a)) {
                	c++;
                	sprintf(buf, "%c:", 'A' + a);
			_dir->add(buf, 0, " DEV ");
			diremu_add_entry(buf, 0, " DEV ");
                        *debug << ' ' << (char)('A' + a) << flush;
                }
        }

        if (c == 0) {
        	*debug << " hmm, no device found!\n";
		return;
       	}
       	*debug << " ok, found " << c << " device"
             << ((c == 1) ? "" : "s") << ".\n" << flush;
}

FileSystemDevDOS::~FileSystemDevDOS(void)
{
}

int FileSystemDevDOS::checkDevice(int device)
{
	int a, curdisk;

	curdisk = getdisk();
	setdisk(device);
	a = (device == getdisk());
	setdisk(curdisk);
	if (!a) return 0;

	/*
	 *  special handling for a: and b:
	 */
	if ((device == 0) || (device == 1)) {
        	a = biosequip(); /* BIOS: get configuration word */
		if ((a & 1) == 0) return 0;
		a = (a >> 6) & 3;
		if (device > a) return 0;
	}

	return 1;
}

#endif /* MSDOS */
