/*
 *  $Id: cable.cc,v 1.1 1998/10/26 02:59:03 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifdef MSDOS

#include <pc.h>

#include "misc/util.h"
#include "misc/debug.h"
#include "misc/assert.h"
#include "vc1541/cable.h"

#define XE1541_EXTENDED_CHECK

#define DEBUG_LEVEL 0
#define _MODULE_ "Cable: "

#if 0
void outhex(int val)
{
	int a;

	debug->form("%02x ", val);
	for (a = 0;a < 8;a++) {
		debug->form("%c", (val & (128 >> a)) ? '1' : '0');
	}
	debug->form(" - ");
}
#endif

Cable::Cable(GUI *gui, LPT *lpt)
{
	int a;

	_gui = gui;
	_lpt = lpt;

	_available_cables = 0;
	for (a = 0;a < NR_OF_CABLES;a++) {
		_cable[a].base = 0;
		_cable[a].type = CABLE_NONE;
		_cable[a].name = NULL;
	}
}

int
Cable::detectX1541(int base)
{
	/*
	 *  this test is based on the connection between
	 *  DATA0 (pin 2) and ERROR (pin 15)
	 */
	outportb(base, 0x00);
	if (!(inportb(base + 1) & 0x08)) {
		// 1. test passed
		outportb(base, 0x01);
		if (inportb(base + 1) & 0x08) {
			// 2. test passed
			outportb(base, 0x00);
			if (!(inportb(base + 1) & 0x08)) {
				// 3. test passed
				return 1;
			}
		}
	}
	return 0;
}

int
Cable::checkBit(int base, int bit, bool invert)
{
	int a;
	int val;

	val = invert ? 0 : (bit << 4);
	a = inportb(base + 2);
	outportb(base + 2, a & ~bit);
	if (val == (inportb(base + 1) & (bit << 4))) {
		// 1. test passed
		outportb(base + 2, a | bit);
		if (val != (inportb(base + 1) & (bit << 4))) {
			// 2. test passed
			outportb(base + 2, a & ~bit);
			if (val == (inportb(base + 1) & (bit << 4))) {
				// 3. test passed
				outportb(base + 2, a);
				return 1;
			}
		}
	}
	outportb(base + 2, a);
	return 0;
}

int
Cable::detectXE1541(int base)
{
#ifdef XE1541_EXTENDED_CHECK
	int a;

	/*
	 *  first check for connection between
	 *  AUTOFEED (pin 14) and ERROR (pin 15)
	 */
	a = inportb(base + 2);
        outportb(base + 2, a & ~0x02);
	if (inportb(base + 1) & 0x08) {
		// 1. test passed
		outportb(base + 2, a | 0x02);
		if (!(inportb(base + 1) & 0x08)) {
			// 2. test passed
			outportb(base + 2, a & ~0x02);
			if (inportb(base + 1) & 0x08) {
				// 3. test passed
				outportb(base + 2, a);
				return 1;
			}
		}
	}
	outportb(base + 2, a);
#endif

	/*
  	 *  CLOCK:
	 *  check loopback from AUTOFEED (pin 14) to PAPEREND (pin 12)
	 */
#if DEBUG_LEVEL > 0
	*debug << _MODULE_ << "checking for CLOCK loopback" << endl;
#endif
	if (checkBit(base, 0x02, false)) return 1;

	/*
 	 *  DATA:
	 *  check loopback from SELECT (pin17) to BUSY (pin 11)
 	 */
#if DEBUG_LEVEL > 0
	*debug << _MODULE_ << "checking for DATA loopback" << endl;
#endif
	if (checkBit(base, 0x08, true)) return 1;

	/*
	 *  ATN:
	 *  check loopback from STROBE (pin 1) to SELECT (pin 13)
 	 */
#if DEBUG_LEVEL > 0
	*debug << _MODULE_ << "checking for ATN loopback" << endl;
#endif
	if (checkBit(base, 0x01, false)) return 1;

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ << "no loopback found" << endl;
#endif
	return 0;
}

