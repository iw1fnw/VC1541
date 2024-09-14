/*
 *  $Id: protocol.cc,v 1.6 1998/10/26 02:59:07 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <ctype.h>
#include <iostream.h>
#include <iomanip.h>
#include <setjmp.h>

#ifdef MSDOS
#include <pc.h>
#include <dos.h>
#endif /* MSDOS */
#ifdef LINUX
#include <asm/io.h>
#define inportb(port) inb(port)
#define outportb(port,val) outb(val,port)
#endif

#include "vc1541/io.h"
#include "vc1541/protocol.h"

#include "misc/util.h"
#include "misc/debug.h"

#include "libvfs/dir.h"

#if 1
#ifdef MSDOS
#define USE_CLI
#endif /* MSDOS */
#endif

#ifdef MSDOS
static unsigned int videoseg = 0xb8010;

void DPRINT(void *s)
{
#if 1
	unsigned short a = 0x0f00 | (*((char *)s) & 0xff);

	asm (
		"movw %%ax, %%gs:(%%ebx)\n\t"
		: /* no output */
		: "a" ((unsigned short)a), "b" (videoseg)
	);
	videoseg = (videoseg + 2) & 0xb807f;
#endif
}

void IDLE(void)
{
#if 1
	static char s[] = "|/-\\";
	static int count = 0;
	unsigned short a = 0x0f00 | s[count];

        asm (
        	"movw %%ax, %%gs:(%%ebx)\n\t"
                : /* no output */
                : "a" ((unsigned short)a), "b" (videoseg)
        );
        if (++count > 3) count = 0;
#endif
}
#endif /* MSDOS */

#ifdef LINUX
void DPRINT(void * /* s */)
{
}

void IDLE(void)
{
}
#endif /* LINUX */

Protocol::Protocol(IO *io, GUI *gui, Device *dev)
{
	_io  = io;
	_gui = gui;
	_dev = dev;

#ifdef MSDOS
	union REGS r;

	/*
	 *  get video mode
	 *
	 *  if we get r.h.al == 7 we are displaying on MDA
	 */
	r.w.ax = 0x0f00;
	int86(0x10, &r, &r);

	if ((r.h.al != 7) && (inportb(0x3cc) & 1)) {
		videoseg = 0xb8010;
		// col      = 0x03;
		// colxor   = 0x0f;
		*debug << "+ using color display." << endl;
	} else {
		videoseg = 0xb0010;
		// col      = 0x03;
		// colxor   = 0x7f;
		*debug << "+ using monochrome display." << endl;
	}
#endif

	_update = NULL;
        _deferred_delete = NULL;
}

