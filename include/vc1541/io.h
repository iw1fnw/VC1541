/*
 *  $Id: io.h,v 1.2 1998/10/26 03:03:30 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __io_io_h
#define __io_io_h

#include "libvfs/common.h"

class IO
{
protected:
	word_t _port;
	byte_t _val;
	int    _offset;
	int    _shift;
	int    _xor;

public:
        static const int NR_OF_PORTS = 6;

	int _timeout;

public:
	static const byte_t IEC_COMMAND  = 0xf0;
	static const byte_t IEC_DEVICE   = 0x0f;
	static const byte_t IEC_UNTALK   = 0x50;
	static const byte_t IEC_UNLISTEN = 0x30;
	static const byte_t IEC_LISTEN   = 0x20;
	static const byte_t IEC_TALK     = 0x40;
	static const byte_t IEC_MODE     = 0xf0;
	static const byte_t IEC_ADDRESS  = 0x0f;
	static const byte_t IEC_OPEN     = 0xf0;
	static const byte_t IEC_CLOSE    = 0xe0;
	static const byte_t IEC_FASTLOAD = 0xd0;
	static const byte_t IEC_DATA     = 0x60;

	byte_t ATN;
	byte_t DATA;
	byte_t CLOCK;
	byte_t ATN_BIT;
	byte_t DATA_BIT;
	byte_t CLOCK_BIT;

	IO(word_t port,	int extended_cable = 0,
 		byte_t atn = 0, byte_t data = 3, byte_t clock = 1);


	virtual void setPort(word_t port);

	virtual byte_t get(byte_t bits) = 0;
	virtual void set(byte_t bits, byte_t val) = 0;

	virtual byte_t wait(byte_t bits, byte_t val) = 0;
	virtual byte_t waitEOI(byte_t bits, byte_t val) = 0;
	virtual void microSleep(word_t val) = 0;
	virtual int  microSleepATN(word_t val) = 0;
};

#endif /* __io_io_h */

