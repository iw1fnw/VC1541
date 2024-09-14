/*
 *  $Id: guidebug.cc,v 1.4 1998/10/26 03:02:56 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <string.h>

#include "gui/guidebug.h"

TDebugWindow::TDebugWindow(TRect bounds) :
    TWindowInit(&TDebugWindow::initFrame),
    TWindow(bounds, "Debug Log", 0)
{
	_interior = makeInterior(bounds);
	insert(_interior);
}

TDebugWindow::~TDebugWindow(void)
{
}

int TDebugWindow::sync(void)
{
 	std::streamsize n = pptr() - pbase();
	return (n && _interior->addStr(pbase(), n) != n) ? EOF : 0;
}

int TDebugWindow::overflow(int c)
{
	std::streamsize n = pptr() - pbase();
	if (n && sync()) return EOF;
	if (c != EOF) {
		char cbuf[1];
		cbuf[0] = c;
		if (_interior->addStr(cbuf, 1) != 1) return EOF;
	}
	pbump(-n); // Reset pptr().
	return 0;
}

std::streamsize TDebugWindow::xsputn (const char *s, std::streamsize n)
{
	return sync() == EOF ? 0 : _interior->addStr(s, n);
}

TDebugScroller * TDebugWindow::makeInterior(TRect bounds)
{
    bounds = getExtent();
    bounds.grow(-1, -1);
    return new TDebugScroller(bounds,
        standardScrollBar(sbHorizontal | sbHandleKeyboard),
        standardScrollBar(sbVertical | sbHandleKeyboard));
};

TDebugScroller::TDebugScroller(const TRect &bounds, TScrollBar *hsb, TScrollBar *vsb) :
	TScroller(bounds, hsb, vsb)
{
	int a;

        growMode = gfGrowHiX | gfGrowHiY;
        options |= ofFramed;

        _buf = new char * [MAX_LINES];
        for (a = 0;a < MAX_LINES;a++) _buf[a] = NULL;
        _cur_line = _max_lines = _max_cols = 0;
        setLimit(200, 0);
}

TDebugScroller::~TDebugScroller(void)
{
	int a;

	for (a = 0;a < MAX_LINES;a++) {
		if (_buf[a] != NULL) delete[] _buf[a];
	}
	delete[] _buf;
}

void TDebugScroller::draw(void)
{
	int a, b;
	ushort color = getColor(0x0301);

        for (a = 0;a < size.y;a++) {
        	TDrawBuffer db;
                db.moveChar(0, ' ', color, size.x);
                if (_max_lines < size.y) {
                	b = delta.y + a;
                } else {
	                b = _cur_line - (_max_lines - delta.y) + a;
                }
                if (b < 0) b = MAX_LINES + b;
                if (_buf[b]) {
                	char s[1024];
                        if (delta.x > (int)strlen(_buf[b])) {
				s[0] = '\0';
                        } else {
                        	strncpy(s, _buf[b] + delta.x, size.x);
                                s[size.x] = '\0';
                        }
                        db.moveStr(0, s, color);
                }
                writeLine(0, a, size.x, 1, db);
	}
}

void TDebugScroller::addToCurrent(const char *s, int len)
{
	int a;
	char *tmp;

	if (_buf[_cur_line] == NULL) {
        	_buf[_cur_line] = new char[len + 1];
                strncpy(_buf[_cur_line], s, len);
                _buf[_cur_line][len] = '\0';
        } else {
        	a = strlen(_buf[_cur_line]) + len;
        	tmp = new char [a + 1];
                strcpy(tmp, _buf[_cur_line]);
                strncat(tmp, s, len);
                tmp[a] = '\0';
                delete[] _buf[_cur_line];
                _buf[_cur_line] = tmp;
        }
}

std::streamsize TDebugScroller::addStr(const char *s, std::streamsize n)
{
	int a, b;

        b = 0;

        while (1) {
		a = b;
	        while ((s[b] != '\n') && (s[b] != '\0') && (b < n)) b++;

	       	if (b > a) {
	       		addToCurrent(&s[a], b - a);
                }
	        if ((s[b] == '\0') || (b >= n)) break;
                b++;

	        if (++_cur_line >= MAX_LINES) _cur_line = 0;

		if (_cur_line > _max_lines) {
			_max_lines = _cur_line;
	                setLimit(200, _max_lines + 1);
	        }

		if (_buf[_cur_line] != NULL) {
	        	delete[] _buf[_cur_line];
                        _buf[_cur_line] = NULL;
	        }
	}

        scrollTo(0, _max_lines);
        drawView();

	return n;
}

