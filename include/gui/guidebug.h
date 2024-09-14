/*
 *  $Id: guidebug.h,v 1.3 1997/12/16 01:52:08 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __gui_guidebug_h
#define __gui_guidebug_h

#define Uses_TRect
#define Uses_TWindow
#define Uses_TScroller
#define Uses_TScrollBar
#include <tv.h>

#include "misc/debug.h"
#include "misc/vector.h"

class TDebugScroller : public TScroller
{
private:
	static const int MAX_LINES = 200;

        int    _cur_line;
        int    _max_lines;
        int    _max_cols;
	char **_buf;
protected:
	void addToCurrent(const char *s, int len);
public:
	TDebugScroller(const TRect &bounds, TScrollBar *hsb, TScrollBar *vsb);
	virtual ~TDebugScroller(void);
        virtual void draw(void);

	streamsize addStr(const char *s, streamsize n);
};

class TDebugWindow : public streambuf, public TWindow
{
private:
	TDebugScroller *_interior;
protected:
	TDebugScroller *makeInterior(TRect bounds);
public:
	TDebugWindow(TRect bounds);
	virtual ~TDebugWindow(void);

	int sync(void);
	int overflow (int c);
         // Defining xsputn is an optional optimization.
         // (streamsize was recently added to ANSI C++, not portable yet.)
	streamsize xsputn(const char *s, streamsize n);
};

class debugstream : public ostream
{
public:
	debugstream(TDebugWindow *dw) : ostream(dw) {}
};

#endif /* __gui_guidebug_h */
