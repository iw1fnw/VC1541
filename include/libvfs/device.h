/*
 *  $Id: device.h,v 1.4 1998/10/26 03:03:03 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_device_h
#define __libvfs_device_h

class GUI;

#include "misc/vector.h"
#include "libvfs/fs.h"
#include "libvfs/dir.h"
#include "libvfs/file.h"

class FileSystem;
class FSFactory;

typedef enum {
	CHANNEL_CLOSE,
	CHANNEL_OPEN,
	CHANNEL_DIRECT_ACCESS
} ChannelState;

class Device
{
private:
	GUI		     *_gui;

	int                   _id;
	char                 *_error;
	char                 *_error_ptr;
	FileSystem           *_root;
	FSFactory            *_fs_factory;
	Vector<FileSystem *>  _fs;

	byte_t		     *_channel_buf[16];
	int                   _channel_ptr[16];
	int		      _channel_idx[16];
	ChannelState	      _channel_state[16];

        int _chdir(const char *path);
public:
	Device(GUI *_gui, FSFactory *fs_factory, int id);
	~Device(void);
	int get_id(void) { return _id; }
        Directory * readdir(void);
	File *open(const char *name);
	File *open_write(const char *name);
	char * command(DeviceCommand cmd, char *arg, int arglen);
	bool command_supported(DeviceCommand cmd);
	ChannelState get_channel_state(int sec_addr);
	int  open_direct_access(int sec_addr, int channel);
	int  handle_command(char *cmd);
	bool read_sector(byte_t *buf, int track, int sector);
	bool write_sector(byte_t *buf, int track, int sector);
	void update(void);

	int id(void);
        const char * path(void);
        int chdir(const char *path);
	const char * fstype(void);
	char error_char(void);
	void error_next(void);
	void error(const char *err);
	char channel_char(int sec_addr);
	void channel_next(int sec_addr);
};

typedef int device_t;

#endif /* __libvfs_device_h */