int Protocol::getIEC(byte_t *ret, int check_atn)
{
	int eoi;
	byte_t b;
	byte_t DATA     = _io->DATA;
	byte_t CLOCK    = _io->CLOCK;
	byte_t DATA_BIT = _io->DATA_BIT;

#ifdef USE_CLI
	//asm ("cli\n\t");
#endif /* USE_CLI */

	eoi = 0;
	if (_io->wait(CLOCK, 0)) return 0;                    	       /* [$e9d5] */

	_io->set(DATA, 0);                                    	       /* [$e9d7] */
	if (_io->wait(DATA, 0)) return 0;   /* [$ff25] (patch called from $e9dc) */

	/*
	 *  check for eoi
	 */
	if (_io->waitEOI(CLOCK, CLOCK)) {  		                       /* [$e9ee] */
		eoi = 1;
		DPRINT("*");
		_io->set(_io->DATA, _io->DATA);                        	       /* [$e9f2] */
		if (check_atn) {
			if (_io->microSleepATN(80)) return 1;                  /* [$e9f8] */
		} else {
			_io->microSleep(80);
		}
		_io->set(_io->DATA, 0);                                        /* [$e9fa] */
		if (_io->wait(CLOCK, CLOCK)) return 0;               /* [$ea05] */
	}

	if (_io->wait(CLOCK, 0)) return 0;                    /* [$ea13] */
	b = (_io->get(DATA) >> DATA_BIT);
	if (_io->wait(CLOCK, CLOCK)) return 0;           /* [$ea22] */

	if (_io->wait(CLOCK, 0)) return 0;                    /* [$ea13] */
	b |= (_io->get(DATA) >> DATA_BIT) << 1;
	if (_io->wait(CLOCK, CLOCK)) return 0;           /* [$ea22] */

	if (_io->wait(CLOCK, 0)) return 0;                    /* [$ea13] */
	b |= (_io->get(DATA) >> DATA_BIT) << 2;
	if (_io->wait(CLOCK, CLOCK)) return 0;           /* [$ea22] */

	if (_io->wait(CLOCK, 0)) return 0;                    /* [$ea13] */
	b |= (_io->get(DATA) >> DATA_BIT) << 3;
	if (_io->wait(CLOCK, CLOCK)) return 0;           /* [$ea22] */

	if (_io->wait(CLOCK, 0)) return 0;                    /* [$ea13] */
	b |= (_io->get(DATA) >> DATA_BIT) << 4;
	if (_io->wait(CLOCK, CLOCK)) return 0;           /* [$ea22] */

	if (_io->wait(CLOCK, 0)) return 0;                    /* [$ea13] */
	b |= (_io->get(DATA) >> DATA_BIT) << 5;
	if (_io->wait(CLOCK, CLOCK)) return 0;           /* [$ea22] */

	if (_io->wait(CLOCK, 0)) return 0;                    /* [$ea13] */
	b |= (_io->get(DATA) >> DATA_BIT) << 6;
	if (_io->wait(CLOCK, CLOCK)) return 0;           /* [$ea22] */

	if (_io->wait(CLOCK, 0)) return 0;                    /* [$ea13] */
	b |= (_io->get(DATA) >> DATA_BIT) << 7;
	if (_io->wait(CLOCK, CLOCK)) return 0;           /* [$ea22] */

	_io->set(DATA, DATA);                                    /* [$ea28] */

	*ret = ~b;  	                   /* all bits we got need to be inverted */

	DPRINT(ret);

	if (eoi) {
		_io->set(CLOCK, CLOCK);                              /* c64: [$ee0d] */
		_io->set(DATA, DATA);                                /* c64: [$ee10] */
	}
	if (check_atn) {
		if (_io->microSleepATN(100)) return 1;               /* c64: [$ee0a] */
	} else {
		_io->microSleep(100);
	}

#ifdef USE_CLI
	//asm("sti\n\t");
#endif

	return eoi;
}

int Protocol::putIEC(byte_t b, int eoi, int check_atn)
{
	int a;
	byte_t tmp;
	int flag = 0;

	/*
	 *  DELAY c64: [$ee1b]
	 */

#ifdef USE_CLI
	//asm ("cli\n\t");
#endif /* USE_CLI */

	DPRINT(&b);
	_io->_timeout = 0;
	if (check_atn) {
		if (_io->microSleepATN(100)) return 1;
	} else {
		_io->microSleepATN(100);
	}

	tmp = _io->get(_io->DATA);                               /* [$e919] */
	_io->set(_io->CLOCK, 0);                    		 /* [$e91f] */

    	if (tmp) {                     			         /* [$e923] */
		if (_io->wait(_io->DATA, 0)) return 0;	         /* [$e92d] */
		flag = eoi;
		if (eoi) DPRINT("e");
	} else {
		DPRINT("=");
		DPRINT("1");
		flag = 1;			       /* [$e923] force eoi */
	}
	if (flag) {
		DPRINT("=");
		DPRINT("2");
		if (_io->wait(_io->DATA, 0)) return 0;		 /* [$e93f] */
		if (_io->wait(_io->DATA, _io->DATA)) return 0;	 /* [$e949] */
	}

	_io->set(_io->CLOCK, _io->CLOCK);        		 /* [$e94b] */
	if (_io->wait(_io->DATA, 0)) return 0; 		         /* [$e956] */

	for (a = 0;a < 8;a++) {

		/*
 		 *  DELAY c64: [$ee5a]
 		 */
		if (check_atn) {
			if (_io->microSleepATN(70)) return 1;
		} else {
			_io->microSleep(70);
		}

		if (_io->get(_io->DATA)) {
			DPRINT("~");
			return 1;			         /* [$e961] */
		}
		if (b & (1 << a)) {
			_io->set(_io->DATA, 0);     	         /* [$e973] */
		} else {
		        _io->set(_io->DATA, _io->DATA);          /* [$e96e] */
		}
		_io->set(_io->CLOCK, 0);     		         /* [$e976] */
		_io->microSleep(70);     /* [$e97d -> $fef3] delay for 1541 */

		/*
		 *  DELAY c64: [$ee67]
		 */

		_io->set(_io->CLOCK, _io->CLOCK);       /* [$e980 -> $fefb] */
		_io->set(_io->DATA, 0);   	        /* [$e980 -> $fefb] */
	}

	if (check_atn) {
		if (_io->microSleepATN(100)) return 1;
	} else {
		//_io->microSleep(100);
	}

	if (_io->wait(_io->DATA, _io->DATA)) return 0;		 /* [$e98f] */

#ifdef USE_CLI
	//asm("sti\n\t");
#endif

	return flag;
}

