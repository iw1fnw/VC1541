/*
 *  $Id: exithook.cc,v 1.1 1998/10/26 03:04:19 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

extern void main_exit(void);

class ExitHook
{
public:
	ExitHook() {}
	~ExitHook() { main_exit(); }
};

static ExitHook exit_hook;
