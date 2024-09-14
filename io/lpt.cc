/*
 *  $Id: lpt.cc,v 1.1 1998/10/26 02:59:06 tp Exp $
 *
 *
 *  Copyright (C) 1997 Wolfgang Moser
 *  (original code was written in assembler; ported to C++ by Torsten Paul)
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifdef MSDOS

#include <pc.h>
#include <iostream.h>
#include <sys/types.h>
#include <sys/movedata.h>

#include "misc/debug.h"
#include "vc1541/lpt.h"

LPT::LPT(void)
{
	int a;

	for (a = 0;a < NR_OF_PORTS;a++) {
		_port[a].base = 0;
		_port[a].mode = LPT_MODE_NONE;
	}
	for (a = 0;a < NR_OF_BIOS_PORTS;a++) _bios_port[a] = 0;
	_std_port[0] = 0x3bc;
	_std_port[1] = 0x378;
	_std_port[2] = 0x278;
}

int
LPT::get_bios_nr(int port)
{
	int a;

	for (a = 0;a < NR_OF_BIOS_PORTS;a++) {
		if (_bios_port[a] == port) return a + 1;
	}
	return 0;
}

int
LPT::get_bios_port(int idx)
{
	unsigned short buf;

	if ((idx < 1) || (idx > 4)) return 0;
	dosmemget(0x0406 + (idx << 1), 2, &buf);
	/*
	 *  windows network printer connected to e.g. LPT3
	 *  store 2 in the BIOS area
	 */
	if (buf <= 2) buf = 0;
        return buf;
}

int
LPT::read_and_reset_epp_timeout(int addr)
{
	int a;

	addr--;
	/* clear flag by reading... */
	a = inportb(addr);
	/* ...by writing a 1... */
	outportb(addr, a | 1);
	/* ...or by writing a 0 to it */
	outportb(addr, a & ~1);

	return a & 1;
}

lpt_mode_t
LPT::lpt_mode(int base)
{
	int a;
	int addr, ecp_addr;
	unsigned char ECRstatus;

	addr = base + 2;
	ecp_addr = addr + 0x400;

	/*
	 *****************************************************************
	 *  check whether addr is a valid port address
	 */
	if ((addr & ~0x1fc) != 0x202) {
		debug->form("  invalid port address: 0x%04x\n", base);
		return LPT_MODE_NONE;
	}

	/*
	 *****************************************************************
	 *  reset port to prevent printers from printing
	 *  unusable stuff
	 */
	a = inportb(addr);
	outportb(addr, a & 0xfb);
	/* FIXME: delay */
	outportb(addr, a);

	/*
	 *****************************************************************
	 *  check, if there's an ECP port
	 */
	/* preserve the status of ECR */
	ECRstatus = inportb(ecp_addr);
	/* switch ECR to Standard-Mode to preserve the
	   FIFO-Bits from the influence of conneted cables */
	outportb(ecp_addr, 0x34);

	/* clear Strobe-Bit, set Auto-Linefeed-Bit */
	outportb(addr, (inportb(addr) & 0xfe) | 0x02);

	/* combined test for isolated FIFO-Bits of ECR
	   and the read-only characteristics of the FIFO
	   --> if there's no ECR, we read the mirrored address
	       of the Control-Port (Strobe and Auto-Linefeed) */
	if (inportb(ecp_addr) == 0x35) {
		/* switch the ECR via Byte-Mode... */
		outportb(ecp_addr, 0x35);
		/* ... to the FIFO-Test-Mode */
		outportb(ecp_addr, 0xd4);

		/* read one, so that the FIFO has room
		   for at least one byte */
		inportb(ecp_addr - 2);
		/* Look, if there is the FIFO-Test-Register at
		   Base + 0x400, but don't test the FIFO-Full/
		   FIFO-Empty-Bits or the Interrupt-Flag */
		outportb(ecp_addr - 2, 0xaa);
		if (inportb(ecp_addr - 2) == 0xaa) {
			outportb(ecp_addr - 2, 0x55);
			if (inportb(ecp_addr - 2) == 0x55) {
				/* temporary used Byte Mode */
				outportb(ecp_addr, 0x35);
				/* switch to output mode */
				outportb(ecp_addr, ECRstatus);
				/* set port to input mode */
				outportb(addr, inportb(addr) & ~0x20);
				return LPT_MODE_ECP;
			}
		}
	}

	/* temporary used Byte Mode */
	outportb(ecp_addr, 0x35);
	/* restore status of ECR */
	outportb(ecp_addr, ECRstatus);

	/*
	 *****************************************************************
	 *  check, if there's an EPP port
	 */
	/* set the Bidirectional-Flag to input (reverse mode) and
	   block the DataStrobe, AddressStrobe and Write line manually,
	   so that the EPP can't send any automatic handshake signals */
	outportb(addr, inportb(addr) | 0x2b);
	/* test, if the EPP-Timeout-Bit reacts */
	read_and_reset_epp_timeout(addr);
	/* write to EPP-Address-Port */
	outportb(addr + 1, 0x00);
	/* test and clear EPP-Bit */
	if (read_and_reset_epp_timeout(addr) != 0) {
		/* test and clear EPP-Bit */
		if (read_and_reset_epp_timeout(addr) == 0) {
			/* Timeout-Bit works, there's an EPP */
			/* set the Bidirectional-Flag to output and free
			   the DataStrobe, AddressStrobe and Write line */
			outportb(addr, inportb(addr) & ~(0x20 | 0x0b));
			return LPT_MODE_EPP;
		}
		/* timeout bit wasn't reset */
	}

	/*
	 *****************************************************************
	 *  check, if there's a PS/2 or SPP port
	 */
	/* reset all handshake lines and switch to output mode */
	outportb(addr, inportb(addr) & ~(0x20 | 0x0b));

	/* write to Data-Port */
	outportb(base, 0xaa);
	/* read from Data-Port, if it doesn't match, there's no port */
	if (inportb(base) != 0xaa) {
		return LPT_MODE_NONE;
	}

	/* write to Data-Port */
	outportb(base, 0x55);
	/* read from Data-Port, if it doesn't match, there's no port */
	if (inportb(base) != 0x55) {
		return LPT_MODE_NONE;
	}

	/*
	 *****************************************************************
	 *  check, if the found port is a PS/2 or SPP port
	 */
	/* input mode */
	outportb(addr, inportb(addr) | 0x20);

	/* write to Data-Port */
	outportb(base, 0xaa);
	/* read from Data-Port, if it doesn't match, there's a PS/2 port */
	if (inportb(base) != 0xaa) {
		/* set port to input mode */
		outportb(addr, inportb(addr) & ~0x20);
		return LPT_MODE_PS2;
	}

	/* write to Data-Port */
	outportb(base, 0x55);
	/* read from Data-Port, if it doesn't match, there's a PS/2 port */
	if (inportb(base) != 0x55) {
		/* set port to input mode */
		outportb(addr, inportb(addr) & ~0x20);
		return LPT_MODE_PS2;
	}

	/* set port to input mode */
	outportb(addr, inportb(addr) & ~0x20);
	return LPT_MODE_SPP;
}

