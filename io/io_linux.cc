/*
 *  $Id: io_linux.cc,v 1.4 1998/10/26 02:59:05 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifdef LINUX

#include <asm/io.h>
#include <unistd.h>
#include <iostream.h>

#include "misc/debug.h"
#include "vc1541/io_linux.h"

/* #define HAVE_TSC */
#include "misc/profile.h"

IO_Linux::IO_Linux(int mhz, word_t port,
 		   byte_t atn, byte_t data, byte_t clock) :
	IO(port, atn, data, clock)
{
	profile_value p1, p2;
        int mhz1, mhz2;

        _mhz = 0;
	if (mhz != 0) {
		_mhz = mhz;
		debug->form("+ processor speed set to %dMHz.\n", mhz);
		return;
	}

#ifdef HAVE_TSC
	*debug << "- calibrating delay loop." << flush;
        sleep(1);

        *debug << "." << flush;
	tsc_read(&p1); sleep(1); tsc_read(&p2);
        mhz1 = (p2.ll - p1.ll) / 1000000;

        *debug << "." << flush;
	tsc_read(&p1); sleep(2); tsc_read(&p2);
        mhz2 = (p2.ll - p1.ll) / 1000000;

        _mhz = (5 * mhz2 - 4 * mhz1) / 6 + 1;

        *debug << " done.\n";
        *debug << "+ processor seems to run at " << dec << _mhz << "MHz.\n";
#endif /* HAVE_TSC */

	//_val = inportb(_port) & ~(ATN_BIT | DATA_BIT | CLOCK_BIT | 0x10);
}

void IO_Linux::set(byte_t /* bits */, byte_t /* val */)
{
	//outportb(_port, _val = ((~bits & _val) | val));
}

byte_t IO_Linux::get(byte_t /* bits */)
{
	//return inportb(_port) & bits;
        return 0;
}

byte_t IO_Linux::wait(byte_t /* bits */, byte_t /* val */)
{
/*
	byte_t a;
	unsigned long count = 0;

	while (1) {
		a = inportb(_port);
		if ((a & bits) == val) return 0;
		if (count++ > 1000000) {
			_timeout = 1;
			return 1;
		}
	}
*/
	return 0;
}

byte_t IO_Linux::waitEOI(byte_t /* bits */, byte_t /* val */)
{
/*
	byte_t a;
	unsigned long count = 0;

	while (1) {
		a = inportb(_port);
		if ((a & bits) == val) return 0;
		if (count++ > 10000) return 1;
	}
*/
	return 0;
}

void IO_Linux::microSleep(word_t val)
{
#ifdef HAVE_TSC
	profile_value p1, p2;

	tsc_read(&p1);
	p1.ll += 200 * val;
	do {
		tsc_read(&p2);
	} while (p1.ll > p2.ll);
#endif /* HAVE_TSC */
}

int IO_Linux::microSleepATN(word_t val)
{
#ifdef HAVE_TSC
	profile_value p1, p2;

	tsc_read(&p1);
	p1.ll += 200 * val;
	do {
		/*
		 * must check for ATN here
		 */
		tsc_read(&p2);
	} while (p1.ll > p2.ll);
#endif /* HAVE_TSC */
}

#endif /* LINUX */
