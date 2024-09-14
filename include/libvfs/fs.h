/*
 *  $Id: fs.h,v 1.5 1998/10/26 03:03:06 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_h
#define __libvfs_fs_h

#include "libvfs/dir.h"
#include "libvfs/file.h"

/* #define DEBUG_CHECK */

/*
 *  valid commands not listed here:
 *
 *    UI+	switch to C64 mode
 *    UI-	switch to VC-20 mode
 */
typedef enum {
	CMD_N,		/* NEW */
	CMD_I,		/* INITIALIZE */
	CMD_V,		/* VALIDATE */
	CMD_C,		/* COPY */
	CMD_R,		/* RENAME */
	CMD_S,		/* SCRATCH */
	CMD_B_R,	/* BLOCK-READ */
	CMD_B_W,	/* BLOCK-WRITE */
	CMD_B_E,	/* BLOCK-EXECUTE */
	CMD_B_P,	/* BUFFER-POINTER */
	CMD_B_A,	/* BLOCK-ALLOCATE */
	CMD_B_F,	/* BLOCK-FREE */
	CMD_M_W,	/* MEMORY-WRITE */
	CMD_M_R,	/* MEMORY-READ */
	CMD_M_E,	/* MEMORY-EXECUTE */
	CMD_UA,		/* replacement for BLOCK-READ (alias is U1) */
	CMD_UB,		/* replacement for BLOCK-WRITE (alias is U2) */
	CMD_UC,		/* jump to $0500 */
	CMD_UD,		/* jump to $0503 */
	CMD_UE,		/* jump to $0506 */
	CMD_UF,		/* jump to $0509 */
	CMD_UG,		/* jump to $050c */
	CMD_UH,		/* jump to $050F */
	CMD_UI,		/* jump to $FFFA */
	CMD_UJ		/* power-up reset */
} DeviceCommand;

class FileSystem
{
private:
        int _refcount;
	char *_fstype;
	byte_t *_dir_sector[19];
	int _dir_len;

protected:
	File *_file;
	Directory *_dir;
	int _cur_idx;
	DirectoryEntry *_cur_dirent;

public:
	FileSystem(File *file, const char *fstype);
	virtual ~FileSystem(void);

	virtual void diremu_init(void);
	virtual void diremu_set_title(const char *title, dword_t free);
	virtual void diremu_add_entry(const char *name, dword_t blocks,
				const char *type);

	virtual int chdir(DirectoryEntry *e, int idx);
	virtual FileSystem * chdir_internal(DirectoryEntry *e, int idx);
	virtual int chdir_internal_up(void);
	virtual File * open(const char *path, DirectoryEntry *e);
	virtual File * open_write(const char *path, const char *name);
	virtual char * command(DeviceCommand cmd, char *arg, int arglen);
	virtual bool command_supported(DeviceCommand cmd);

	virtual bool		read_sector(byte_t *buf,
					    int track,
			       		    int sector);
	virtual bool		write_sector(byte_t *buf,
					     int track,
			       		     int sector);
	virtual void update(void) {}

	virtual Directory * get_dir(void);
	virtual const char * get_path(void);
	virtual File * file(void);
	virtual const char * fstype(void);

        virtual int  get_refcount(void) { return _refcount;   }
        virtual void inc_refcount(void) {        _refcount++; }
        virtual void dec_refcount(void) {        _refcount--; }
};

#endif /* __libvfs_fs_h */