void Protocol::sendDirLine(word_t *addr, DirectoryEntry *de)
{
	word_t blocks;
	byte_t a, b, len;
	char name[17];
        const char *type;

	type   = de->type();
	blocks = de->blocks();
	sprintf(name, "\"%s\"", de->name());

	b = 1;
	if (blocks < 100) b++;
	if (blocks <  10) b++;
	len = strlen(name);
	*addr += 26 + b;

	putIEC(*addr);
	putIEC(*addr >> 8);
	putIEC(blocks);
	putIEC(blocks >> 8);
	if (_io->_timeout) return;
	for (a = 0;a < b;a++)        putIEC(' ');
	if (_io->_timeout) return;
	for (a = 0;a < len;a++)	     putIEC(name[a]);
	if (_io->_timeout) return;
	for (a = 0;a < 18 - len;a++) putIEC(' ', 0);
	if (_io->_timeout) return;
	for (a = 0;a < 5;a++)	     putIEC(type[a]);
	putIEC(0x00);
}

void Protocol::sendDir(Directory *dir, const char *pattern)
{
	byte_t c;
	word_t a, len, addr;

        *debug << "sending directory...\n";

	addr = 0x0801;
	putIEC(addr);				// load address low
	putIEC(addr >> 8);			// load address hi
	if (_io->_timeout) return;

	len = strlen(dir->title());
	addr += len + 5;
	putIEC(addr);				// link pointer low
	putIEC(addr >> 8);			// link pointer high
	putIEC(0x00);				// line number low
	putIEC(0x00);				// line number high
	if (_io->_timeout) return;

	putIEC('\x12');				// reverse
	putIEC('"');				// "
	for (a = 0;a < len;a++) {
		c = toupper(dir->title()[a]);
		if (c == '\\') c = '/';
		putIEC(c);			// directory name
		if (_io->_timeout) return;
	}
	putIEC('"');				// "
	putIEC(0x00);				// end of line
	if (_io->_timeout) return;

	for (Directory::iterator it = dir->begin();it != dir->end();it++) {
		if (pattern != NULL) {
			/*
			 *  if pattern is given we emulate the original
			 *  floppy behaviour and send only matching
			 *  directory entries
			 */
			if (!util_glob(pattern, (*it)->name())) {
				continue;
			}
		}
		sendDirLine(&addr, *it);
		if (_io->_timeout) return;
	}

	putIEC(addr);				// link pointer low
	putIEC(addr >> 8);			// link pointer high
        /*
         *  we can't have more than 65535 blocks free
         */
        if (dir->free() > 0xffff) {
        	putIEC(0xff);			// blocks free low
                putIEC(0xff);			// blocks free high
        } else {
	        putIEC(dir->free() & 0xff);
	        putIEC((dir->free() >> 8) & 0xff);
        }
        for (a = 0;a < 12;a++) {
        	putIEC("BLOCKS FREE."[a]);
        }
	putIEC(0x00);				// end of line

	putIEC(0x00);
	putIEC(0x00, 1);			// basic end, send with eoi
}

