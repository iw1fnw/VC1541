/*
 *  $Id: lptdetct.h,v 1.1.1.1 1997/10/25 23:11:09 tp Exp $
 *
 *
 *  Copyright (C) 1997 Wolfgang Moser aka Womo
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Parallel port autodetection routines
 *
 * Wolfgang Moser, 1997, September, 13, 22:00
 *   ia122@advs2.gm.fh-koeln.de, http://www.gm.fh-koeln.de/~isp005
 *   (up to January 1998)
 *
 * Basic information from JAN'S PARALLEL PORT FAQ, Latest update: 4/22/97
 *   Jan Axelson, jaxelson@lvr.com
 *   links at: http://www.lvr.com/parport.htm
 *
 * Basic implementations by Kov cs Bal zs aka Joe Forster/STA
 *   sta@ludens.elte.hu, http://ludens.elte.hu/~sta
 * Check out for his Star Commander at http://ludens.elte.hu/~sta/sc.html,
 * the final solution to handle disk/tape images for C64 emulators.
 *
 *
 * For additional informations to printer ports check:
 *   http://www.doc.ic.ac.uk/~ih/doc/par/index.html
 *   http://www.senet.com.au/~cpeacock/parallel.htm
 *   http://www.cs.unc.edu/~tracker/tech/epp.html
 *   http://www.paranoia.com/~filipg/HTML/LINK/PORTS/F_Parallel.html
 *
 * One of the best parallel port detection utilities is
 * PARALLEL, Version 1.4 from Parallel Technologies, Inc.
 *   http://www.lpt.com/
 * It includes tests for an automatic IRQ detection.
 * You can get the file para14.zip from: ftp://lpt.com/Parallel/
 *                                      http://www.fapo.com/useful.htm
 */

#ifndef __LPTdetect_H__
#define __LPTdetect_H__

#include <dos.h>

	// I've included tests on unusual port addresses, because
	// I bought an dual ECP/EPP-card a few months ago, which is
	// able to set up ports from LPT1 to LPT6.
	//
	// Addresses: 0x3bc, 0x378, 0x278, 0x268, 0x27c, 0x26c
	//
	// With ports on 0x27c there are conflicts with an EPP at 0x278
	// possible, lptMode checks for such a conflict, too
	//
	// Set ExtendedTests to 0, if you only want to test LPT1 to LPT3
	// #define ExtendedTests 0
#define ExtendedTests 1
	// Set ExtendedTests to 1, if you want to try searching for unusual EPP's
	// But take notice that the functionality of this test is not guaranteed!
	// #define AdvancedEPPTests 0
#define AdvancedEPPTests 1


enum lptMode{
	lptN_A	= 0,
	lptSPP	= 1,
	lptPS2	= 2,
	lptEPP	= 3,
	lptECP	= 4,
	lptEPPc	= 5,			// EPP with special control word to enable
	lptMIR	= 7,        // at 0x26c there's a mirrored port from 0x268
								// at 0x27c there's a mirrored port from 0x278
	lptERR = 8           // conflict on 0x27c with EPP on 0x278
	};

enum ecpMode{
	ecpNOECR   = 0,      // no ECP-Port found!!!

	ecpSTNDRD  = 1,
	ecpBYTE    = 2,
	ecpSTDFIFO = 3,
	ecpECPFIFO = 4,
	ecpEPP     = 5,
	ecpRESVRD  = 6,
	ecpFIFOTST = 7,
	ecpCONFIG  = 8
	};

int            BIOSlpt2port(int lpt);
lptMode        LPTmode     (int port);
ecpMode        ECPmode     (int port);
char          *EPPcontrol  (int port);

#endif
