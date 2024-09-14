/*
 *  $Id: fs_d64.cc,v 1.3 1998/10/26 03:02:12 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include "misc/debug.h"
#include "misc/assert.h"
#include "libvfs/f_d64.h"
#include "libvfs/fs_d64.h"

#define DEBUG_LEVEL 0
#define _MODULE_ "FS [D64]: "

bool
FileSystemD64::read_sector(byte_t *buf,
			       int track,
			       int sector)
{
	long a, offset;
	static int stab[] = {
		21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
		21, 21, 21, 21, 21, 21, 21,
		19, 19, 19, 19, 19, 19, 19,
		18, 18, 18, 18, 18, 18,
		17, 17, 17, 17, 17
	};

	if (track <= 0 || track > 35) return false;
	if (sector < 0 || sector >= stab[track - 1]) return false;
	offset = 0;
	for (a = 0;a < track - 1;a++) offset += stab[a];
	offset = (offset + sector) * 256;
	if (!_file->seek(offset)) return false;
	if (_file->read((char *)buf, 256) != 256) return false;

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "reading [" << track << "/ " << sector << "]"
	       << endl;
#endif

	return true;
}

bool
FileSystemD64::write_sector(byte_t *buf,
			    int track,
			    int sector)
{
	long a, offset;
	static int stab[] = {
		21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
		21, 21, 21, 21, 21, 21, 21,
		19, 19, 19, 19, 19, 19, 19,
		18, 18, 18, 18, 18, 18,
		17, 17, 17, 17, 17
	};

	if (track <= 0 || track > 35) return false;
	if (sector < 0 || sector >= stab[track - 1]) return false;
	offset = 0;
	for (a = 0;a < track - 1;a++) offset += stab[a];
	offset = (offset + sector) * 256;
	if (!_file->seek(offset)) return false;
	if (_file->write((char *)buf, 256) != 256) return false;

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "writing [" << track << "/ " << sector << "]"
	       << endl;
#endif

	return true;
}

static int d64_free_blocks(unsigned char *buf)
{
	int a, b;

	/*
	 *  count free block in BAM but
	 *  ignore the directory track 18
	 */
	b = 0;
	for (a = 4;a < 143;a += 4) {
		if (a != 72) b += buf[a];
	}

	return b;
}

FileSystemD64::FileSystemD64(File *file) : FileSystem(file, "Disk Image (D64)")
{
	init(file);
}

void FileSystemD64::init(File * /* file */)
{
	byte_t buf[256];
	char name[23];
	char type[6];
	word_t a, c, blocks, free;

	for (a = 0;a < MAX_NR_OF_TRACKS;a++) {
		_BAM[a] = ~0;
		_free[a] = 0;
	}

	read_bam();

	if (!read_sector(buf, 18, 0)) return;

	for (a = 0;a < 16;a++) {
		name[a] = buf[144 + a];
	}
	name[a++] = ' ';
	name[a++] = buf[162];			               /* disk id */
	name[a++] = buf[163];			    	       /* disk id */
	name[a++] = buf[164] & 0x7f;			 /* shifted space */
	name[a++] = buf[165];		   	   /* dos version/ format */
	name[a++] = buf[166];		   	   /* dos version/ format */
	name[a]   = '\0';
	free = d64_free_blocks(buf);

	_dir->clear();
	_dir->set_title(name, free);

	do {
		if (buf[0] != 18) break;
		if (!read_sector(buf, buf[0], buf[1])) break;
		for (a = 0;a < 256;a += 32) {
			if ((buf[a + 2] & 0x8f) == 0) continue;
			blocks = buf[a + 30] + 256 * buf[a + 31];

			for (c = 0;c < 16;c++) {
				if (buf[a + 5 + c] == 0xa0) {
					break;
				}
				name[c] = buf[a + 5 + c];
			}
			name[c] = '\0';

			/*
			 *  deleted file ?
			 */
			if (buf[a + 2] & 0x80) {
				strcpy(type, " ");
			} else {
				strcpy(type, "*");
			}
			switch (buf[a + 2] & 0x0f) {
			case 0:  strcat(type, "DEL"); break;
			case 1:  strcat(type, "SEQ"); break;
			case 2:  strcat(type, "PRG"); break;
			case 3:  strcat(type, "USR"); break;
			case 4:  strcat(type, "REL"); break;
			default: strcat(type, "???"); break;
			}
			/*
			 *  protected file ?
			 */
			if (buf[a + 2] & 0x40) {
				strcat(type, "<");
			} else {
				strcat(type, " ");
			}
			_dir->add(name, blocks, type);
		}
	} while (buf[0] != 0);
}