void Protocol::sendFile(File *file)
{
	int c2;
	byte_t c1;
        word_t b, c;

	if (file == NULL) return;

        *debug << "sending file (" << file->path() << ")...\n";

	c2 = file->getc();
	if (c2 == EOF) return;

        b = c = 0;
	while (1) {
		c1 = (byte_t)c2;
		c2 = file->getc();
		if (c2 == EOF) {
			if ((c % 8) != 0) debug->form("\n");
			DPRINT("-");
			DPRINT("E");
			DPRINT("O");
			DPRINT("F");
			DPRINT("-");
			putIEC(c1, 1);
			break;
		} else {
			putIEC(c1);
			if (_io->_timeout) return;
		}
		if ((b++ % 254) == 0) {
                	c++;
			debug->form("[%3d]%c", c, ((c % 8) == 0) ? '\n' : ' ');
		}
	}
}

void Protocol::receiveFile(Device *dev, char *name)
{
	int a;
	int eoi;
	byte_t b;
	File *f;

	debug->form("Protocol::receiveFile() -> %s\n", name);

	f = dev->open_write(name);
	if (f == NULL) {
		debug->form("Protocol::receiveFile(): write not supported!\n");
		return;
	}

	if (!f->ok()) {
		debug->form("Protocol::receiveFile(): can't write file!\n");
		delete f;
		return;
	}

	a = 0;
	do {
		if (_io->get(_io->ATN)) {
			DPRINT("*");
			DPRINT("A");
			DPRINT("*");
			return;
		}
		eoi = getIEC(&b);
		f->putc(b);
		if (_io->_timeout) break;
	} while (!eoi);

	delete f;
}

/*
 *  [$ea2e]
 *
 *  If iobuf != NULL we have an open command and the filename
 *  is following. The other case is a data transfer.
 *  FIXME: buffer overflow not tested!
 *
 */
void Protocol::handleLISTEN(Device * /* dev */, int sec_addr, char *iobuf)
{
	int a;
	int eoi;
	byte_t b;
	char command[1024]; /* fixme: */

	switch (sec_addr) {
	case 0:
        	debug->form("Protocol::handleLISTEN(): open -> '%s'\n",
 			iobuf?iobuf:"<null>");
		break;
	case 1: /* save */
		debug->form("Protocol::handleLISTEN(): save -> '%s'\n",
			iobuf?iobuf:"<null>");
		receiveFile(_dev, iobuf);
		_update = util_strdup(iobuf);
		/*
		a = 0;
		do {
			if (_io->get(_io->ATN)) {
				DPRINT("*");
				DPRINT("A");
				DPRINT("*");
				return;
			}
			eoi = getIEC(&b);
			if (iobuf) {
				iobuf[a++] = b;
				iobuf[a] = '\0';
			}
			if (_io->_timeout) return;
		} while (!eoi);
		*/
		break;
	case 15:
        	debug->form("Protocol::handleLISTEN(): command channel\n");

		/*
		 *  need a little pause here !?!
		 */

		a = 0;
		do {
			if (_io->get(_io->ATN)) {
				DPRINT("*");
				DPRINT("A");
				DPRINT("*");
				return;
			}
			eoi = getIEC(&b);
			command[a++] = b;
			if (_io->_timeout) break;
		} while (!eoi);
		command[a] = '\0';

		_dev->handle_command(command);
		/*
		for (size_t x = 0;x < strlen(iobuf);x++) {
			debug->form("%02x ", iobuf[x] & 0xff);
		}
		*debug << endl;
		*/

		break;
	default:
                debug->form("Protocol::handleLISTEN(): secondary address %d -> '%s'\n",
                	sec_addr, iobuf?iobuf:"<null>");
		break;
	}
}

