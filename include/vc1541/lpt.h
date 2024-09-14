/*
 *  $Id: lpt.h,v 1.1 1998/10/26 03:03:33 tp Exp $
 *
 *
 *  Copyright (C) 1997 Wolfgang Moser
 *  (original code was written in assembler; ported to C++ by Torsten Paul)
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __vc1541_lpt_h
#define __vc1541_lpt_h

typedef enum {
	LPT_MODE_NONE,
	LPT_MODE_SPP,
	LPT_MODE_PS2,
	LPT_MODE_EPP,
	LPT_MODE_ECP
} lpt_mode_t;

class LPT
{
public:
	static const int NR_OF_PORTS      = 4;
	static const int NR_OF_STD_PORTS  = 3;
	static const int NR_OF_BIOS_PORTS = 4;

protected:
	struct {
		int base;
		lpt_mode_t mode;
	} _port[NR_OF_PORTS];
	int _std_port[NR_OF_STD_PORTS];
	int _bios_port[NR_OF_BIOS_PORTS];

	int read_and_reset_epp_timeout(int addr);
	lpt_mode_t lpt_mode(int port);

public:
	LPT(void);
	~LPT(void) {}
	void detect(void);
	int get_bios_nr(int port);
	int get_bios_port(int idx);
	int get_base(int idx);
	lpt_mode_t get_mode(int idx);
};

#endif /* __vc1541_lpt_h */

