/*
 *  $Id: gui.cc,v 1.7 1998/10/26 03:02:54 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include "gui/gui.h"
#include "gui/guidebug.h"
#include "misc/util.h"
#include "misc/assert.h"
#include "vc1541/version.h"


#define _MODULE_ "GUI: "

const short cmNewWindow         = 199;
const short cmChdir		= 200;
const short cmShowAbout         = 201;
const short cmSectorView	= 202;
const short cmSetTrackSector    = 203;

#define cpChangeableText "\x06"

TChangeableText::TChangeableText( const TRect& bounds, const char *aText ) :
    TView( bounds ),
    text( newStr( aText ) )
{
}

TChangeableText::~TChangeableText()
{
    delete (char *)text;
}

void TChangeableText::draw()
{
    uchar color;
    Boolean center;
    int i, j, l, p, y;
    TDrawBuffer b;
    char s[256];

    color = getColor(1);
    getText(s);
    l = strlen(s);
    p = 0;
    y = 0;
    center = False;
    while (y < size.y)
        {
        b.moveChar(0, ' ', color, size.x);
        if (p < l)
            {
            if (s[p] == 3)
                {
                center = True;
                ++p;
                }
            i = p;
            do {
               j = p;
               while ((p < l) && (s[p] == ' ')) 
                   ++p;
               while ((p < l) && (s[p] != ' ') && (s[p] != '\n'))
                   ++p;
               } while ((p < l) && (p < i + size.x) && (s[p] != '\n'));
            if (p > i + size.x)
                if (j > i)
                    p = j; 
                else 
                    p = i + size.x;
            if (center == True)
               j = (size.x - p + i) / 2 ;
            else 
               j = 0;
            b.moveBuf(j, &s[i], color, (p - i));
            while ((p < l) && (s[p] == ' '))
                p++;
            if ((p < l) && (s[p] == '\n'))
                {
                center = False;
                p++;
                if ((p < l) && (s[p] == 10))
                    p++;
                }
            }
        writeLine(0, y++, size.x, 1, b);
        }
}

TPalette& TChangeableText::getPalette() const
{
	static TPalette palette(cpChangeableText, sizeof(cpChangeableText) - 1);
	return palette;
}

void TChangeableText::getText(char *s)
{
	if (text == 0) {
		*s = '\0';
	} else {
	        strcpy(s, text);
	}
}

void TChangeableText::setText(const char *s)
{
	delete (char *)text;
	text = newStr(s);
	drawView();
}

TDirTree::TDirTree(const TRect &r, Device *dev) : TView(r)
{
        _dev = dev;
}

void TDirTree::update(void)
{
        drawView();
}

void TDirTree::draw(void)
{
        int x, y;
        char *ptr, *ptr2;
        ushort color = getColor(0x0301);
        const char *path = _dev->path();
        char *buf = util_strdup(path);
        TDrawBuffer b;

        TView::draw();
        
        x = -2;
        y = 0;
        if (buf[0] == '/') {
                b.moveStr(0, "/", color);
                writeLine(0, y++, 1, 1, b);
                x += 2;
        }
        
        ptr = buf;
        while (1) {
                while (*ptr == '/') ptr++;
                if (*ptr == '\0') break;
                
                ptr2 = strchr(ptr, '/');
                if (ptr2 != NULL) {
                        *ptr2 = '\0';
                }

                if (x < 0) {
                        b.moveStr(0, ptr, color);
                        writeLine(0, y++, strlen(ptr), 1, b);
                } else {
                        b.moveStr(0, "+-", color);
                        b.moveStr(2, ptr, color);
                        writeLine(x, y++, strlen(ptr) + 2, 1, b);
                }
                x += 2;

                if (ptr2 == NULL) break;
                ptr = ptr2 + 1;
        }
        
        delete buf;
        delete path;
}

TDirList::TDirList(const TRect &r, TScrollBar *vScrollBar, Device *dev) :
		TListViewer(r, 1, 0, vScrollBar)
{
	_dev = dev;
	update();
}

void TDirList::update(int idx)
{
	_dir = _dev->readdir();

	setRange(_dir->size());
	focusItem(idx);
	drawView();
}

void TDirList::update(const char *name)
{
	int a, idx;

	_dir = _dev->readdir();
	setRange(_dir->size());
	a = 0;
	idx = 0;
	for (Directory::iterator it = _dir->begin();it != _dir->end();it++) {
		if (strcmp(name, (*it)->name()) == 0) {
			idx = a;
			break;
		}
		a++;
	}
	focusItem(idx);
	drawView();
}

#ifdef MSDOS
void TDirList::getText(char *dest, ccIndex item, short maxLen)
#endif /* MSDOS */
#ifdef LINUX
void TDirList::getText(char *dest, short item, short maxLen)
#endif /* LINUX */
{
	int a;
	char buf[100];

	ASSERT(maxLen > 10);

	a = 0;
	Directory::iterator it;
	for (it = _dir->begin();(a < item) && (it != _dir->end());it++, a++);

	sprintf(buf, "%-5ld %-16.16s %-5.5s", (*it)->blocks(), (*it)->name(), (*it)->type());
	maxLen--;
	strncpy(dest, buf, maxLen);
	dest[maxLen] = '\0';
}