void Protocol::handleTALK(Device *dev, int sec_addr, char *s)
{
	File *f;
	char c;
	ChannelState ch_state;
	static int set_error = 0;

	switch (sec_addr) {
	case 15:
		debug->form("Protocol::handleTALK(): command channel\n");

		if (set_error) {
			set_error = 0;
			dev->error("00, OK,00,00");
		}

		while ((c = dev->error_char()) != '\0') {
			if (putIEC(c, 0, 1)) return;
			if (_io->_timeout) return;
			dev->error_next();
		}
		if (putIEC(0x0d, 0, 1)) return; /* no eoi here! */
		dev->error_next();
		set_error = 1;
		break;
	default:
		ch_state = dev->get_channel_state(sec_addr);
		switch (ch_state) {
		case CHANNEL_DIRECT_ACCESS:
			while (1) {
				c = dev->channel_char(sec_addr);
				if (putIEC(c, 0, 1)) return;
				if (_io->_timeout) return;
				dev->channel_next(sec_addr);
			}
			break;
		default:
			dev->error("00, OK,00,00");
			debug->form("Protocol::handleTALK(): sec_addr = %d\n", sec_addr);
			if (s[0] == '$') {
				if (s[1] != '$') {
					if (strlen(s) > 1) {
						/*
						 *  LOAD"$xx",8
						 */
						sendDir(dev->readdir(), s + 1);
					} else {
						/*
						 *  only LOAD"$",8
						 */
						sendDir(dev->readdir());
					}
				} else {
					if (strlen(s) > 2) {
						/*
						 *  emulate old behavior where LOAD"$dir",8
						 *  does a chdir into dir first
						 *  the only difference is that now two '$'
						 *  are needed, e.g. LOAD"$$dir",8
						 */
						_gui->chdir(s + 2);
					}
					sendDir(dev->readdir());
				}
			} else {
				f = NULL;
				if (strcmp(s, "*") == 0) {
					/*
					 *  when opening "*" use the highlighted entry
					 */
					f = dev->open(_gui->get_selected());
				}
				if (!f) f = dev->open(s);
				if (f) {
					if (_deferred_delete) delete _deferred_delete;
					_deferred_delete = f;
					if (f->is_directory()) {
						_gui->chdir(f->name());
						sendDir(dev->readdir());
					} else {
						sendFile(f);
					}
				} else {
					debug->form("Protocol::handleTALK(): file not found! [%s]\n", s);
					dev->error("62, FILE NOT FOUND,00,00");
				}
			}
			break;
		}
	}
}

void Protocol::handleOPEN(Device * /* dev */, int sec_addr, char *iobuf)
{
	int a;
	int eoi;
	byte_t b;

	debug->form("Protocol::handleOPEN(): sec_addr = 0x%02x\n", sec_addr);
	a = 0;
	do {
		if (_io->get(_io->ATN)) {
			DPRINT("*");
			DPRINT("A");
			DPRINT("*");
			return;
		}
		eoi = getIEC(&b);
		iobuf[a++] = b;
		if (_io->_timeout) break;
	} while (!eoi);
	iobuf[a] = '\0';

	// debug->form("Protocol::handleOPEN(): -> %s\n", iobuf);
	switch (sec_addr) {
	case 0:
	case 1:
		break;
	case 15:
		break;
	default:
		if (iobuf[0] == '#') {
			_dev->open_direct_access(sec_addr, -1);
		}
		break;
	}
}

void Protocol::handleCLOSE(Device * /* dev */, int /* sec_addr */)
{
	// debug->form("Protocol::handleCLOSE(): sec_addr = %d\n", sec_addr);
}

void Protocol::putFL(byte_t val)
{
	int a;
	byte_t b;

	_io->set(_io->CLOCK, 0);
	_io->set(_io->DATA, _io->DATA);
/* sleep(1); */
	_io->microSleep(200);
	_io->microSleep(200);
	_io->microSleep(200);

	b = val >> 4; if (b > 9) b = 'A' + b - 10; else b += '0';
	DPRINT(&b);
	b = val & 0x0f; if (b > 9) b = 'A' + b - 10; else b += '0';
	DPRINT(&b);
	DPRINT("-");

	DPRINT("~"); DPRINT("1"); DPRINT("~");

	while (_io->get(_io->CLOCK));

	DPRINT("~"); DPRINT("2"); DPRINT("~");
/* sleep(2); */
	_io->set(_io->DATA, 0);

	DPRINT("~"); DPRINT("3"); DPRINT("~");
	while (!_io->get(_io->CLOCK));

	DPRINT("~"); DPRINT("4"); DPRINT("~");

	for (a = 0;a < 8;a++) {
		_io->microSleep(200);
		b = (val & 0x80) ? 0 : _io->DATA;
		b |= (a & 1) ? 0 : _io->CLOCK;
		_io->set(_io->DATA | _io->CLOCK, b);
		val <<= 1;
	}

	_io->microSleep(200);
	_io->microSleep(200);
	_io->microSleep(200);
	_io->microSleep(200);
	_io->microSleep(200);
	_io->microSleep(200);
	_io->microSleep(200);
	_io->microSleep(200);
}

