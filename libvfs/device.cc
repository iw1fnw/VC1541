/*
 *  $Id: device.cc,v 1.5 1998/10/26 03:02:08 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifdef MSDOS
#include "ctype.h"
#endif /* MSDOS */

#include "gui/gui.h"

#include "misc/util.h"
#include "misc/debug.h"

#include "libvfs/device.h"
#include "libvfs/dev_dos.h"
#include "libvfs/fs_fact.h"


/* #define DEBUG_CHDIR */
#define _MODULE_ "Device: "

Device::Device(GUI *gui, FSFactory *fs_factory, int id)
{
	int a;

	_gui = gui;

	_id = id;
	_error = util_strdup("73,CBM DOS V2.6 1541,00,00");
	_error_ptr = _error;
	_fs_factory = fs_factory;
	_root = _fs_factory->get_fs(NULL, NULL, NULL);
	_fs.add(_root);

	for (a = 0;a < 16;a++) {
		_channel_buf[a] = new byte_t[256];
		_channel_ptr[a] = -1;
		_channel_state[a] = CHANNEL_CLOSE;
		_channel_idx[a] = 0;
	}
}

Device::~Device(void)
{
	int a;
	FileSystem *fs;

	/*
	 *  this removes _root too!
	 */
	while (_fs.size() > 0) {
		fs = _fs.remove();
                if (fs->get_refcount() < 1) {
                        delete fs;
                }
	}

	for (a = 0;a < 16;a++) {
		delete _channel_buf[a];
	}
}

int Device::id(void)
{
	return _id;
}

Directory * Device::readdir(void)
{
	return _fs.top()->get_dir();
}

void Device::update(void)
{
	_fs.top()->update();
}

const char * Device::fstype(void)
{
	return _fs.top()->fstype();
}

const char * Device::path(void)
{
        char *buf;
        const char *p;
	int a, len, maxlen, size;

        maxlen = 1024;
        buf = new char[maxlen];

	len = 1;
        buf[0] = '\0';
	size = _fs.size();
	a = 0;
	while (a < size) {
        	if ((p = _fs[a]->get_path()) != NULL) {
			len += strlen(p) + 1;
	                if (len >= maxlen) {
		                while (maxlen < len) maxlen *= 2;
	                        char *tmp = new char[maxlen];
	                        strcpy(tmp, buf);
	                        delete buf;
	                        buf = tmp;
	                }
	                strcat(buf, p);
	                strcat(buf, "/");
			delete p;
		}
		a += _fs[a]->get_refcount() + 1;
        }

	return buf;
}

File * Device::open(const char *name)
{
	int idx;
	File *f;
	const char *p;
	Directory *dir;
	DirectoryEntry *e;

	e = NULL;
        dir = readdir();

	idx = 0;
	for (Directory::iterator it = dir->begin();it != dir->end();it++) {
		if (util_glob(name, (*it)->name())) {
			e = *it;
			break;
		}
		idx++;
	}
	if (e == NULL) return NULL;

	_gui->set_selected(idx);

	p = path();
	f = _fs.top()->open(p, e);
	delete p;
	return f;
}

File * Device::open_write(const char *name)
{
        const char *p = path();
	File *f = _fs.top()->open_write(p, name);
	delete p;

	return f;
}

char * Device::command(DeviceCommand cmd, char *arg, int arglen)
{
	return _fs.top()->command(cmd, arg, arglen);
}

bool Device::command_supported(DeviceCommand cmd)
{
	return _fs.top()->command_supported(cmd);
}

int Device::open_direct_access(int sec_addr, int channel)
{
	if (channel < 0) {
		int a;

		for (a = 0;a < 16;a++) {
			if (_channel_state[a] == CHANNEL_CLOSE) {
				channel = a;
				break;
			}
		}
	}

	if (channel < 0) return -1;

	if (_channel_state[channel] == CHANNEL_CLOSE) {
		_channel_state[channel] = CHANNEL_DIRECT_ACCESS;
		_channel_ptr[sec_addr] = channel;
		_channel_buf[channel] = new byte_t[256];
		_channel_idx[channel] = 0;
		memset(_channel_buf[channel], 0, 256);
	}

	/*
	*debug << _MODULE_ "DA channel " << channel
 	       << " opened for sec_addr " << sec_addr << endl;
	*/

	return channel;
}

