/*
 *  $Id: fs_zipc.cc,v 1.3 1998/10/26 03:02:23 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <iostream>
#include <iomanip>

#include <ctype.h>

#include "misc/debug.h"
#include "libvfs/fs_zipc.h"

/*
static void dump(byte_t *ptr)
{
	int _a, _b;

	for (_a = 0;_a < 16;_a++) {
		for (_b = 0;_b < 16;_b++) {
			cerr.form("%02x ", ptr[16 * _a + _b] & 0xff);
		}
		cerr << " -  ";
		for (_b = 0;_b < 16;_b++) {
			if (isprint(ptr[16 * _a + _b])) {
				cerr.form("%c", ptr[16 * _a + _b]);
			} else {
				cerr << ".";
			}
		}
		cerr << endl;
	}
	cerr << endl;
}
*/

static void zipcode_copy_sector(byte_t *sectorbuf, byte_t **bufptr)
{
	memcpy(sectorbuf, (*bufptr), 256);
	(*bufptr) += 256;
}

static void zipcode_fill_sector(byte_t *sectorbuf, byte_t **bufptr)
{
	int fill;

	fill = *(*bufptr);
	memset(sectorbuf, fill, 256);
	(*bufptr)++;
}

static void zipcode_rle_sector(byte_t *sectorbuf, byte_t **bufptr)
{
	int a, b;
	int count;
	int rep, length;

	length = (*bufptr)[0];
	rep    = (*bufptr)[1];
	(*bufptr) += 2;

	count = 0;
	for (a = 0;a < length;a++) {
		b = *(*bufptr);
		if (b != rep) {
			/*
			 *  normal data
			 */
			*sectorbuf++ = b;
			(*bufptr)++;
		} else {
			/*
			 *  rle data
			 */
			a += 2;
			count = (*bufptr)[1];
			b =     (*bufptr)[2];
			(*bufptr) += 3;
			while (count > 0) {
				count--;
				*sectorbuf++ = b;
			}
		}
	}
}

static void zipcode_decompress_track(byte_t *trackbuf, byte_t **bufptr)
{
	int a;
	int mode;
	int track;
	int sector;
	static int stab[] = {
		21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
		21, 21, 21, 21, 21, 21, 21,
		19, 19, 19, 19, 19, 19, 19,
		18, 18, 18, 18, 18, 18,
		17, 17, 17, 17, 17
	};

	track = (*bufptr)[0];
	sector = (*bufptr)[1];
	mode = (track >> 6) & 0x03;
	track &= 0x3f;
	(*bufptr) += 2;

	for (a = 0;a < stab[track - 1];a++) {
		if (a > 0) {
			track = (*bufptr)[0];
			sector = (*bufptr)[1];
			mode = (track >> 6) & 0x03;
			track &= 0x3f;
			(*bufptr) += 2;
		}
		switch (mode) {
		case 0:
			zipcode_copy_sector(trackbuf + 256 * sector, bufptr);
			break;
		case 1:
			zipcode_fill_sector(trackbuf + 256 * sector, bufptr);
			break;
		case 2:
			zipcode_rle_sector(trackbuf + 256 * sector, bufptr);
			break;
		default:
			cerr << "ZipCode: wrong mode\n";
			exit(1);
		}
	}
}

static int zipcode_free_blocks(unsigned char *buf)
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

