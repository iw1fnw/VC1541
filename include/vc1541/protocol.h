/*
 *  $Id: protocol.h,v 1.4 1998/10/26 03:03:33 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __io_protocol_h
#define __io_protocol_h

#include "vc1541/io.h"
#ifndef __KERNEL__
#include "gui/gui.h"
#endif
#include "libvfs/device.h"

class Protocol
{
private:
	char   *_update;
	File   *_deferred_delete;
	IO     *_io;
#ifndef __KERNEL__
	GUI    *_gui;
	Device *_dev;
#endif
protected:
	int  getIEC(byte_t *ret, int check_atn = 0);
	int  putIEC(byte_t b, int eoi = 0, int check_atn = 0);
	void putFL(byte_t val);
	void sendFile(File *file);
	void receiveFile(Device *dev, char *name);
	void sendDir(Directory *dir, const char *pattern = NULL);
	void sendDirLine(word_t *addr, DirectoryEntry *de);
	void handleATN(void);
	void handleTALK(Device *dev, int sec_addr, char *s);
	void handleLISTEN(Device *dev, int sec_addr, char *iobuf);
	void handleOPEN(Device *dev, int sec_addr, char *iobuf);
	void handleCLOSE(Device *dev, int sec_addr);
	void handleFASTLOAD(Device *dev, int sec_addr);
public:
	Protocol(IO *io
#ifndef __KERNEL__
		, GUI *gui, Device *dev
#endif
	);
	void execute(void);
};

#endif /* __io_protocol_h */

