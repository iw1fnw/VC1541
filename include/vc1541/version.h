/*
 *  $Id: version.h,v 1.3 1998/10/26 03:03:34 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __vc1541_version_h
#define __vc1541_version_h

#define PROG_NAME			"VC 1541"
#define PROG_LONG_NAME			"VC 1541 Emulator"
#define PROG_COPYRIGHT			"Copyright (C) 1997,1998 Torsten Paul"
#define PROG_EMAIL			"paul@os.inf.tu-dresden.de"
#define PROG_VERSION_MAJOR		0
#define PROG_VERSION_MINOR		4
#define PROG_PATCHLEVEL                 6

#if PROG_PATCHLEVEL == 0
#define PROG_MK_VERSION2(min,maj,pl)	"V"#maj"."#min
#define PROG_MK_VERSION1(min,maj,pl)	PROG_MK_VERSION2(maj,min,pl)
#else
#define PROG_MK_VERSION2(min,maj,pl)	"V"#maj"."#min"pl"#pl
#define PROG_MK_VERSION1(min,maj,pl)	PROG_MK_VERSION2(maj,min,pl)
#endif
#define PROG_VERSION			PROG_MK_VERSION1(PROG_VERSION_MAJOR,\
					PROG_VERSION_MINOR,\
					PROG_PATCHLEVEL)

#endif /* __vc1541_version_h */