const char * TDirList::getSelected(void)
{
	int a;

	a = 0;
	Directory::iterator it;
	for (it = _dir->begin();(a < focused) && (it != _dir->end());it++, a++);

	if (*it == NULL) return NULL;
	return (*it)->name();
}

void TDirList::handleEvent(TEvent &event)
{
	TListViewer::handleEvent(event);
	if (event.what == evKeyDown) {
		switch (event.keyDown.keyCode) {
		case kbEnter:
		case kbRight:
			selectItem(focused);
		        clearEvent(event);
			break;
		case kbLeft:
			event.what = evCommand;
	                event.message.command = cmChdir;
        	        event.message.infoPtr =	(char *)"..";
	                putEvent(event);
		        clearEvent(event);
			break;
		}
	}
        if (event.what == evBroadcast) {
        	switch (event.message.command) {
                case cmListItemSelected:
	                event.what = evCommand;
	                event.message.command = cmChdir;
                                /* argh */
        	        event.message.infoPtr = (char *)getSelected();
	                putEvent(event);
		        clearEvent(event);
                        break;
		}
	}
}

App::App(void) :
	TProgInit(&App::initStatusLine, &App::initMenuBar, &App::initDeskTop)
{
	eventMask |= evBroadcast;
	options |= ofPostProcess;

	_listbox = NULL;

	TRect r = deskTop->getExtent();
        r.a.y = r.b.y - 7;
        TDebugWindow *dw = new TDebugWindow(r);
        dw->flags &= ~wfClose;
        debug = new debugstream(dw);
        deskTop->insert(dw);
}

App::~App(void)
{
	shutDown();
}

void App::about(void)
{
	const char *buf = "\003" PROG_LONG_NAME " " PROG_VERSION "\n"
			  "\003\n"
			  "\003 compiled: " __TIME__ ", " __DATE__ "\n"
			  "\003\n"
		          "\003\n"
		          "\003" PROG_COPYRIGHT "\n"
		          "\003" PROG_EMAIL "\n";

	TDialog *dialog = new TDialog(TRect(0, 0, 60, 15), "About VC1541");
	dialog->options |= ofCentered;
	TStaticText *text = new TStaticText(TRect(0, 0, 50, 9), buf);
	text->options |= ofCentered;
	dialog->insert(text);
	dialog->insert(new TButton(TRect(25, 11, 35, 13), "~O~K", cmOK,
		       bfDefault));
	TProgram::deskTop->execView(dialog);
	destroy(dialog);
}