void Protocol::handleFASTLOAD(Device *dev, int /* sec_addr */)
{
	int a;
	File *f;
	long size;
 	word_t val;
	byte_t b, c;

	DPRINT("=");
	DPRINT(">");
	_io->microSleep(200);
	_io->set(_io->CLOCK, 0);

	do {
		val = 0;
		c = 0;
		for (a = 0;a < 8;a++) {
			c ^= _io->CLOCK;
			do {
				b = _io->get(_io->CLOCK | _io->ATN);
				if (b & _io->ATN) return;
			} while ((b & _io->CLOCK) != c);
			b = _io->get(_io->DATA);
			/*
			if (b) {
				DPRINT("1");
			} else {
				DPRINT("0");
			}
			*/
			val <<= 1;
			val |= b;
		}
		val >>= _io->DATA_BIT;
		val = (val >> 3) | (val << 5);
		val &= 0xff;
		DPRINT(&val);
	} while (val != 0);
	DPRINT("+");
	DPRINT("+");
	DPRINT("+");
/* sleep(3); */
	_io->set(_io->CLOCK, _io->CLOCK);				/* CLOCK low */
	_io->microSleep(200);

	/*
	 *  send
	 */

	f = dev->open("XXX.BAS");
	if (f == NULL) {
		debug->form("Protocol::handleFASTLOAD(): file not found!\n");
		return;
	}

	size = f->size() - 3;
	if (size > 0xffff) {
		val = 0xffff;
	} else {
		val = size;
	}

	DPRINT("*");
	DPRINT("*");
	DPRINT("*");
/* sleep(3); */
	_io->set(_io->CLOCK, 0);
	_io->microSleep(100);

	/*
	 *  send size
	 */
	putFL(val & 0xff);
	putFL(val >> 8);

	/*
	 *  send start address
	 */
	putFL(f->getc());
	putFL(f->getc());

	/*
	 *  send file
	 */
	while ((a = f->getc()) != EOF) {
		putFL(a);
	}

	_io->microSleep(200);

	delete f;
}

/*
 *  [$e85b]
 */
