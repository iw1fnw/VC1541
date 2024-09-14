/*
 *  $Id: cable.h,v 1.1 1998/10/26 03:03:30 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __vc1541_cable_h
#define __vc1541_cable_h

#include "gui/gui.h"
#include "vc1541/lpt.h"

typedef enum {
	CABLE_NONE, CABLE_X1541, CABLE_XE1541
} cable_t;

class Cable
{
public:
	static const int NR_OF_CABLES = 5;

protected:
	GUI *_gui;
	LPT *_lpt;
	struct {
		int base;
		cable_t type;
		char *name;
	} _cable[NR_OF_CABLES];
	int _available_cables;

	int checkBit(int base, int bit, bool invert);
	int detectX1541(int base);
	int detectXE1541(int base);
public:
	Cable(GUI *gui, LPT *lpt);
	~Cable(void) {}
	int detect(int bios_nr, const char *cable_type);
	cable_t get_type(int base);
};

#endif /* __vc1541_cable_h */
