/*
 *  $Id: lptdetct.cc,v 1.2 1997/12/09 09:41:22 tp Exp $
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
 * Basic informations from JAN'S PARALLEL PORT FAQ, Latest update: 4/22/97
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

#ifdef MSDOS

#include <sys/types.h>
#include <sys/movedata.h>

#include "vc1541/lptdetct.h"

inline static void 	BIDIinp 	(int port);
inline static void 	BIDIoutp	(int port);
inline static void 	EPPclear	(int port);
static int  			ECPdetect(int port);
static int  			EPPdetect(int port);
inline static int 	SPPdetect(int port);
inline static int 	PS2detect(int port);


int BIOSlpt2port(int lpt){
	unsigned short buf;

		// The BIOS table has space for 4 LPT-entries,
		// but uses only up to 3 in most cases
	if(lpt<1 || lpt>4) return 0;
	dosmemget(0x0406 + (lpt << 1), 2, &buf);
        return buf;
	}

lptMode LPTmode(int port){
		// check for valid portaddresses (LPT 1-6)
#if ExtendedTests
		//    LPT1                   LPT2,3
	if(port!=0x3bc && (port&~0x100)!=0x278 &&
		//    LPT4                   LPT5,6
		port!=0x268 && (port&~0x010)!=0x26c) return lptN_A;
#else
		//    LPT1                   LPT2,3
	if(port!=0x3bc && (port&~0x100)!=0x278) return lptN_A;
#endif

	  // begin tests
	if((port&~0x100)==0x278){        // ECP/EPP-Ports can only
												// exist at 0x378/0x278
			// tests for an ECP
		if(ECPdetect(port)) return lptECP;

			// tests for an EPP
		BIDIoutp(port);               // clear the Bidirectional-Flag
		if(EPPdetect(port)) return lptEPP;

#if AdvancedEPPTests
		unsigned char EPPctrl;

		EPPctrl=0;  // check all control words to free up the EPP
		do{
				// write the special Control word, for freeing up the EPP
			outportb(port+2,EPPctrl);
			if(EPPdetect(port)) return lptEPPc;
			}while(EPPctrl);
			// I think, there's really no EPP
#endif
		}
#if ExtendedTests
	else if((port&~0x010)==0x26c){
		lptMode mode2x8=LPTmode(port-4);
		if(mode2x8==lptEPP || mode2x8==lptEPPc || mode2x8==lptECP){
													// only possible at 0x278
			unsigned char freeADR,ECR;
				// tests on a possible collision with an EPP at 0x278

			if(mode2x8==lptECP){
				ECR=inportb(port+0x402);      // preserve the status of ECR
					// switch ECR to EPP-Mode
				outportb(port+0x402,0x34);
				}
			// test bit pattern changes at 0x27F, if they do, the EPP
			// has the ability to transfer DWORDS (4 Bytes at once)

			BIDIoutp(port-4);                // clear the Bidirectional-Flag
			EPPclear(port-4);           		// reset Timeout-Flag
			outportb(port+3,0xAA);      		// write to EPP-Data-Port 3
			EPPclear(port-4);           		// reset Timeout-Flag
			if(inportb(port+3)==0xAA){  		// if register base+8 exists
				EPPclear(port-4);        		// reset Timeout-Flag
				outportb(port+3,0x55);   		// write to EPP-Data-Port 3
				EPPclear(port-4);        		// reset Timeout-Flag
				if(inportb(port+3)==0x55){
						// the EPP has the ability to transfer DWORDS

						// write to status, if it doesn't change
						// on all bits (reserved ones) ==> conflict
					outportb(port+1,0xAA);     // write to Status-Port
					EPPclear(port-4);          // reset Timeout-Flag
					if(inportb(port+1)!=0xAA){ // read from Status-Port, if it doesn't
						if(mode2x8==lptECP) outportb(port+0x402,ECR); // restore ECR
						return lptERR;          // match, there's possibly a conflict
						}
					outportb(port+1,0x55);     // write to Status-Port
					EPPclear(port-4);          // reset Timeout-Flag
					if(inportb(port+1)!=0x55){ // read from Status-Port, if it doesn't
						if(mode2x8==lptECP) outportb(port+0x402,ECR); // restore ECR
						return lptERR;          // match, there's possibly a conflict
						}
					if(mode2x8==lptECP) outportb(port+0x402,ECR); // restore ECR
					return lptN_A;
					}
				}
				// the EPP doesn't have the ability to transfer DWORDS

				// get Control, if there are no ports, it contains mostly 0xFF
			freeADR=inportb(port+2);
				// write to Control, if something changes, there's
				// a port, but not from an EPP (no DWORD transfers)
			outportb(port+2,0xAA);
			EPPclear(port-4);                    // reset Timeout-Flag
			if(inportb(port+2)==freeADR){
				outportb(port+2,0x55);
				EPPclear(port-4);                 // reset Timeout-Flag
				if(inportb(port+2)==freeADR){
					if(mode2x8==lptECP) outportb(port+0x402,ECR); // restore ECR
					return lptN_A;
					}
				}
			if(mode2x8==lptECP) outportb(port+0x402,ECR); // restore ECR
			return lptERR;
			}
		else if(mode2x8>lptN_A && mode2x8<=lptECP){  // if SPP, PS/2 or ECP
			unsigned char ECR;
				// tests if there's only a mirrored SPP or PS/2 from 0x278/0x268

			if(mode2x8==lptECP){
				ECR=inportb(port-4+0x402);
				outportb(port-4+0x402,0x34);
				}

			BIDIoutp(port-4);
			BIDIoutp(port);
			outportb(port-4,0x55);
			outportb(port,0xAA);        // posibly a mirrored address of 0x278
			if(inportb(port-4)==0xAA){  // if it's a mirror, we read 0xAA
				if(mode2x8==lptECP) outportb(port-4+0x402,ECR);
				return lptMIR;
				}
			if(mode2x8==lptECP) outportb(port-4+0x402,ECR);
			}
		}
#endif

	if(!SPPdetect(port)) return lptN_A;
	if(PS2detect(port)) return lptPS2;
	return lptSPP;                   // that's an SPP-Port
	}

ecpMode ECPmode(int port){
	if(LPTmode(port)!=lptECP) return ecpNOECR;
	return ecpMode(ecpSTNDRD + (inportb(port+0x402)>>5));
	}

#if AdvancedEPPTests
// Resolving the special Control-Word to enable an EPP
char *EPPcontrol(int port){
	static char ctrlWord[8];
	unsigned int  done0[8], done1[8];
	unsigned char i,EPPctrl,mask;

	for(i=0;i<8;i++) done0[i]=done1[i]=0;


	EPPctrl=0;  // check all control words to free up the EPP
	do{
			// write the special Control word, for freeing up the EPP
		outportb(port+2,EPPctrl);
		if(EPPdetect(port)){
			// EPP is enabled with this control word
			// return EPPctrl;
			for(i=0,mask=0x80;i<8;i++,mask>>=1){
				if(EPPctrl&mask) done1[i]++;
				else             done0[i]++;
				}
			i=EPPctrl++;
			}
		}while(EPPctrl);
		// I think, there's really no EPP
	outportb(port+2,i);	// Enable the EPP with the last Control-Word found
	for(i=0;i<8;i++){
		if(done0[i]==done1[i]){
			if(!done0[i]) 	 ctrlWord[i]='!';	// Control-Word could not found
			else			  	 ctrlWord[i]='X';	// This Bit cares nobody
			}
		else if(!done0[i]) ctrlWord[i]='1'; // This Bit must be 1
		else if(!done1[i]) ctrlWord[i]='0';	// This Bit must be 0
		else               ctrlWord[i]='?';	// This Bit depends on other Bits
		}
	return ctrlWord;
	}
#else
char *EPPcontrol(int){
	return "XX0XXXXX";
	}
#endif

//----------------------------------------------------------------
// some Helper-Funcs, for use only in this file-context
//----------------------------------------------------------------

static int ECPdetect(int port){
	unsigned char  ECR;

	ECR=inportb(port+0x402);      // preserve the status of ECR
		// switch ECR to Standard-Mode to preserve the
		// FIFO-Bits from the influence of connected cables
	outportb(port+0x402,0x34);

		// clear Strobe-Bit, set Auto-Linefeed-Bit
	outportb(port+2,(inportb(port+2)&0xFE)|0x02);   // xxxxxx10

		// combined test for isolated FIFO-Bits of ECR
		// and the read-only characteristics of the FIFO
		// --> if there's no ECR, we read the mirrored address
		//     of the Control-Port (Strobe and Auto-Linefeed)
	if(inportb(port+0x402)==0x35){                  // xxxxxx01 ?
			// switch the ECR to FIFO-Test-Mode
		outportb(port+0x402,0xd4);
			// Look, if there is the FIFO-Test-Register at Base+0x400,
			// but don't test the FIFO-Full/Empty-Bits or the Interrupt-Flag
		inportb(port+0x400);				// Read one, so that the FIFO
												// has room for at least one byte
		outportb(port+0x400,0xAA);
		if(inportb(port+0x400)==0xAA){
			outportb(port+0x400,0x55);
			if(inportb(port+0x400)==0x55){
				outportb(port+0x402,ECR);
				return 1;
				}
			}
		}
	outportb(port+0x402,ECR);
	return 0;
	}

/* Umstellung auf Timeout-Bit-Test
static int EPPdetect(int port){
		// test, if there is a register at base+3
	EPPclear(port);               // reset Timeout-Flag
	outportb(port+3,0xAA);        // write to EPP-Address-Port
	EPPclear(port);               // reset Timeout-Flag
	if(inportb(port+3)==0xAA){    // if register base+3 exists
		EPPclear(port);            // reset Timeout-Flag
			// test, if it is really a port at base+3
		outportb(port+3,0x55);     // write to EPP-Address-Port
		EPPclear(port);            // reset Timeout-Flag
		if(inportb(port+3)==0x55) return 1;
		}
	return 0;
	}
*/

