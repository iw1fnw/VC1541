/*
 *  $Id: io_linux.h,v 1.3 1998/10/26 03:03:32 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __io_io_linux_h
#define __io_io_linux_h

#include "vc1541/io.h"

class IO_Linux : public IO
{
private:
	int _mhz;
public:
	IO_Linux(int mhz, word_t port,
 		byte_t atn = 0, byte_t data = 3, byte_t clock = 1);

	virtual byte_t get(byte_t bits);
	virtual void set(byte_t bits, byte_t val);
	virtual byte_t wait(byte_t bits, byte_t val);
	virtual byte_t waitEOI(byte_t bits, byte_t val);
	virtual void microSleep(word_t val);
	virtual int  microSleepATN(word_t val);
};

#endif /* __io_io_linux_h */
