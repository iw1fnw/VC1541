/*
 *  $Id: main.cc,v 1.8 1998/10/26 03:02:43 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#ifdef MSDOS
#include <dpmi.h>
#endif /* MSDOS */

#include "misc/cmdline.h"

#include "gui/gui.h"
#include "misc/debug.h"
#include "misc/assert.h"
#include "vc1541/version.h"

#include "vc1541/cable.h"
#ifdef MSDOS
#include "vc1541/io_dos.h"
#endif /* MSDOS */
#ifdef LINUX
#include "vc1541/io_linux.h"
#endif /* LINUX */
#include "vc1541/protocol.h"
#include "libvfs/device.h"
#include "libvfs/fs_fact.h"

#ifdef LINUX
#define stricmp strcasecmp
#endif /* LINUX */

static char * abort_str = NULL;
static jmp_buf jmp_buf_sigill;
static volatile int have_tsc = 1;

static CmdLineArgs *args = NULL;

static bool constrain_lpt(const char *name, const char *arg)
{
	char *endp;

	int val = strtol(arg, &endp, 0);

	if ((val >= 1) && (val <= 4) && (endp[0] == '\0')) return true;
	*debug << "* Warning: invalid argument (" << arg << ") for option '" << name << "'!" << endl;
	*debug << "           ignoring this option." << endl;
	*debug << "           valid arguments: 1 - 4." << endl;
	return false;
}

static bool constrain_cable(const char *name, const char *arg)
{
	if (stricmp(arg, "X1541") == 0) return true;
	if (stricmp(arg, "XE1541") == 0) return true;
	*debug << "* Warning: invalid argument (" << arg << ") for option '" << name << "'!" << endl;
	*debug << "           ignoring this option." << endl;
	*debug << "           valid arguments: X1541, XE1541" << endl;
	return false;
}

static bool constrain_mhz(const char *name, const char *arg)
{
	char *endp;

	int val = strtol(arg, &endp, 0);

	if ((val >= 33) && (val <= 1000) && (endp[0] == '\0')) return true;
	*debug << "* Warning: invalid argument (" << arg << ") for option '" << name << "'!" << endl;
	*debug << "           ignoring this option." << endl;
	*debug << "           valid arguments: 33 - 1000." << endl;
	return false;
}

static bool constrain_speed(const char *name, const char *arg)
{
	char *endp;

	int val = strtol(arg, &endp, 0);

	if ((val >= 50) && (val <= 1000000) && (endp[0] == '\0')) return true;
	*debug << "* Warning: invalid argument (" << arg << ") for option '" << name << "'!" << endl;
	*debug << "           ignoring this option." << endl;
	*debug << "           valid arguments: 50 - 1000000." << endl;
	return false;
}

static bool constrain_dev(const char *name, const char *arg)
{
	char *endp;

	int val = strtol(arg, &endp, 0);

	if ((val >= 8) && (val <= 11) && (endp[0] == '\0')) return true;
	*debug << "* Warning: invalid argument (" << arg << ") for option '" << name << "'!" << endl;
	*debug << "           ignoring this option." << endl;
	*debug << "           valid arguments: 8 - 11." << endl;
	return false;
}

CmdArguments CmdLineArgs::_arg[] = {
	/*
	 *  name,  has_arg,  is_given,  arg, constrain_func
	 */
	{ "-h",       false,    false, NULL, NULL,
	  "show this help" },
	{ "-dev",      true,    false, NULL, constrain_dev,
	  "set device number" },
	{ "-dir",      true,    false, NULL, NULL,
	  "set startup directory" },
	{ "-lpt",      true,    false, NULL, constrain_lpt,
	  "set printer port" },
	{ "-cable",    true,    false, NULL, constrain_cable,
	  "force cable type (X1541 or XE1541)" },
	{ "-mhz",      true,    false, NULL, constrain_mhz,
	  "set processor speed to x MHz" },
	{ "-speed",    true,    false, NULL, constrain_speed,
	  "set processor speed index" },
	{ "-notsc",   false,    false, NULL, NULL,
	  "disable usage of the pentium timestamp counter (TSC)" },
	{ "-noabout", false,    false, NULL, NULL,
	  "suppress display of about dialog" },
	{ "-guionly", false,    false, NULL, NULL,
	  "run only the user interface (this is for debugging)" },
	{ NULL,       false,    false, NULL, NULL }
};

void
signal_handler_SIGSEGV(int /* sig */)
{
	sleep(2);
	abort_str = "Oops, caught signal SEGV!\nBye bye...";
	exit(1);
}