FileSystemZIPC::FileSystemZIPC(File *file) : FileSystem(file, "ZipCode (DiskPacked)")
{
	File *f;
	char ch;
	char *n;
	byte_t *ptr;
	byte_t *trackbuf;
	byte_t *buf, *bufptr;
	static const int read_len = (21 + 19) * 258 + 2 + 1000;

	char name[23];
	char type[6];
	word_t a, c, blocks, free;


	n = 0;
	ch = file->name()[0];
	if (ch != '3') {
		n = new char[strlen(file->name()) + 1];
		strcpy(n, file->name());
		n[0] = '3';
		f = new FileDOS(file->dir(), n);
		if (!f->ok()) {
			delete n;
			delete f;
			return;
		}
	} else {
		f = file;
	}

	trackbuf = new byte_t[21 * 256]; /* space for the largest track (21 sectors) */
	buf = new byte_t[read_len];      /* for track 18 we need at least this
						number of bytes from the file (3!xxxxx) */

	if (f->read((char *)buf, read_len) != read_len) return;
	bufptr = buf + 2; /* skip load address fixme: should check for 0x0400 ? */
	zipcode_decompress_track(trackbuf, &bufptr); /* track 17 */
	zipcode_decompress_track(trackbuf, &bufptr); /* track 18 */

	ptr = trackbuf;

	for (a = 0;a < 16;a++) {
		name[a] = ptr[144 + a];
	}
	name[a++] = ' ';
	name[a++] = ptr[162];			               /* disk id */
	name[a++] = ptr[163];			    	       /* disk id */
	name[a++] = ptr[164] & 0x7f;			 /* shifted space */
	name[a++] = ptr[165];		   	   /* dos version/ format */
	name[a++] = ptr[166];		   	   /* dos version/ format */
	name[a]   = '\0';
	free = zipcode_free_blocks(ptr);

	_dir->set_title(name, free);

	do {
		if (ptr[0] != 18) break;
		ptr = trackbuf + 256 * (int)ptr[1];

		for (a = 0;a < 256;a += 32) {
			if ((ptr[a + 2] & 0x8f) == 0) continue;
			blocks = ptr[a + 30] + 256 * ptr[a + 31];

			for (c = 0;c < 16;c++) {
				if (ptr[a + 5 + c] == 0xa0) {
					break;
				}
				name[c] = ptr[a + 5 + c];
			}
			name[c] = '\0';

			/*
			 *  deleted file ?
			 */
			if (ptr[a + 2] & 0x80) {
				strcpy(type, " ");
			} else {
				strcpy(type, "*");
			}
			switch (ptr[a + 2] & 0x0f) {
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
			if (ptr[a + 2] & 0x40) {
				strcat(type, "<");
			} else {
				strcat(type, " ");
			}
			_dir->add(name, blocks, type);
		}
	} while (ptr[0] != 0);

	if (n != 0) {
		delete n;
		delete f;
	}

	delete trackbuf;
	delete buf;
}

FileSystemZIPC::~FileSystemZIPC(void)
{
}

File * FileSystemZIPC::open(const char * /*path*/, DirectoryEntry * /*e*/)
{
	return 0;
}

bool FileSystemZIPC::check_type_zipc(File *file)
{
	int a;
	char c, *n;
	unsigned char buf[CHECK_LEN];
	File *f;

	if (!file->is_regular()) return false;
	if (file->name()[1] != '!') return false;
	n = 0;
	c = file->name()[0];
	if (c != '1') {
		n = new char[strlen(file->name()) + 1];
		strcpy(n, file->name());
		if ((c >= '1') && (c <= '5')) {
			n[0] = '1';
		} else {
			delete n;
			return false;
		}
		f = new FileDOS(file->dir(), n);
		if (!f->ok()) {
			delete n;
			delete f;
			return false;
		}
	} else {
		f = file;
	}

	a = f->read(buf, CHECK_LEN);
	f->rewind();

	if (n != 0) {
		delete n;
		delete f;
	}

	if (a != CHECK_LEN) return false;

	/*
	 *  check load address (0x03fe)
	 */
	if ((buf[0] == 0xfe) && (buf[1] == 0x03)) return true;

	return false;
}

FileSystem * FileSystemZIPC::check(File *file)
{
	if (check_type_zipc(file)) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemZIPC::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemZIPC(file);
	}
#ifdef DEBUG_CHECK
	*debug << "FileSystemZIPC::check(): failed!\n";
#endif /* DEBUG_CHECK */
	return NULL;
}