void App::newWindow(Device *dev)
{
	TRect r;
	char buf[100];

        _dev = dev;

	if (_listbox != NULL) return;

	r = deskTop->getExtent();
	r.b.y = r.b.y - 7;
        r.b.x /= 2;
        r.b.x--;
	sprintf(buf, "Device %d", _dev->get_id());
	TDialog *v1 = new TDialog(r, buf);
	v1->flags &= ~wfClose;

	r = v1->getExtent();
	r.grow(-2, -2);
	r.b.x--;
	TRect r2(r);
	r2.a.x = r2.b.x++;
        TScrollBar *v4 = new TScrollBar(r2);
        _listbox = new TDirList(r, v4, _dev);
	_label = new TChangeableText(TRect(2, 1, 30, 2), _dev->fstype());
        r.a.y = r.b.y++;
        _free = new TChangeableText(r, "");

        v1->insert(v4);
        v1->insert(_label);
        v1->insert(_listbox);
        v1->insert(_free);

        r = deskTop->getExtent();
        r.b.y = r.b.y - 7;
        r.a.x = r.b.x / 2;
        r.a.x++;
        TDialog *v2 = new TDialog(r, "Directory Tree");
        v2->flags &= ~wfClose;

        r = v2->getExtent();
        r.grow(-2, -2);
        _dirtree = new TDirTree(r, _dev);

        v2->insert(_dirtree);

#define __DO_CHDIR(x) \
	do { \
		TEvent event; \
		event.what = evCommand; \
		event.message.command = cmChdir; \
		event.message.infoPtr =	x; \
		handleEvent(event); \
	} while (0)

#ifdef MSDOS
	/*
	for (int a = 0;a < 100;a++) {
		__DO_CHDIR("64ERGAME.ZIP");
		__DO_CHDIR("..");
	}
	*/
#endif /* MSDOS */
#ifdef LINUX
	/*
	_dev->chdir("/");
	__DO_CHDIR("home");
	__DO_CHDIR("packages");
	__DO_CHDIR("c64");
	__DO_CHDIR("demos");
	__DO_CHDIR("padua");
	{
		int b = 0, c = 0;
		for (int a = 0;a < 200;a++) {
			__DO_CHDIR("torture3_side1.d64.gz");
			__DO_CHDIR("..");
			if (++b == 10) {
				b = 0;
				c++;
				*debug << ".";
			}
			if (c == 50) {
				c = 0;
				*debug << endl;
			}
		}
	}
	*/
#endif /* LINUX */
#undef __DO_CHDIR

        deskTop->insert(v2);
        deskTop->insert(v1);
}

void App::chdir(const char *dir)
{
	int idx;
        char buf[100];

	if ((dir != NULL) && ((idx = _dev->chdir(dir)) >= 0)) {
		_listbox->update(idx);
		_dirtree->update();
		_label->setText(_dev->fstype());
		sprintf(buf, "%ld blocks free.", _dev->readdir()->free());
		_free->setText(buf);
	}
}

void App::set_selected(int idx)
{
	_listbox->update(idx);
}

void App::set_selected(const char *name)
{
	_listbox->update(name);
}

const char * App::get_selected(void)
{
	return _listbox->getSelected();
}

void App::handleEvent(TEvent &event)
{
	TApplication::handleEvent(event);
        if (event.what == evCommand) {
        	switch (event.message.command) {
                case cmNewWindow:
		        clearEvent(event);
                        break;
		case cmShowAbout:
			about();
			break;
		case cmSectorView:
			TView *v;

			v = new TSectorWindow(TRect(0, 0, 75, 20), "Sector View", _dev);
			v->options |= ofCentered;
			deskTop->execView(v);
			break;
		case cmChdir:
			chdir((const char *)event.message.infoPtr);
		        clearEvent(event);
			break;
		}
	}
}

TMenuBar * App::initMenuBar(TRect r)
{
	r.b.y = r.a.y + 1;
	return new TMenuBar(r,

		* new TSubMenu("~F~ile", kbAltF) +
		* new TMenuItem("~A~bout", cmShowAbout, kbF1, hcNoContext, "F1") +
		* new TMenuItem("~S~ector View", cmSectorView, kbF3, hcNoContext, "F3") +
		  newLine() +
		* new TMenuItem("E~x~it", cmQuit, kbNoKey, hcNoContext, 0) +

		* new TSubMenu("~W~indow", kbAltW) +
		* new TMenuItem("~Z~oom", cmZoom, kbF5, hcNoContext, "F5") +
		* new TMenuItem("~N~ext", cmNext, kbF6, hcNoContext, "F6"));
}