void
signal_handler_SIGILL(int /* sig */)
{
	*debug << " *** SIGILL ***";
	have_tsc = 0;
        longjmp(jmp_buf_sigill, 0);
}

static int
check_tsc(void)
{
	signal(SIGILL, signal_handler_SIGILL);
	setjmp(jmp_buf_sigill);

	if (have_tsc) {
		*debug << "- checking for 'rdtsc' instruction..." << flush;
		asm("rdtsc");
	        *debug << " ok.\n";
	} else {
	        *debug << " failed!\n";
	}

	signal(SIGILL, SIG_DFL);

	return have_tsc;
}

void showWarranty(void)
{
	cout << endl
	     << PROG_LONG_NAME " " PROG_VERSION ", " PROG_COPYRIGHT << endl
	     << endl
	     << PROG_NAME " comes with ABSOLUTELY NO WARRANTY, to the extent" << endl
	     << "permitted by applicable law. This is free software, and you" << endl
	     << "are welcome to redistribute it under certain conditions; see" << endl
	     << "the file 'COPYING' for details." << endl
	     << endl;
}

void showCredits(void)
{
	cout << "Credits:" << endl
	     << endl
	     << "\tWolfgang Moser, Nicolas Welte, Joe Forster/STA" << endl
	     << "\tPeter Schepers, Leopoldo Ghielmetti" << endl;
}

void main_exit(void)
{
	if (abort_str) {
		cerr << abort_str << endl;
	} else {
		showWarranty();
		if (args != NULL) {
			args->showArgs();
			delete args;
		} else {
			showCredits();
		}
		cout << endl;
	}
}

int main(int argc, char **argv)
{
	int lpt;
        int mhz;
	int port;
	int speed;
	int devno;
	bool noabout;
        const char *dir;
	const char *cable_type;
	int extended_cable;
	FSFactory fs_factory;

	signal(SIGSEGV, signal_handler_SIGSEGV);

	GUI *gui = new App();
	args = new CmdLineArgs(&argc, &argv);

	if (args->isGiven("-h") || args->isGiven("-help")) {
		delete gui;
		exit(0);
	}

	noabout = args->isGiven("-noabout");
	if (!noabout) ((App *)gui)->about();

	lpt     = args->getIntValue("-lpt");
        mhz     = args->getIntValue("-mhz");
	speed   = args->getIntValue("-speed");
	devno   = args->getIntValue("-dev", 8);
#ifdef MSDOS
        dir = args->getStringValue("-dir", "C:");
#endif /* MSDOS */
#ifdef LINUX
	dir = getenv("HOME");
	if (dir == NULL) dir = "/";
        dir = args->getStringValue("-dir", dir);
#endif /* LINUX */
#ifdef MSDOS
	cable_type = args->getStringValue("-cable");

	Cable *cable = new Cable(gui, new LPT());

	port = cable->detect(lpt, cable_type);
#endif /* MSDOS */

#ifdef MSDOS
	IO *io = NULL;
	if (port != 0) {
		extended_cable = 0;
		switch (cable->get_type(port)) {
		case CABLE_XE1541:
			extended_cable = 1;
			break;
		default:
			break;
		}

		if (!args->isGiven("-notsc") && check_tsc()) {
			if (args->isGiven("-speed")) {
				debug->form("* Warning: option '-speed' is ignored,");
				debug->form(" use '-mhz' instead.\n");
			}
			io = new IO_DOS_TSC(mhz, port, extended_cable);
		} else {
			if (args->isGiven("-mhz")) {
				debug->form("* Warning: option '-mhz' is ignored,");
				debug->form(" use '-speed' instead.\n");
			}
			io = new IO_DOS(speed, port, extended_cable);
		}
	}
#endif /* MSDOS */
#ifdef LINUX
	IO       *io   = new IO_Linux(mhz, 0x378);
#endif /* LINUX */

	*debug << "+ device number is " << devno << endl;
	Device   *dev  = new Device(gui, &fs_factory, devno);
        dev->chdir(dir);

	Protocol *prot = new Protocol(io, gui, dev);

	((App *)gui)->newWindow(dev);
#ifdef MSDOS
	if ((port == 0) || (args->isGiven("-guionly"))) {
		((App *)gui)->run();
	} else {
		prot->execute();
	}

#endif /* MSDOS */
#ifdef LINUX
	((App *)gui)->run();
#endif /* LINUX */

#ifdef MSDOS
	delete cable;
#endif /* MSDOS */
	delete prot;
	delete dev;
	delete io;
	delete args;
	delete gui;

	args = NULL;

	return 0;
}