void FileSystemD64::write_bam(void)
{
	int a, track;
	byte_t buf[256];

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "writing BAM..." << endl;
#endif

	if (!read_sector(buf, 18, 0)) {
		*debug << _MODULE_ "error while reading BAM!" << endl;
		return;
	}

	for (track = 1;track <= 35;track++) {
		a = 4 * track;
		buf[a] = _free[track];
		buf[a + 1] = _BAM[track] & 0xff;
		buf[a + 2] = (_BAM[track] >> 8) & 0xff;
		buf[a + 3] = (_BAM[track] >> 16) & 0xff;
	}

	if (!write_sector(buf, 18, 0)) {
		*debug << _MODULE_ "error while writing BAM!" << endl;
	}
}

void FileSystemD64::read_bam(void)
{
	int a, track;
	byte_t buf[256];
	/*
	int b;
	static int stab[] = {
		0,
		21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
		21, 21, 21, 21, 21, 21, 21,
		19, 19, 19, 19, 19, 19, 19,
		18, 18, 18, 18, 18, 18,
		17, 17, 17, 17, 17
	};
	*/

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "reading BAM..." << endl;
#endif

	if (!read_sector(buf, 18, 0)) {
		*debug << _MODULE_ "error while reading BAM!" << endl;
		return;
	}

	for (track = 1;track <= 35;track++) {
		a = 4 * track;
		_free[track] = buf[a];
		_BAM[track] = (buf[a + 3] << 16) | (buf[a + 2] << 8) | buf[a + 1];
		/*
		debug->form("%02x %02x %02x %02x %08x | %2d: %2d : ",
				buf[a], buf[a + 1], buf[a + 2], buf[a + 3], _BAM[track],
				track, _free[track]);
		for (b = 0;b < stab[track];b++) {
			if (_BAM[track] & (1 << b)) {
				*debug << '-';
			} else {
				*debug << '+';
			}
		}
		*debug << endl;
		*/
	}
}

int
FileSystemD64::find_file(const char   *filename,
			 char   *filename_ret,
			 byte_t *track,
 			 byte_t *sector,
 			 word_t *blocks)
{
	int a, b;
	char fname[17];
	byte_t buf[256];

	read_sector(buf, 18, 0);
	do {
		read_sector(buf, buf[0], buf[1]);
		for (a = 0;a < 256;a += 32) {
			/*
			 *  ignore everything else than normal prg file
			 */
			if ((buf[a + 2] & 0xbf) == 0x82) {
				for (b = 0;b < 16 && buf[a + b + 5] != 0xa0;b++) {
					fname[b] = buf[a + b + 5];
				}
				fname[b] = '\0';
				/* if (util_glob_file(filename, fname, 0)) {*/
				if (strcmp(filename, fname) == 0) {
					if (filename_ret) strcpy(filename_ret, fname);
					if (track) *track = buf[a + 3];
					if (sector) *sector = buf[a + 4];
					if (blocks) *blocks = buf[a + 30] + 256 * buf[a + 31];
					return 1;
				}
			}
		}
	} while (buf[0] != 0);
	return 0;
}

int FileSystemD64::find_free_track(void)
{
	int a, track;

	track = 18;
	for (a = 1;a < 18;a++) {
		if (_free[track - a] > 0) return track - a;
		if (_free[track + a] > 0) return track + a;
	}
	return -1;
}

track_sector_t FileSystemD64::find_free_block(void)
{
	int a;
	int track;
	track_sector_t ts = INVALID_TRACK_SECTOR;

	track = find_free_track();
	if (track < 0) {
		*debug << _MODULE_ "no free block found!" << endl;
		return ts;
	}
	for (a = 0;a < 35;a++) {
		if (_BAM[track] & (1 << a)) {
			ts.track = track;
			ts.sector = a;
#if DEBUG_LEVEL > 0
			*debug << _MODULE_ "found free block ["
 			       << track << "/ " << a << "]" << endl;
#endif
			return ts;
		}
	}

	INTERNAL_ERROR("corrupted BAM");

	return ts;
}