void
LPT::detect(void)
{
	int a;

	/*
	 *  read bios ports
	 */
	for (a = 0;a < NR_OF_BIOS_PORTS;a++) {
		_bios_port[a] = get_bios_port(a + 1);
		if(!_bios_port[a]) break;
	}

	*debug << "- checking BIOS for parallel ports..." << endl;
	for (a = 0;a < NR_OF_BIOS_PORTS;a++) {
		if (_bios_port[a] == 0) continue;
		debug->form("  LPT%d is at %03xh\n", a + 1, _bios_port[a]);
	}

	*debug << "- detecting parallel ports..." << endl;
	for (a = 0;a < NR_OF_STD_PORTS;a++) {
		debug->form("  %03xh - ", _std_port[a]);
		_port[a].base = _std_port[a];
		_port[a].mode = lpt_mode(_port[a].base);
		switch (_port[a].mode) {
		case LPT_MODE_NONE:
			*debug << "N/A" << endl;
			break;
		case LPT_MODE_SPP:
			*debug << "SPP mode" << endl;
			break;
		case LPT_MODE_PS2:
			*debug << "PS/2 mode" << endl;
			break;
		case LPT_MODE_EPP:
			*debug << "EPP mode" << endl;
			break;
		case LPT_MODE_ECP:
			*debug << "ECP mode" << endl;
			/* switch ECP ports to Byte Mode */
			outportb(_port[a].base + 2, (inportb(_port[a].base + 2) & 0x1f) | 0x20);
			break;
		}
	}
}

int LPT::get_base(int idx)
{
	if ((idx < 0) || (idx >= NR_OF_PORTS)) return 0;
	return _port[idx].base;
}

lpt_mode_t LPT::get_mode(int idx)
{
	if ((idx < 0) || (idx >= NR_OF_PORTS)) return LPT_MODE_NONE;
	return _port[idx].mode;
}

#endif /* MSDOS */