ChannelState Device::get_channel_state(int sec_addr)
{
	int channel;

	channel =  _channel_ptr[sec_addr];
	if (channel < 0) return CHANNEL_CLOSE;
	return _channel_state[channel];
}

int Device::handle_command(char *cmd)
{
	int len;
	int sec_addr;
	int channel;
	int drive;
	int track;
	int sector;
	char *ptr;
	char *name;

	ptr = cmd;
	while (*ptr != ':') {
		if (*ptr == '\0') return 0;
		ptr++;
	}
	len = ptr - cmd;
	name = new char[len + 1];
	strncpy(name, cmd, len);
	name[len] = '\0';
	ptr++;

	if (strcmp(name, "U1") == 0) {
		sscanf(ptr, "%d %d %d %d", &sec_addr, &drive, &track, &sector);
		channel = _channel_ptr[sec_addr];
		//debug->form("U1 - %d %d %d %d - %d\n", sec_addr, drive, track, sector, channel);
		if (channel < 0) {
			error("no channel 1");
			return 0;
		}
		if (_channel_state[channel] != CHANNEL_DIRECT_ACCESS) {
			error("no channel 2");
		}
		read_sector(_channel_buf[channel], track, sector);
	}

	return 0;
}

bool Device::read_sector(byte_t *buf, int track, int sector)
{
	return _fs.top()->read_sector(buf, track, sector);
}

bool Device::write_sector(byte_t *buf, int track, int sector)
{
	return _fs.top()->write_sector(buf, track, sector);
}

/*
 *  return value:
 *    -1: error
 *   >=0: active index
 */
int Device::chdir(const char *newpath)
{
	unsigned int a;
	int err;
	char *p;
	char *ptr, *tmp;
	const char *root;
	Directory *dir;
        FileSystem *new_fs;

	p = util_strdup(newpath);
#ifdef MSDOS
	for (a = 0;a < strlen(p);a++) {
		// p[a] = toupper(p[a]);
		if (p[a] == '\\') p[a] = '/';
	}
#endif /* MSDOS */

#ifdef DEBUG_CHDIR
	*debug << "chdir: " << newpath << endl;
#endif /* DEBUG_CHDIR */

	ptr = p;

	root = NULL;
	dir = _fs[0]->get_dir();
	for (Directory::iterator it = dir->begin();it != dir->end();it++) {
		const char *n = (*it)->name();
#ifdef DEBUG_CHDIR
		*debug << "chdir: matching: " << n << endl;
#endif /* DEBUG_CHDIR */
		if (strncmp(ptr, n, strlen(n)) == 0) {
#ifdef DEBUG_CHDIR
			*debug << "chdir: => " << n << endl;
#endif /* DEBUG_CHDIR */
			root = n;
			break;
		}
	}

	/*
	 *  absolute path: go up to root directory
	 */
#ifdef DEBUG_CHDIR
	*debug << "chdir: current path: " << (char*)path() << endl;
#endif /* DEBUG_CHDIR */
	if (root != NULL) {
#ifdef DEBUG_CHDIR
		*debug << "chdir: toplevel dir: " << root << endl;
#endif /* DEBUG_CHDIR */
        	while (_fs.size() > 1) {
                	/*
                         *  don't remove the root file system
                         */
			new_fs = _fs.remove();
                        if (new_fs->get_refcount() < 1) {
                                delete new_fs;
                        }
			_fs.top()->chdir(NULL, 0);
                }
		ptr += strlen(root);
		while (*ptr == '/') ptr++;
#ifdef DEBUG_CHDIR
		*debug << "chdir: new path is: " << ptr << endl;
#endif /* DEBUG_CHDIR */
		err = _chdir(root);
		if (err < 0) {
			delete p;
			return err;
		}
#ifdef DEBUG_CHDIR
		*debug << "chdir: now in root dir: cd " << ptr << endl;
#endif /* DEBUG_CHDIR */
	}

	if (*ptr == '\0') {
		delete p;
		return 0;
	}

	while (1) {
#ifdef DEBUG_CHDIR
		*debug << "chdir: ptr = '" << ptr;
#endif /* DEBUG_CHDIR */
		tmp = strchr(ptr, '/');
#ifdef DEBUG_CHDIR
		*debug << "', tmp = '" << tmp;
#endif /* DEBUG_CHDIR */
		if (tmp == NULL) break;
		*tmp = '\0';
#ifdef DEBUG_CHDIR
		*debug << "', ptr = '" << ptr;
#endif /* DEBUG_CHDIR */
		err = _chdir(ptr);
#ifdef DEBUG_CHDIR
		*debug << "' -> " << err << endl;
#endif /* DEBUG_CHDIR */
		if (err < 0) {
			delete p;
			return err;
		}
		tmp++;
		while (*tmp == '/') tmp++;
		if (*tmp == '\0') {
			delete p;
			return err;
		}
		ptr = tmp;
	}

#ifdef DEBUG_CHDIR
	*debug << "', done, ptr = '" << ptr << "'";
#endif /* DEBUG_CHDIR */
	err = _chdir(ptr);
#ifdef DEBUG_CHDIR
	*debug << " -> " << err << endl;
#endif /* DEBUG_CHDIR */
	delete p;
	return err;
}