TStatusLine * App::initStatusLine(TRect r)
{
	r.a.y = r.b.y - 1;
        return new TStatusLine(r,
        	*new TStatusDef(0, 0xffff) +
                *new TStatusItem("~Alt-X~ Exit", kbAltX, cmQuit) +
		*new TStatusItem("~F1~ About", kbF1, cmShowAbout) +
		*new TStatusItem("~F3~ Sector View", kbF3, cmSectorView) +
                *new TStatusItem(0, kbF10, cmMenu) +
                *new TStatusItem(0, kbAltF3, cmClose) +
                *new TStatusItem(0, kbF5, cmZoom) +
                *new TStatusItem(0, kbCtrlF5, cmResize) +
                *new TStatusItem(0, kbF6, cmNext) +
                *new TStatusItem(0, kbShiftF6, cmPrev));
}

bool App::poll(void)
{
	TEvent e;
	getEvent(e);
	handleEvent(e);
	if (e.what != evNothing) eventError(e);
	if (endState == 0) return true;
	if (!valid(endState)) return true;
	return false;
}

int App::select(const char *title, const char *label, const char **values)
{
	char *l;
	ushort value;
	char *label_tmp;
	int a, len, lines;
	TDialog *d;

	lines = 1;
	for (a = 0;label[a] != '\0';a++) if (label[a] == '\n') lines++;
	for (len = 0;values[len] != NULL;len++);

	value = 0;
	d = new TDialog(TRect(0, 0, 40, 7 + lines + len), title);
	if (d) {
		TSItem *item = NULL;

		for (a = len - 1;a >= 0;a--) {
			item = new TSItem(values[a], item);
		}

		TView *v = new TRadioButtons(TRect(2, 3 + lines, 38, 3 + lines + len), item);
		d->insert(new TButton(TRect(15, 4 + lines + len, 25, 6 + lines + len),
 			"~O~K", cmOK, bfDefault));

		label_tmp = util_strdup(label);
		l = strtok(label_tmp, "\n");
		for (a = 0;a < lines;a++) {
			d->insert(new TLabel(TRect(1, 2 + a, 38, 3 + a), l, v));
			l = strtok(NULL, "\n");
		}
		delete label_tmp;
		d->insert(v);
		d->options |= ofCentered;
		deskTop->execView(d);
		v->getData(&value);
	}
	destroy(d);

	return value;
}

void App::suspend(void)
{
	TApplication::suspend();
}

void App::resume(void)
{
	TApplication::resume();
}

void App::shutDown(void)
{
	debug = &cerr;
	TApplication::shutDown();
}

TSectorWindow::TSectorWindow(const TRect &bounds, const char *title, Device *dev)
	: TDialog(bounds, title),
	  TWindowInit(&TSectorWindow::initFrame)
{
	_view =	new TSectorView(TRect(1, 3, 73, 19), dev);
	insert(_il_s = new TInputLine(TRect(24, 1, 29, 2), 3));
	insert(new TLabel(TRect(15, 1, 23, 2), "~S~ector:", _il_s));
	insert(_il_t = new TInputLine(TRect(9, 1, 14, 2), 3));
	insert(new TLabel(TRect(1, 1, 8, 2), "~T~rack:", _il_t));
	insert(_view);
	_view->set_track_sector(18, 0);
	_il_t->setData((char *)"18");
	_il_s->setData((char *)"0");
}

void TSectorWindow::handleEvent(TEvent &event)
{
	if (event.what == evKeyDown) {
		switch (event.keyDown.keyCode) {
                case kbEsc:
			event.what = evCommand;
			event.message.command = cmCancel;
			event.message.infoPtr = 0;
			putEvent(event);
			clearEvent(event);
			break;
		case kbRight:
			_view->next_sector();
			clearEvent(event);
			break;
		case kbLeft:
			_view->previous_sector();
			clearEvent(event);
			break;
		case kbUp:
			_view->next_track();
			clearEvent(event);
			break;
		case kbDown:
			_view->previous_track();
			clearEvent(event);
			break;
		case kbEnter:
			int t, s;
			char buf[10];
			_il_t->getData(buf);
			t = atoi(buf);
			_il_s->getData(buf);
			s = atoi(buf);
			_view->set_track_sector(t, s);
			clearEvent(event);
			break;
		}
		switch (event.keyDown.charScan.charCode) {
		case 'n':
			_view->follow_sector();
			clearEvent(event);
			break;
		case 'd':
			_view->set_track_sector(18, 0);
			clearEvent(event);
			break;
		}
		if (event.keyDown.charScan.charCode >= ' ') {
			if (event.keyDown.charScan.charCode < '0') clearEvent(event);
		    	if (event.keyDown.charScan.charCode > '9') clearEvent(event);
		}
	}
	if (event.what == evCommand) {
        	switch (event.message.command) {
                case cmSetTrackSector:
			char buf[10];
			sprintf(buf, "%d", (ushort)(ulong)event.message.infoPtr >> 8);
			_il_t->setData(buf);
			sprintf(buf, "%d", (ushort)(ulong)event.message.infoPtr & 0xff);
			_il_s->setData(buf);
			break;
		}
	}

	TDialog::handleEvent(event);
}