static int EPPdetect(int port){
		// test, if the EPP-Timeout-Bit reacts
	EPPclear(port);               	// reset Timeout-Flag
	outportb(port+3,0x00);        	// write to EPP-Address-Port
	if(inportb(port+1)&0x01){			// Timeout-Bit is 1
		EPPclear(port);            	// reset Timeout-Flag
		if(!(inportb(port+1)&0x01)){	// Timeout-Bit is 0
			return 1;
			}
		}
	return 0;
	}

inline static int SPPdetect(int port){
		// tests for a SPP
	BIDIoutp(port);                  // clear the Bidirectional-Flag

	outportb(port,0xAA);             // write to Data-Port
	if(inportb(port)!=0xAA)          // read from Data-Port, if it
		return 0;                		// doesn't match, there's no port
	outportb(port,0x55);             // write to Data-Port
	if(inportb(port)!=0x55)          // read from Data-Port, if it
		return 0;                		// doesn't match, there's no port
	return 1;
	}

inline static int PS2detect(int port){
		// tests for a PS/2
	BIDIinp(port);                   // set the Bidirectional-Flag
	outportb(port,0xAA);             // write to Data-Port
	if(inportb(port)!=0xAA){         // read from Data-Port, if it
		BIDIoutp(port);
		return 1;                		// doesn't match, there's an PS/2
		}
	outportb(port,0x55);             // write to Data-Port
	if(inportb(port)!=0x55){         // read from Data-Port, if it
		BIDIoutp(port);
		return 1;                		// doesn't match, there's an PS/2
		}
	return 0;                   		// that's an SPP-Port
	}

	// set the Bidirectional-Flag to input (reverse mode)
inline static void BIDIinp(int port){
	outportb(port+2,inportb(port+2)|0x20);
	}

	// set the Bidirectional-Flag to output (normal mode)
inline static void BIDIoutp(int port){
	outportb(port+2,inportb(port+2)&0xDF);
	}

	// clear the Timeout-Flag
inline static void EPPclear(int port){
		// Reset Timeout-Flag by with reading
	register unsigned char val=inportb(port+1);
	outportb(port+1,val|0x01);    // or by writing 0
	outportb(port+1,val&0xFE);    // or 1 to it
	}

#endif /* MSDOS */
