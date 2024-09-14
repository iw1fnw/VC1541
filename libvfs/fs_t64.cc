/*
 *  $Id: fs_t64.cc,v 1.5 1998/10/26 03:02:19 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include "misc/debug.h"
#include "libvfs/fs_t64.h"

FileSystemT64::FileSystemT64(File *file) : FileSystem(file, "Tape Image (T64)")
{
	int a, b;
	char *type;
	t64_tape_t tr;
	t64_file_t fr;
	word_t blocks;
	char name[25];

	if (file->read(&tr, sizeof(t64_tape_t)) != sizeof(t64_tape_t)) return;

	diremu_init();

	for (a = 0;a < 24;a++) name[a] = tr.user_desc[a];
	name[24] = '\0';
	_dir->set_title(name, 0);
	diremu_set_title(name, 0);

	for (a = 0;a < tr.entries;a++) {
		if (file->read(&fr, sizeof(t64_file_t)) != sizeof(t64_file_t)) {
			return;
		}
		if (fr.entry_type != 0) {
			switch (fr.entry_type) {
			case 1:  type = " PRG "; break;
			case 3:  type = " USR "; break; /* snapshot V0.9 */
			default: type = " ??? "; break;
			}
			blocks = (((fr.end - fr.start) & 0xffff) + 253) / 254;
			strncpy(name, fr.name, 16);
			name[16] = '\0';
                        b = 15;
                        do {
                        	if (name[b] == ' ') {
                        		name[b] = '\0';
                                } else {
                                	break;
                                }
                                b--;
                        } while (b > 0);

			_dir->add_sorted(name, blocks, type);
			diremu_add_entry(name, blocks, type);
		}
	}
}

FileSystemT64::~FileSystemT64(void)
{
}

File * FileSystemT64::open(const char * /*path*/, DirectoryEntry *e)
{
	File *f;
	File *_file;
	char tmp_name[L_tmpnam];
	int found;
	byte_t tmp;
	char fname[17];
	t64_tape_t tr;
	t64_file_t fr;
	word_t a, b, c, blocks;

	_file = file();
	_file->rewind();

	if (_file->read(&tr, sizeof(t64_tape_t)) != sizeof(t64_tape_t)) return NULL;

	found = 0;
	for (a = 0;a < tr.entries;a++) {
		if (_file->read(&fr, sizeof(t64_file_t)) != sizeof(t64_file_t)) return NULL;
		if (fr.entry_type != 0) {
			for (b = 0;b < 16;b++) fname[b] = fr.name[b];
			fname[b] = '\0';
		}
                b = 15;
                do {
                       	if (fname[b] == ' ') {
                       		fname[b] = '\0';
                        } else {
                             	break;
                        }
                        b--;
                } while (b > 0);
		if (strcmp(e->name(), fname) == 0) {
			found = 1;
			break;
		}
	}

	if (found) {
		blocks = (fr.end - fr.start + 253) / 254;

		tmpnam(tmp_name);
	        f = new FileDOS_Tmp(tmp_name, NULL, File_C_RDWR);
		if (!f->ok()) {
	        	delete f;
	                return NULL;
	        }

		tmp = fr.start;      f->write(&tmp, 1);
		tmp = fr.start >> 8; f->write(&tmp, 1);

		if (!_file->seek(fr.offset)) return NULL;

		b = c = 0;
		for (a = fr.start;a < fr.end - 1;a++) {
			_file->read(&tmp, 1);
			f->write(&tmp, 1);
			if ((b++ % 254) == 0) {
				/*
				printf("[%03d/ %03d] '%s'\n", c++, blocks, fname);
				*/
			}
		}
		/*
		printf("[%03d/ %03d] '%s'\n", c, blocks, fname);
		*/

		_file->read(&tmp, 1);
		f->write(&tmp, 1);
		f->rewind();
		return f;
	}

	return NULL;
}

bool FileSystemT64::check_type_t64(File *file)
{
	int a;
	char buf[CHECK_LEN];

	if (!file->is_regular()) return false;

	a = file->read(buf, CHECK_LEN);
	file->rewind();

	if (a != CHECK_LEN) return 0;

	if (strncmp(buf, "C64 tape file", 13) == 0) return true;
	if (strncmp(buf, "C64 tape image file", 19) == 0) return true;
	if (strncmp(buf, "C64S tape file", 14) == 0) return true;
	if (strncmp(buf, "C64S tape image file", 20) == 0) return true;

	return false;
}

FileSystem * FileSystemT64::check(File *file)
{
	if (check_type_t64(file)) {
#ifdef DEBUG_CHECK
		*debug << "FileSystemT64::check(): ok... " << file->path() << "\n";
#endif /* DEBUG_CHECK */
		return new FileSystemT64(file);
	}
#ifdef DEBUG_CHECK
	*debug << "FileSystemT64::check(): failed!\n";
#endif /* DEBUG_CHECK */
	return NULL;
}

