/*
 *  $Id: debug.cc,v 1.1.1.1 1997/10/25 23:11:09 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <string.h>

#include "misc/debug.h"

using namespace std;

ostream *debug = &cerr;

