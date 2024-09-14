/*
 *  $Id: gui.h,v 1.4 1998/10/26 03:02:59 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __gui_gui_h
#define __gui_gui_h

#include <stdio.h>
#include <string.h>

#define Uses_TScreen
#define Uses_TEventQueue
#define Uses_TDialog
#define Uses_TKeys
#define Uses_TDeskTop
#define Uses_TApplication
#define Uses_TMenuBar
#define Uses_TSubMenu
#define Uses_TMenuItem
#define Uses_TRadioButtons
#define Uses_TSItem
#define Uses_TRect
#define Uses_TLabel
#define Uses_TInputLine
#define Uses_TButton
#define Uses_TWindow
#define Uses_TEvent
#define Uses_TStatusDef
#define Uses_TScrollBar
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TScroller
#define Uses_TListViewer
#define Uses_TListBox
#define Uses_TTerminal
#define Uses_otstream
#define Uses_TStringCollection
#define Uses_TPalette
#include <tv.h>

#include "libvfs/dir.h"
#include "libvfs/device.h"

const int GUI_KEY_F1 = 0xf1;
const int GUI_KEY_F2 = 0xf2;
const int GUI_KEY_F3 = 0xf3;

class GUI
{
protected:
	Device *_dev;
public:
	GUI(void) { _dev = NULL; }
	virtual ~GUI(void) {}
	virtual bool poll(void) = 0;

	virtual int select(const char *title, const char *label, const char **values) = 0;

	virtual void chdir(const char *dir) = 0;
	virtual const char * get_selected(void) = 0;
	virtual void set_selected(int idx) = 0;
	virtual void set_selected(const char *name) = 0;

	virtual void suspend(void) {};
	virtual void resume(void) {};
};

class TChangeableText : public TView
{
public:
    TChangeableText(const TRect& bounds, const char *aText);
    ~TChangeableText();

    virtual void draw();
    virtual TPalette& getPalette() const;
    virtual void getText(char *);
    virtual void setText(const char *);
protected:
    char *text;
};

class TDirTree : public TView
{
private:
        Device    *_dev;
public:
        TDirTree(const TRect &r, Device *dev);

        virtual void update(void);
        virtual void draw(void);
};

class TDirList : public TListViewer
{
private:
	Device    *_dev;
	Directory *_dir;
public:
	TDirList(const TRect &r, TScrollBar *vScrollBar, Device *dev);
#ifdef MSDOS
	void getText(char *dest, ccIndex item, short maxLen);
#endif /* MSDOS */
#ifdef LINUX
	void getText(char *dest, short item, short maxLen);
#endif /* LINUX */
	void handleEvent(TEvent &event);

	virtual const char * getSelected(void);
	virtual void update(int idx = 0);
	virtual void update(const char *name);
};

class TSectorView : public TView
{
private:
	static int _stab[36];
	int _track;
	int _sector;
	byte_t _buf[256];
	Device *_dev;
public:
	TSectorView(const TRect &bounds, Device *dev);
	virtual void draw();

	bool read_sector(int t, int s);
	bool set_track_sector(int t, int s);
	void goto_sector(void);
	void follow_sector(void);
	void next_sector(void);
	void previous_sector(void);
	void next_track(void);
	void previous_track(void);
};

class TSectorWindow : public TDialog
{
private:
	TInputLine *_il_t;
	TInputLine *_il_s;
	TSectorView *_view;
public:
	TSectorWindow(const TRect &bounds, const char *title, Device *dev);
	void handleEvent(TEvent &event);
};

class App : public GUI, public TApplication
{
private:
        TDirList        *_listbox;
        TDirTree        *_dirtree;
        TChangeableText *_label;
        TChangeableText *_free;
        TWindow         *_termwin;
        TTerminal       *_terminal;

	static TMenuBar * initMenuBar(TRect r);
	static TStatusLine * App::initStatusLine(TRect r);
	void handleEvent(TEvent &event);
public:
	App(void);
	virtual ~App(void);
	void about(void);
	void newWindow(Device *dev);
	void showSpecial(char *str);
	virtual bool poll(void);

	virtual int select(const char *title, const char *label, const char **values);

	virtual void chdir(const char *dir);
	virtual const char * get_selected(void);
	virtual void set_selected(int idx);
	virtual void set_selected(const char *name);

	virtual void suspend(void);
	virtual void resume(void);
	virtual void shutDown(void);
};

#endif /* __gui_gui_h */