int TSectorView::_stab[] = {
	0,
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	21, 21, 21, 21, 21, 21, 21,
	19, 19, 19, 19, 19, 19, 19,
	18, 18, 18, 18, 18, 18,
	17, 17, 17, 17, 17
};

TSectorView::TSectorView(const TRect &bounds, Device *dev) : TView(bounds)
{
	_dev = dev;

	_track = 18;
	_sector = 0;
}

bool TSectorView::read_sector(int t, int s)
{
	if (!_dev->read_sector(_buf, t, s)) {
		return false;
	}
	return true;
}

void TSectorView::follow_sector(void)
{
	int t, s;

	t = _buf[0];
	s = _buf[1];

	if ((t < 1) || (t > 35)) return;
	if ((s < 0) || (s >= _stab[t])) return;
	if (read_sector(t, s)) {
		_track = t;
		_sector = s;
	}
	drawView();
}

void TSectorView::next_sector(void)
{
	int s = _sector + 1;
	int t = _track;

	if (s >= _stab[t]) {
		s = 0;
		t++;
	}
	if (t > 35) return;
	if (read_sector(t, s)) {
		_track = t;
		_sector = s;
	}
	drawView();
}

void TSectorView::previous_sector(void)
{
	int s = _sector - 1;
	int t = _track;

	if (s < 0) {
		t--;
		if (t < 1) return;
		s = _stab[t] - 1;
	}
	if (read_sector(t, s)) {
		_track = t;
		_sector = s;
	}
	drawView();
}

void TSectorView::next_track(void)
{
	int s = _sector;
	int t = _track;

	if (t >= 35) return;
	t++;
	if (s >= _stab[t]) {
		s = _stab[t] - 1;
	}
	if (read_sector(t, s)) {
		_track = t;
		_sector = s;
	}
	drawView();
}

void TSectorView::previous_track(void)
{
	int s = _sector;
	int t = _track;

	if (t < 2) return;
	t--;

	if (read_sector(t, s)) {
		_track = t;
		_sector = s;
	}
	drawView();
}

bool TSectorView::set_track_sector(int t, int s)
{
	bool ret = true;

	if ((t < 1) || (t > 35)) ret = false;
	if ((s < 0) || (s >= _stab[t])) ret = false;

	if (ret && read_sector(t, s)) {
		_track = t;
		_sector = s;
	}
	drawView();

	return ret;
}

void TSectorView::draw(void)
{
	int a, b;
	int off;
	char buf[30];
	TDrawBuffer db;
	ushort color = getColor(0x0301);
	ushort color_hl = getColor(0x0302);
	ushort color_hl2 = getColor(0x0309);

	TView::draw();

	for (a = 0;a < 16;a++) {
		sprintf(buf, "%02x", 16 * a);
		db.moveStr(0, buf, color_hl);
		db.moveStr(2, ": ", color_hl2);
		off = 4;
		for (b = 0;b < 16;b++) {
			sprintf(buf, "%02x ", _buf[16 * a + b]);
			db.moveStr(off, buf, color);
			off += 3;
			if (b == 7) {
				db.moveStr(off, "- ", color_hl2);
				off += 2;
			}
		}
		db.moveStr(off, "| ", color_hl2);
		off += 2;
		for (b = 0;b < 16;b++) {
			db.putChar(off, _buf[16 * a + b]);
			db.putAttribute(off, color);
			off++;
		}
		writeLine(1, a, 71, 1, db);
	}

	ushort ts;
	TEvent event;
	ts = _track << 8 | _sector;
	event.what = evCommand;
	event.message.command = cmSetTrackSector;
	event.message.infoPtr = (void *)ts;
	putEvent(event);
}