/*
 *  return value:
 *    -1: error
 *   >=0: active index
 */
int Device::_chdir(const char *path)
{
	int size, idx;
	Directory *dir;
        FileSystem *new_fs;
	DirectoryEntry *e;

	if (strlen(path) < 1) return -1;
        if (strcmp(path, ".") == 0) return 0;
        if (strcmp(path, "..") == 0) {
        	size = _fs.size();
                /*
                 *  don't remove the root file system
                 */
        	if (size > 1) {
			int refcount = _fs.top()->get_refcount();

                        idx = _fs.top()->chdir_internal_up();
                        
			new_fs = _fs.remove();
                        if (refcount < 1) {
                                delete new_fs;
                        }

                        if (idx >= 0) return idx;
                        
			return _fs.top()->chdir(NULL, 0);
                }
                return 0;
        }

	e = NULL;
        dir = readdir();

	idx = 0;
	for (Directory::iterator it = dir->begin();it != dir->end();it++) {
		/*
		 *  FIXME: globbing goes here!
		 */
		if (strcmp(path, (*it)->name()) == 0) {
			e = *it;
			break;
		}
		idx++;
	}
	if (e == NULL) return -1;

	new_fs = _fs.top()->chdir_internal(e, idx);

	if (new_fs == NULL) {
		new_fs = _fs_factory->get_fs(this, _fs.top(), e);
		if (new_fs == NULL) return -1;
	}

        _fs.top()->chdir(e, idx);
        _fs.add(new_fs);

        return 0;
}

char Device::channel_char(int sec_addr)
{
	int ch = _channel_ptr[sec_addr];
	return _channel_buf[ch][_channel_idx[ch]];
}

void Device::channel_next(int sec_addr)
{
	int ch = _channel_ptr[sec_addr];
	_channel_idx[ch] = (_channel_idx[ch] + 1) & 0xff;
}

char Device::error_char(void)
{
	return *_error_ptr;
}

void Device::error_next(void)
{
	if (*_error_ptr == '\0') {
		_error_ptr = _error;
	} else {
		_error_ptr++;
	}
}

void Device::error(const char *err)
{
	if (_error != NULL) delete _error;
	if (err != NULL) {
		_error = util_strdup(err);
		_error_ptr = _error;
		// *debug << "Device::error(): " << _error << endl;
	}
}