void Protocol::handleATN(void)
{
	byte_t b, c;
	static char iobuf[4096]; /* fixme: */
	byte_t mode, sec_addr;

#ifdef USE_CLI
	//asm ("cli\n\t");
#endif /* USE_CLI */

	_io->_timeout = 0;

	_io->wait(_io->CLOCK, 0);             			       /* [$e882] */

	DPRINT("a");

	_io->microSleep(200);
	getIEC(&b);	  					     /* get device number */
	if (_io->_timeout) return;

	mode = b & IO::IEC_COMMAND;
	switch (mode) {
	case IO::IEC_UNLISTEN:
		DPRINT("L");
		_io->wait(_io->ATN, 0);
		DPRINT("A");
		return;
	case IO::IEC_UNTALK:
		DPRINT("T");
		_io->wait(_io->ATN, 0);
		DPRINT("A");
		return;
	case IO::IEC_LISTEN:
		DPRINT("l");
		/*
		 *  release bus if an other device is addressed
		 */
		if (_dev->id() != (b & IO::IEC_DEVICE)) {
			DPRINT("-");
			_io->set(_io->DATA | _io->CLOCK, 0);  	       /* [$e8ff] */
			while (_io->wait(_io->ATN, 0));	 	       /* [$e907] */
			DPRINT("A");
			return;
		}
		break;
	case IO::IEC_TALK:
		DPRINT("t");

		/*
		 *  release bus if an other device is addressed
		 */
		if (_dev->id() != (b & IO::IEC_DEVICE)) {
			DPRINT("-");
			_io->set(_io->DATA | _io->CLOCK, 0);  	       /* [$e8ff] */
			while (_io->wait(_io->ATN, 0));	 	       /* [$e907] */
			DPRINT("A");
			return;
		}
		break;
	}

	_io->microSleep(200);
	getIEC(&c);              		 /* get secondary address */

	if (_io->_timeout) return;

#ifdef USE_CLI
	//asm("sti\n\t");
#endif /* USE_CLI */

	switch (c & IO::IEC_MODE) {
	case IO::IEC_OPEN:
		_io->wait(_io->ATN, 0);					/* [$e907] */
		DPRINT("A");
		DPRINT("o");
		sec_addr = c & IO::IEC_ADDRESS;
		handleOPEN(_dev, c, iobuf);			/* [$e8e7 -> $ea2e] */
		DPRINT("O");
		break;
	case IO::IEC_CLOSE:
		// _io->wait(_io->ATN, 0);				/* [$e907] */
		DPRINT("A");
		DPRINT("c");
		sec_addr = c & IO::IEC_ADDRESS;
		handleCLOSE(_dev, sec_addr);				/* [$e8ce -> $dac0] */
		DPRINT("C");
		break;
	case IO::IEC_DATA:
		DPRINT("d");
		sec_addr = c & IO::IEC_ADDRESS;
		switch (mode) {
		case IO::IEC_LISTEN:
			_io->wait(_io->ATN, 0);				/* [$e907] */
			DPRINT("A");
			handleLISTEN(_dev, sec_addr, iobuf);		/* [$e8e7 -> $ea2e] */
			break;
		case IO::IEC_TALK:
			_io->set(_io->DATA | _io->CLOCK, 0);		/* [$e8ff] */
			_io->wait(_io->ATN, 0);				/* [$e907] */
			DPRINT("A");
			_io->microSleep(200); /* FIXME */
			_io->set(_io->DATA, 0);				/* [$e8f1] */
			_io->set(_io->CLOCK, _io->CLOCK);		/* [$e8f4] */
			_io->microSleep(200); /* FIXME */
			handleTALK(_dev, sec_addr, iobuf);		/* [$e8f7 -> $e909] */
                        //_io->microSleep(100);
			_io->set(_io->DATA | _io->CLOCK, 0);
			break;
		}
		DPRINT("D");
		break;
	case IO::IEC_FASTLOAD:
		_io->set(_io->DATA, 0);
		_io->set(_io->CLOCK, _io->CLOCK);
		_io->wait(_io->ATN, 0);
		DPRINT("A");

		DPRINT(" ");
		DPRINT(">");
		DPRINT("F");
		DPRINT("A");
		DPRINT("S");
		DPRINT("T");
		DPRINT("L");
		DPRINT("O");
		DPRINT("A");
		DPRINT("D");
		DPRINT("<");
		DPRINT(" ");
		sec_addr = c & IO::IEC_ADDRESS;
		handleFASTLOAD(_dev, sec_addr);
		return;
	}

	if (!_io->_timeout) return;
	DPRINT("*");
	DPRINT("T");
	DPRINT("*");
}

void Protocol::execute(void)
{
	int first = 1;

	while (1) {
		if (first) {
			_io->set(_io->DATA | _io->CLOCK, 0);
			first = 0;
			DPRINT("-");
			DPRINT(">");
			DPRINT(" ");
		}
		if (_io->get(_io->ATN)) {
#ifdef USE_CLI
			asm ("cli\n\t");
#endif /* USE_CLI */
			first = 1;
			_io->set(_io->CLOCK, 0);		/* [$e86d] */
			_io->set(_io->DATA, _io->DATA);		/* [$e870] */
			handleATN();
#ifdef MSDOS
			if (inportb(0x60) == 1) break;		/* check ESC key */
#endif
#ifdef USE_CLI
			asm("sti\n\t");
#endif /* USE_CLI */
			continue;
		}

                if (_deferred_delete) delete _deferred_delete;
                _deferred_delete = NULL;
		if (_update) {
 			_dev->update();
			_gui->set_selected(_update);
			delete _update;
			_update = NULL;
		}

		_io->microSleep(100);
		IDLE();
		if (!_gui->poll()) break;
#ifdef MSDOS
		//if (inportb(0x60) == 1) break;		/* check ESC key */
#endif /* MSDOS */
	}
	_io->set(_io->DATA | _io->CLOCK, 0);
#ifdef USE_CLI
	asm("sti\n\t");
#endif /* USE_CLI */
}

