/*
 *  $Id: fs_lnx.cc,v 1.4 1998/10/26 03:02:18 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include <stdlib.h>

#include "misc/debug.h"
#include "libvfs/fs_lnx.h"

FileSystemLNX::FileSystemLNX(File *file) : FileSystem(file, "LYNX Archive")
{
	int state;
	word_t a, b, c;
	word_t dirlen, direntries;
	char *name, *len, *type, *last;
	char buf[BLOCK_LEN], *dirmem;
	word_t blocks;

	if (file->read(buf, BLOCK_LEN) != BLOCK_LEN) return;

	if ((c = lnx_get_header(buf, &dirlen, &direntries)) == 0) return;

	file->rewind();
	dirmem = new char[dirlen];
	if (file->read(dirmem, dirlen) != dirlen) {
		delete dirmem;
		return;
	}

	diremu_init();
	_dir->set_title(file->name(), 0);
	diremu_set_title(file->name(), 0);

	a = c;
	for (c = 0;c < direntries;c++) {
		if ((name = lnx_getstr(&a, dirmem, dirlen)) == NULL) break;
		if ((len  = lnx_getstr(&a, dirmem, dirlen)) == NULL) break;
		if ((type = lnx_getstr(&a, dirmem, dirlen)) == NULL) break;
		if ((last = lnx_getstr(&a, dirmem, dirlen)) == NULL) break;

		switch (*type) {
		case 'P': type = " PRG "; break;
		case 'S': type = " SEQ "; break;
		case 'R': type = " REL "; break;
		case 'U': type = " USR "; break;
		default:  type = " ??? "; break;
		}

		blocks = atoi(len);
                b = 15;
                state = 1;
		do {
			if ((byte_t)name[b] == 0xa0) {
				name[b] = state ? '\0' : ' ';
                        } else {
                        	state = 0;
                        }
                        b--;
		} while (b > 0);

		_dir->add(name, blocks, type);
		diremu_add_entry(name, blocks, type);
	}

	delete dirmem;
}

FileSystemLNX::~FileSystemLNX(void)
{
}

int FileSystemLNX::lnx_get_header(char *buf,
			word_t *dirlen, word_t *direntries)
{
	char *ptr;
	word_t a, b;

	for (a = 0;a < BLOCK_LEN - 2;a++) {
		if ((buf[a] | buf[a + 1] | buf[a + 2]) == 0) break;
	}
	if ((a += 3) >= BLOCK_LEN) return 0;
	if (buf[a++] != 0x0d) return 0;

	/*
	 *  # of dir blocks
	 */
	while ((a < BLOCK_LEN) && (buf[a] == 0x20)) a++;
	b = a;
	while ((b < BLOCK_LEN) && (buf[b] != 0x20)) b++;
	if (b >= BLOCK_LEN) return 0;
	ptr = &buf[a];
	buf[b] = '\0';
	if ((*dirlen = atoi(ptr)) == 0) return 0;
	*dirlen *= BLOCK_LEN;

	/*
	 *  skip over signature
	 */
	while ((a < BLOCK_LEN) && (buf[a] != 0x0d)) a++;
	a++;
	if (a >= BLOCK_LEN) return 0;

	/*
	 *  # of dir entries
	 */
	b = a;
	while ((b < BLOCK_LEN) && (buf[b] != 0x0d)) b++;
	if (b >= BLOCK_LEN) return 0;
	ptr = &buf[a];
	buf[b] = '\0';
	if ((*direntries = atoi(ptr)) == 0) return 0;

	return b + 1;
}

char * FileSystemLNX::lnx_getstr(word_t *a, char *dirmem, word_t dirlen)
{
	word_t b;
	char *ptr;

	b = *a;
	while ((b < dirlen) && (dirmem[b] != 0x0d)) b++;
	if (b >= dirlen) return NULL;
	ptr = &dirmem[*a];
	dirmem[b] = '\0';
	*a = b + 1;

	return ptr;
}

File * FileSystemLNX::open(const char * /*path*/, DirectoryEntry *e)
{
	File *f;
	File *_file;
	char tmp_name[L_tmpnam], buf[BLOCK_LEN];
	word_t a, c;
	word_t dirlen, direntries;
	char *name, *len, *type, *last;
	char *dirmem;
	long offset;
	int found, blocks, lastlen;

	_file = file();
	_file->rewind();

	if (_file->read(buf, BLOCK_LEN) != BLOCK_LEN) return NULL;

	if ((c = lnx_get_header(buf, &dirlen, &direntries)) == 0) return NULL;

	_file->rewind();
	dirmem = new char[dirlen];
	if (_file->read(dirmem, dirlen) != dirlen) return NULL;
	offset = dirlen;

	a = c;
	found = 0;
        name = len = type = last = NULL; /* make compiler happy */
	for (c = 0;c < direntries;c++) {
		if ((name = lnx_getstr(&a, dirmem, dirlen)) == NULL) break;
		if ((len  = lnx_getstr(&a, dirmem, dirlen)) == NULL) break;
		if ((type = lnx_getstr(&a, dirmem, dirlen)) == NULL) break;
		if ((last = lnx_getstr(&a, dirmem, dirlen)) == NULL) break;

		if (strcmp(e->name(), name) == 0) {
			found = 1;
			break;
		}

		offset += BLOCK_LEN * atoi(len);
	}

	if (found) {
        	blocks = atoi(len);
                lastlen = atoi(last);
                if (!_file->seek(offset)) return NULL;

	        tmpnam(tmp_name);
		f = new FileDOS_Tmp(tmp_name, NULL, File_RDWR);
	        if (!f->ok()) {
	        	delete f;
	                return NULL;
	        }

                for (a = blocks;a > 1;a--) {
			if (_file->read(buf, BLOCK_LEN) != BLOCK_LEN) {
                        	delete f;
				return NULL;
                        }
                        f->write(buf, BLOCK_LEN);
                }
                if (_file->read(buf, lastlen) != lastlen) {
                	delete f;
                	return NULL;
                }
                f->write(buf, lastlen);
                f->rewind();
		return f;
	}

	return NULL;
}

