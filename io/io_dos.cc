/*
 *  $Id: io_dos.cc,v 1.5 1998/10/26 02:59:04 tp Exp $
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
#include <signal.h>
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
	int a, inc;
	unsigned long counter_val;

	*debug << "- calibrating delay loop." << flush;

        irq_init();

        a = 32768;
        do {
                a *= 2;
	        counter = 0;
	        while (counter == 0);
	        asm (
	                "0:decl	%%ecx			\n\t"
	                "jnz	0b			\n\t"
	                :
	                : "c" (a)
	        );
                *debug << "." << flush;
	} while (counter < 2);

        *debug << "o" << flush;

        a /= 2;
        inc = a / 2;
        do {
	        counter = 0;
	        while (counter == 0);
	        asm (
	                "0:decl	%%ecx			\n\t"
	                "jnz	0b			\n\t"
	                :
	                : "c" (a + inc)
	        );
                *debug << "." << flush;
                if (counter >= 2) {
                	inc /= 2;
                } else {
                	a += inc;
                }
	} while ((a / inc) < 100);

        irq_done();

	counter_val = (unsigned long)((a * 20) / 1000);

        *debug << " done.\n";

	return counter_val;
}

IO_DOS::IO_DOS(int mhz, int port, int extended_cable, byte_t atn, byte_t data, byte_t clock) :
	IO(port, extended_cable, atn, data, clock)
{
        _mhz = 0;
	_val = 0xc4;

	if (mhz != 0) {
		_mhz = mhz;
		*debug << "+ processor speed index set to " << _mhz << endl;
		return;
	}

        _mhz = calibrate();
        *debug << "+ processor has speed index " << _mhz << endl;
}

void
IO_DOS::set(byte_t bits, byte_t val)
{
	outportb(_port, _val = ((~bits & _val) | val));
}

byte_t
IO_DOS::get(byte_t bits)
{
	return ((inportb(_port + _offset) >> _shift) ^ _xor) & bits;
}

byte_t
IO_DOS::wait(byte_t bits, byte_t val)
{
	/*
	register byte_t ret;

	asm (
	      "0:movb	%%al, %%ah	\n\t"
		"inb	(%%dx), %%al	\t\n"
		"andb	%%bl, %%al	\t\n"
		"cmpb	%%al, %%ah	\t\n"
		"jnz	1f		\t\n"
		"movb	$0, %%al	\t\n"
		"jmp	2f		\t\n"
	      "1:decl	%%ecx		\t\n"
	        "jnz	0b		\t\n"
		"movb	$1, %%al	\t\n"
	      "2:			\t\n"

		: "=1" (ret)
		: "a" ((byte_t)val),
		  "b" ((byte_t)bits),
		  "c" (1000 * _mhz),
		  "d" ((word_t)_port)
	);
	_timeout = ret;
	return ret;
	*/

	if (get(bits) == val) return 0;

	unsigned long count = 64 * _mhz;

	for (;;) {
		if (get(bits) == val) return 0;
		if (--count == 0) break;
	}
	_timeout = 1;
	return 1;
}

byte_t
IO_DOS::waitEOI(byte_t bits, byte_t val)
{
	if (get(bits) == val) return 0;

	unsigned long count = _mhz / 16;

	for (;;) {
		if (get(bits) == val) return 0;
		if (--count == 0)  break;
	}
	return 1;
}

void
IO_DOS::microSleep(word_t val)
{
	unsigned long x =  (_mhz * val) / 1000;
	asm (
		"0:decl  %%ecx	\n\t"
		"jnz     0b	\n\t"
                : /* no output */
	        : "c" (x)
        );
}

int
IO_DOS::microSleepATN(word_t val)
{
	unsigned long x =  (_mhz * val) / 1000;
	asm (
		"0:decl  %%ecx	\n\t"
		"jnz     0b	\n\t"
                : /* no output */
	        : "c" (x)
        );
	return 0;
}

#endif /* MSDOS */
