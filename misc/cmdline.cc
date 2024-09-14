/*
 *  $Id: cmdline.cc,v 1.4 1998/10/26 03:03:38 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <string.h>
#include <stdlib.h>
#include <iostream.h>
#include <iomanip.h>

#include "misc/util.h"
#include "misc/debug.h"
#include "misc/cmdline.h"

char * CmdLineArgs::_program = NULL;
CmdLineArgs * CmdLineArgs::_self = NULL;

CmdLineArgs::CmdLineArgs(int *argc, char ***argv)
{
	int a, b;
	bool found;

	if (_self != NULL) {
		cerr << "*** ERROR ***" << endl << endl
		     << "constructor for class CmdLineArgs called twice!" << endl;
		exit(1);
	}

	_self = this;

	_program = util_strdup((*argv)[0]);

	a = 1;
	while (a < *argc) {
		found = false;
		for (b = 0;_arg[b].name != NULL;b++) {
			if (strcmp(_arg[b].name, (*argv)[a]) != 0) continue;
			found = true;
			_arg[b].is_given = true;
			if (!_arg[b].has_arg) break;
			a++;
			if (a >= *argc) {
				debug->form("* Warning: missing parameter for option '%s'!\n",
					_arg[b].name);
				debug->form("           ignoring this option.\n");
				_arg[b].is_given = false;
				break;
			}
			if ((_arg[b].func == NULL)
 			    || (_arg[b].func(_arg[b].name, (*argv)[a]))) {
				_arg[b].arg = (*argv)[a];
			} else {
				_arg[b].is_given = false;
				_arg[b].arg = NULL;
			}
			break;
		}
		if (!found) {
			debug->form("* Warning: unknown option '%s'!\n", (*argv)[a]);
			debug->form("           ignoring this option.\n");
		}
		a++;
	}

	return;
}

CmdLineArgs::~CmdLineArgs(void)
{
	delete _program;
}

const CmdLineArgs * CmdLineArgs::getInstance(void)
{
	if (_self == NULL) {
		cerr << "*** ERROR ***" << endl << endl
		     << "constructor for class CmdLineArgs not called!" << endl;
		exit(1);
	}
	return _self;
}

const CmdArguments * CmdLineArgs::findArg(const char *arg)
{
	int a;

	for (a = 0;_arg[a].name != NULL;a++) {
		if (strcmp(_arg[a].name, arg) == 0) {
			if (_arg[a].is_given) return &_arg[a];
			return NULL;
		}
	}

	return NULL;
}

const char * CmdLineArgs::programName(void)
{
	return _program;
}

bool CmdLineArgs::isGiven(const char *name)
{
	const CmdArguments *arg = findArg(name);

	if (arg == NULL) return false;
	return arg->is_given;
}

int CmdLineArgs::getIntValue(const char *name)
{
	return getIntValue(name, 0);
}

int CmdLineArgs::getIntValue(const char *name, int def)
{
	const CmdArguments *arg = findArg(name);

	if (arg == NULL) return def;
	return strtol(arg->arg, NULL, 0);
}

const char * CmdLineArgs::getStringValue(const char *name)
{
	return getStringValue(name, NULL);
}

const char * CmdLineArgs::getStringValue(const char *name, const char *def)
{
	const CmdArguments *arg = findArg(name);

	if (arg == NULL) return def;
	return arg->arg;
}

void CmdLineArgs::showArgs(void)
{
	int a;
	int max, len;

	max = 0;
	for (a = 0;_arg[a].name != NULL;a++) {
		len = strlen(_arg[a].name);
		if (len > max) max = len;
	}
	cout << "Command line options:" << endl;
 	cout << endl;
	for (a = 0;_arg[a].name != NULL;a++) {
		cout.setf(ios::left);
		cout << "    " << setw(max) << _arg[a].name;
		if (_arg[a].desc != NULL) cout << "  " << _arg[a].desc;
		cout << endl;
	}
}