bool FileSystemLNX::check_type_lnx(File *file)
{
	int a, b;
	char buf[CHECK_LEN], *ptr;

	if (!file->is_regular()) return false;

	a = file->read(buf, CHECK_LEN);
	file->rewind();

	if (a != CHECK_LEN) return false;

	for (a = 0;a < CHECK_LEN - 2;a++) {
		if ((buf[a] | buf[a + 1] | buf[a + 2]) == 0) break;
	}
	if ((a += 3) >= CHECK_LEN) return false;
	if (buf[a++] != 0x0d) return false;

	while ((a < CHECK_LEN) && buf[a] == 0x20) a++;
	while ((a < CHECK_LEN) && buf[a] != 0x20) a++;
	while ((a < CHECK_LEN) && buf[a] == 0x20) a++;
	b = a;
	while ((b < CHECK_LEN) && buf[b] != 0x0d) b++;
	if (b >= CHECK_LEN) return false;

	ptr = &buf[a];
	buf[b] = '\0';

	if (strstr(ptr, "LYNX") == 0) return false;

	return true;
}

FileSystem * FileSystemLNX::check(File *file)
{
	if (check_type_lnx(file)) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemLNX::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemLNX(file);
	}
#ifdef DEBUG_CHECK
	*debug << "FileSystemLNX::check(): failed!\n";
#endif /* DEBUG_CHECK */
	return NULL;
}

#if 0

#include <stdio.h>

#include "fileio.h"

extern int timeout; 			           /* this is set by Wait */

static int lnx_send_dir(device_t *d)
{
}

static int lnx_send_file(device_t *d, char *filename)
{
	FILE *f;
	word_t a, b, c, found;
	word_t dirlen, direntries, blocks;
	byte_t *name, *len, *type, *last;
	byte_t buf[BLOCK_LEN], *ptr, *dirmem, tmp;

	if ((f = fopen(d->path, "rb")) == NULL) return 0;

	if (fread(buf, 1, BLOCK_LEN, f) != BLOCK_LEN) {
		fclose(f);
		return 0;
	}

	if ((a = lnx_get_header(buf, &dirlen, &direntries)) == 0) return 0;

	rewind(f);
	if ((dirmem = (byte_t *)malloc(dirlen)) == NULL) {
		return 0;
	}
	if (fread(dirmem, 1, dirlen, f) != dirlen) {
		fclose(f);
		return 0;
	}

	found = 0;
	blocks = 0;
	for (b = 0;b < direntries;b++) {
		if ((name = lnx_getstr(&a, dirmem, dirlen)) == NULL) return 0;
		if ((len  = lnx_getstr(&a, dirmem, dirlen)) == NULL) return 0;
		if ((type = lnx_getstr(&a, dirmem, dirlen)) == NULL) return 0;
		if ((last = lnx_getstr(&a, dirmem, dirlen)) == NULL) return 0;
		if (util_glob_file(filename, name, 0)) {
			found = 1;
			break;
		}
		blocks += atoi(len);
	}
	if (found) {
		if (fseek(f, blocks * BLOCK_LEN + dirlen, SEEK_SET) != 0) {
			fclose(f);
			return 0;
		}
		blocks = atoi(len);
		b = c = 0;
		for (a = 0;a < (blocks - 1) * BLOCK_LEN;a++) {
			PutIECByte(fgetc(f), 0);
			if (timeout) return 0;
			if ((b++ % 254) == 0) {
				FileInfo(d->dev, "[%03d/ %03d] '%s'",
						c++, blocks, name);
			}
		}
		for (a = 0;a < atoi(last) - 2;a++) {
			PutIECByte(fgetc(f), 0);
			if (timeout) return 0;
			if ((b++ % 254) == 0) {
				FileInfo(d->dev, "[%03d/ %03d] '%s'",
						c++, blocks, name);
			}
		}
		FileInfo(d->dev, "[%03d/ %03d] '%s'", c, blocks, name);

		tmp = fgetc(f);
		fclose(f);
		PutIECByte(tmp, 1);
		if (timeout) return 0;
	}

	return 1;
}

#endif