cable_t
Cable::get_type(int base)
{
	int a;

	for (a = 0;a < _available_cables;a++) {
		if (base == _cable[a].base) {
			return _cable[a].type;
		}
	}
	return CABLE_NONE;
}

int
Cable::detect(int bios_nr, const char *cable_type)
{
	int a;
	int nr;
	int idx;
	int base;
	char buf[100];
	lpt_mode_t mode;

	_lpt->detect();

	*debug << "- detecting cables..." << endl;
	idx = 0;
	for (a = 0;;a++) {
		base = _lpt->get_base(a);
		if (base == 0) break;
		mode = _lpt->get_mode(a);
		if (mode == LPT_MODE_NONE) continue;
		nr = _lpt->get_bios_nr(base);
		debug->form("  %03xh - ", base);
		if (detectX1541(base)) {
			if (nr > 0) {
				sprintf(buf, "X1541 cable at %03xh [LPT%d]",
 					base, nr);
			} else {
				sprintf(buf, "X1541 cable at %03xh", base);
			}
			_cable[idx].base = base;
			_cable[idx].type = CABLE_X1541;
			_cable[idx].name = util_strdup(buf);
			idx++;
			*debug << "X1541 cable detected" << endl;
			/*
			 *  connecting a X1541 cable to EPP or ECP ports
			 *  is generally not a good idea...
			 */
			if (mode == LPT_MODE_EPP) {
				*debug << "* Warning: X1541 on an EPP port!" << endl;
			}
			if (mode == LPT_MODE_ECP) {
				*debug << "* Warning: X1541 on an ECP port!\n" << endl;
			}
		} else if (detectXE1541(base)) {
			if (nr > 0) {
				sprintf(buf, "XE1541 cable at %03xh [LPT%d]",
 					base, nr);
			} else {
				sprintf(buf, "XE1541 cable at %03xh", base);
			}
			_cable[idx].base = base;
			_cable[idx].type = CABLE_XE1541;
			_cable[idx].name = util_strdup(buf);
			idx++;
			*debug << "XE1541 cable detected" << endl;
		} else {
			*debug << "no cable found" << endl;
		}
	}
	_available_cables = idx;

	if (bios_nr != 0) {
		*debug << "- trying LPT" << bios_nr << "... " << flush;
		base = _lpt->get_bios_port(bios_nr);
		if (base != 0) {
			*debug << "ok." << endl;
			for (a = 0;a < _available_cables;a++) {
				if (base == _cable[a].base) {
					*debug << "+ using detected " << _cable[a].name << endl;
					return base;
				}
			}
			/*
			 *  add a cable that is not detected but given on
			 *  the command line (default type is X1541 unless
			 *  specified by '-cable')
			 */
			_cable[_available_cables].base = base;
			if ((cable_type == NULL) || (stricmp(cable_type, "X1541") == 0)) {
				sprintf(buf, "X1541 cable at %03xh [LPT%d]", base, bios_nr);
				_cable[_available_cables].type = CABLE_X1541;
			} else if (stricmp(cable_type, "XE1541") == 0) {
				sprintf(buf, "XE1541 cable at %03xh [LPT%d]", base, bios_nr);
				_cable[_available_cables].type = CABLE_XE1541;
			} else {
				INTERNAL_ERROR("Cable::detect(): unknown cable type");
			}
			_cable[_available_cables].name = util_strdup(buf);
			*debug << "+ using specified " << _cable[_available_cables].name << endl;
			_available_cables++;
			return base;
		}
		*debug << "not available!" << endl;
	}

	idx = 0;
	if (_available_cables > 1) {
		const char **val = new const char * [_available_cables];
		for (a = 0;a < _available_cables;a++) {
			val[a] = _cable[a].name;
		}
		val[a] = NULL;
		idx = _gui->select("Select Cable...",
			"More than one cable found:",
			val);
	}

	if (_cable[idx].base > 0) {
		*debug << "+ using " << _cable[idx].name << endl;
	} else {
		*debug << "* no cable available, file transfer disabled!" << endl;
	}

	return _cable[idx].base;
}

#endif /* MSDOS */