track_sector_t
FileSystemD64::allocate_block(void)
{
	track_sector_t ts;

	ts = find_free_block();
	if (IS_INVALID_TRACK_SECTOR(ts)) return ts;

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "allocating [" << (int)ts.track << "/ "
 	       << (int)ts.sector << "]" << endl;
#endif
	ASSERT(_free[ts.track] > 0);
	ASSERT((_BAM[ts.track] & (1 << ts.sector)) != 0);
	_BAM[ts.track] &= ~(1 << ts.sector);
	_free[ts.track]--;

	return ts;
}

track_sector_t FileSystemD64::find_free_dir_block(track_sector_t ts)
{
	int a, sector;

	ASSERT(ts.track == 18);

	sector = (ts.sector + 3) % 19;
	for (a = 0;a < 19;a++) {
#if DEBUG_LEVEL > 0
		*debug << _MODULE_ "checking [" << (int)ts.track << "/ " << sector << "]" << endl;
#endif
		if (_BAM[ts.track] & (1 << sector)) {
#if DEBUG_LEVEL > 0
			*debug << _MODULE_ "found" << endl;
#endif
			ts.sector = sector;
			return ts;
		}
		sector++;
	}

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "not found" << endl;
#endif
	return INVALID_TRACK_SECTOR;
}

void
FileSystemD64::write_file_slot(byte_t *buf, int idx, const char *name, track_sector_t ts)
{
	int a, state;

	buf[idx + 2] = 0x02; /* PRG, not closed! */
	buf[idx + 3] = ts.track;
	buf[idx + 4] = ts.sector;
	state = 0;
	for (a = 0;a < 16;a++) {
		switch (state) {
		case 0:
			if (name[a] != '\0') {
				buf[idx + a + 5] = name[a];
				break;
			}
			state = 1;
			/* fall trough */
		case 1:
			buf[idx + a + 5] = '\xa0';
			break;
		default:
			INTERNAL_ERROR("FileSystemD64::write_file_slot");
			break;
		}
	}
	buf[idx + 21] = 0; /* used for ... */
	buf[idx + 22] = 0; /* ...relative files... */
	buf[idx + 23] = 0; /* ...only */
	buf[idx + 24] = 0; /* unused */
	buf[idx + 25] = 0; /* unused */
	buf[idx + 26] = 0; /* unused */
	buf[idx + 27] = 0; /* unused */
	buf[idx + 28] = 0; /* used for... */
	buf[idx + 29] = 0; /* ...@ save */
	buf[idx + 30] = 0; /* blocks low */
	buf[idx + 31] = 0; /* blocks high */
}

/*
 *  this should be called only with a name that is not already present
 *
 *  returns the index for the allocated slot (in bytes)
 */
int
FileSystemD64::allocate_file_slot(const char *name,
				  track_sector_t ts_file,
				  track_sector_t *ts_dir)
{
	int a;
	byte_t buf[256];
	track_sector_t ts_new;

#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "find_file_slot..." << endl;
#endif
	buf[0] = 18;
	buf[1] = 1;
	do {
#if DEBUG_LEVEL > 0
		*debug << _MODULE_ "[" << (int)buf[0] << "/ " << (int)buf[1] << "]" << endl;
#endif
		ts_dir->track = buf[0];
		ts_dir->sector = buf[1];
		read_sector(buf, ts_dir->track, ts_dir->sector);
		for (a = 0;a < 256;a += 32) {
			if (buf[a + 2] == 0) {
#if DEBUG_LEVEL > 0
				*debug << _MODULE_ "free slot found: " << a << endl;
#endif
				write_file_slot(buf, a, name, ts_file);
				write_sector(buf, ts_dir->track, ts_dir->sector);
				return a;
			}
		}
	} while (buf[0] != 0);
#if DEBUG_LEVEL > 0
	*debug << _MODULE_ "[" << (int)buf[0] << "/ " << (int)buf[1] << "]" << endl;
#endif
	if (_free[18] == 0) return -1;
	ts_new = find_free_dir_block(*ts_dir);
	if (IS_INVALID_TRACK_SECTOR(ts_new)) return -1;

	buf[0] = ts_new.track;
	buf[1] = ts_new.sector;
	write_sector(buf, ts_dir->track, ts_dir->sector);

	read_sector(buf, ts_new.track, ts_new.sector);
	buf[0] = 0;
	buf[1] = 0xff;
	write_file_slot(buf, 0, name, ts_file);
	write_sector(buf, ts_new.track, ts_new.sector);

	*ts_dir = ts_new;

	return 0;
}

