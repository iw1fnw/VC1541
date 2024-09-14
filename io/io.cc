/*
 *  $Id: io.cc,v 1.2 1998/10/26 02:59:03 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <iostream.h>
#include <iomanip.h>

#include "vc1541/io.h"

IO::IO(word_t port, int extended_cable,
 	byte_t atn, byte_t data, byte_t clock)
{
	if (port != 0) {
		_port = port + 2;
        } else {
		_port = 0;
        }
	_timeout  = 0;
	_val      = 0xc4;

	ATN_BIT   = atn;
 	DATA_BIT  = data;
 	CLOCK_BIT = clock;
	ATN       = 1 << atn;
	DATA      = 1 << data;
	CLOCK     = 1 << clock;

	if (extended_cable) {
		/*
		 *  XE1541 cable
		 */
		_offset = -1;
		_shift  = 4;
		_xor    = 3;
	} else {
		/*
		 *  X1541 cable
		 */
		_offset = 0;
		_shift  = 0;
		_xor    = 0;
	}
}

void IO::setPort(word_t port)
{
	_port = port + 2;
}

