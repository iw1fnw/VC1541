/*
 *  $Id: io_dos_t.cc,v 1.1 1998/10/26 02:59:05 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifdef MSDOS

#include <pc.h>
#include <dos.h>
#include <go32.h>
#include <dpmi.h>
#include <unistd.h>
#include <iostream.h>
#include <iomanip.h>

#include "misc/debug.h"
#include "vc1541/io_dos.h"

#include "misc/profile.h"

#include "vc1541/lptdetct.h"

static volatile unsigned long counter;

static _go32_dpmi_seginfo irq_handler_old;
static _go32_dpmi_seginfo irq_handler_new;

static void
irq_handler_8h(void)
{
        disable();
	counter++;
        outportb(0x20, 0x20);
}

static void
irq_init(void)
{
        disable();
        _go32_dpmi_get_protected_mode_interrupt_vector(8, &irq_handler_old);
        irq_handler_new.pm_offset = (int)irq_handler_8h;
	irq_handler_new.pm_selector = _go32_my_cs();
        _go32_dpmi_chain_protected_mode_interrupt_vector(8, &irq_handler_new);
        enable();
}

static void
irq_done(void)
{
        disable();
        _go32_dpmi_set_protected_mode_interrupt_vector(8, &irq_handler_old);
        enable();
}

static long
calibrate(void)
{
        int mhz;
	profile_value p1, p2;

	*debug << "- calibrating delay loop..." << flush;

	/*
	int mhz1, mhz2;

        sleep(1);

        *debug << "." << flush;
	tsc_read(&p1); sleep(1); tsc_read(&p2);
        mhz1 = (p2.ll - p1.ll) / 1000000;

        *debug << "." << flush;
	tsc_read(&p1); sleep(2); tsc_read(&p2);
        mhz2 = (p2.ll - p1.ll) / 1000000;

	mhz = (5 * mhz2 - 4 * mhz1) / 6 + 1;
	*/

	irq_init();

	tsc_read(&p1);
	counter = 0;
	while (counter < 1);
	tsc_read(&p1);
	while (counter < 2);
	tsc_read(&p2);

	irq_done();

        mhz = (int)((18.2 * (p2.ll - p1.ll)) / 1000000);

        *debug << " done.\n";

	return mhz;
}

IO_DOS_TSC::IO_DOS_TSC(int mhz, int port, int extended_cable, byte_t atn, byte_t data, byte_t clock) :
	IO(port, extended_cable, atn, data, clock)
{
        _mhz = 0;
	_val = 0xc4;

	if (mhz != 0) {
		_mhz = mhz;
	        *debug << "+ processor speed set to " << _mhz << "MHz" << endl;
		return;
	}

        _mhz = calibrate();
        *debug << "+ processor seems to run at " << _mhz << "MHz" << endl;
}

void
IO_DOS_TSC::set(byte_t bits, byte_t val)
{
	outportb(_port, _val = ((~bits & _val) | val));
}

byte_t
IO_DOS_TSC::get(byte_t bits)
{
	return ((inportb(_port + _offset) >> _shift) ^ _xor) & bits;
}

byte_t
IO_DOS_TSC::wait(byte_t bits, byte_t val)
{
	profile_value p1, p2;

	tsc_read(&p1);
	p1.ll += _mhz * 1000000;
	do {
        	if (get(bits) == val) return 0;
		tsc_read(&p2);
	} while (p1.ll > p2.ll);
	_timeout = 1;
        return 1;
}

byte_t
IO_DOS_TSC::waitEOI(byte_t bits, byte_t val)
{
	profile_value p1, p2;

	tsc_read(&p1);
	p1.ll += _mhz * 1000;
	do {
        	if (get(bits) == val) return 0;
		tsc_read(&p2);
	} while (p1.ll > p2.ll);
        return 1;
}

void
IO_DOS_TSC::microSleep(word_t val)
{
	profile_value p1, p2;

	tsc_read(&p1);
	p1.ll += _mhz * val;
	do {
		tsc_read(&p2);
	} while (p1.ll > p2.ll);
}

int
IO_DOS_TSC::microSleepATN(word_t val)
{
	profile_value p1, p2;

	tsc_read(&p1);
	p1.ll += _mhz * val;
	do {
		if (get(ATN)) return 1;
		tsc_read(&p2);
	} while (p1.ll > p2.ll);
	return 0;
}

#endif /* MSDOS */