void
FileSystemD64::close_dir_slot(track_sector_t ts,
			      int idx,
			      int blocks)
{
	byte_t buf[256];
	read_sector(buf, ts.track, ts.sector);
	buf[idx + 2] |= 0x80; /* close file */
	buf[idx + 30] = blocks & 0xff; /* blocks low */
	buf[idx + 31] = (blocks >> 8) & 0xff; /* blocks high */
	write_sector(buf, ts.track, ts.sector);
	write_bam();
}

File * FileSystemD64::open(const char * /*path*/, DirectoryEntry *e)
{
	File *f;
	char tmp_name[L_tmpnam];
	char fname[17];
	byte_t buf[256];
	word_t a, c, blocks;

	_file->rewind();

	c = 0;
	if (find_file(e->name(), fname, &buf[0], &buf[1], &blocks)) {

		tmpnam(tmp_name);
	        f = new FileDOS_Tmp(tmp_name, NULL, File_C_RDWR);
		if (!f->ok()) {
	        	delete f;
	                return NULL;
	        }

		do {
			c++;
			/*
			printf("[%03d/ %03d: T%02d S%02d] '%s'\n", c,
					blocks, buf[0], buf[1], fname);
					*/
			if (!read_sector(buf, buf[0], buf[1])) {
                        	delete f;
				return 0;
                        }
			if (buf[0] == 0) {
				/*
				 *  last block
				 */
				for (a = 2;a <= buf[1];a++) {
                                	f->write(&buf[a], 1);
				}
                                f->write(&buf[a], 1);
                                f->rewind();
				return f;
			} else {
				for (a = 2;a < 256;a++) {
                                        f->write(&buf[a], 1);
				}
			}
		} while (buf[0] != 0);

	        delete f;
	}

	return NULL;
}

File * FileSystemD64::open_write(const char *path, const char *name)
{
	int idx;
	track_sector_t ts, ts_dir;

	*debug << _MODULE_ "saving '" << name << "'..." << endl;

	ts = allocate_block();
	if (IS_INVALID_TRACK_SECTOR(ts)) return NULL; /* disk full */

	idx = allocate_file_slot(name, ts, &ts_dir);
	if (idx < 0) return NULL; /* directory slot allocation failed */

	return new FileD64(this, ts, ts_dir, idx, path, name, File_WR);
}

bool FileSystemD64::command_supported(DeviceCommand cmd)
{
	if (cmd == CMD_B_R) return true;
	return false;
}

char * FileSystemD64::command(DeviceCommand /* cmd */, char * /* arg */, int /* arglen */)
{
	return NULL;
}

bool FileSystemD64::check_type_d64(File *file)
{
	if (!file->is_regular()) return false;

        /*
         *  standard disk image, 35 tracks
         */
	if (file->size() == 174848) return true;

        /*
         *  standard disk image, 35 tracks with 683 bytes error info
         */
        if (file->size() == 175531) return true;

        /*
         *  extended disk image, 40 tracks
         */
        if (file->size() == 196608) return true;

        /*
         *  extended disk image, 40 tracks with 768 bytes error info
         */
        if (file->size() == 197376) return true;
        
	return false;
}

FileSystem * FileSystemD64::check(File *file)
{
	if (check_type_d64(file)) {
#ifdef DEBUG_CHECK
	*debug << _MODULE_ "check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemD64(file);
	}
#ifdef DEBUG_CHECK
	*debug << _MODULE_ "check(): failed!\n";
#endif /* DEBUG_CHECK */
	return NULL;
}

void FileSystemD64::update(void)
{
	*debug << "FileSystemD64::update()" << endl;
	init(_file);
}

