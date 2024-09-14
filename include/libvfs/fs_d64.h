/*
 *  $Id: fs_d64.h,v 1.3 1998/10/26 03:03:08 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __libvfs_fs_d64_h
#define __libvfs_fs_d64_h

#include "libvfs/fs.h"
#include "libvfs/file.h"

typedef struct
{
	unsigned char track;
	unsigned char sector;
} track_sector_t;

#define INVALID_TRACK_SECTOR ((track_sector_t){0, 0})
#define IS_INVALID_TRACK_SECTOR(ts) ((ts.track == 0) && (ts.sector == 0))

class FileSystemD64 : public FileSystem
{
public:
	static const int MAX_NR_OF_TRACKS = 45;

protected:
 	/*
 	 *  _free[0] and _BAM[0] is unused 'cause track numbering
	 *  starts with 1! we reserve a bit more space than for
	 *  35 tracks as real world images can hold up to 43 tracks,
	 *  although those are not yet supported
	 */
	unsigned long _free[MAX_NR_OF_TRACKS];
	unsigned long _BAM[MAX_NR_OF_TRACKS];

	// virtual void		free_block(void);
	virtual track_sector_t	allocate_block(void);
	virtual bool		read_sector(byte_t *buf,
					    int track,
			       		    int sector);
	virtual bool		write_sector(byte_t *buf,
					     int track,
			       		     int sector);
	virtual int		find_file(const char *filename,
			      		  char *filename_ret = NULL,
			      		  byte_t *track = NULL,
			      		  byte_t *sector = NULL,
			      		  word_t *blocks = NULL);
	virtual void		read_bam(void);
	virtual void		write_bam(void);
	virtual track_sector_t	find_free_block(void);
	virtual int		find_free_track(void);
	virtual void		write_file_slot(byte_t *buf,
						int idx,
						const char *name,
						track_sector_t ts);
	virtual int		allocate_file_slot(const char *name,
						   track_sector_t ts,
						   track_sector_t *ts_dir);
	virtual void		close_dir_slot(track_sector_t ts,
					       int idx,
					       int blocks);
	virtual track_sector_t	find_free_dir_block(track_sector_t ts);

	static bool check_type_d64(File *file);

	void init(File *file);

public:
	FileSystemD64(File *file);
	virtual ~FileSystemD64(void) {}

	File * open(const char *path, DirectoryEntry *e);
	virtual File * open_write(const char *path, const char *name);
	virtual char * command(DeviceCommand cmd, char *arg, int arglen);
	virtual bool command_supported(DeviceCommand cmd);

	virtual void update(void);

	static FileSystem * check(File *file);

	friend class FileD64;
};

#endif /* __libvfs_fs_d64_h */
