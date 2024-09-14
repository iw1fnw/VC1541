/*
 *  $Id: assert.h,v 1.2 1998/10/26 03:03:23 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __misc_assert_h
#define __misc_assert_h

#include <stdio.h>
#include <signal.h>

#undef assert

#define INTERNAL_ERROR(a) do { \
	fprintf(stderr, "*** internal error ***\n"); \
	fprintf(stderr, "file: %s, line: %d\n", __FILE__, __LINE__); \
	fprintf(stderr, "=> %s\n\n", a); \
	exit(1); \
} while (0);

#define __ASSERT(a,b,c) do { \
	fprintf(stderr, "assertion failed (file: %s, line: %d)\n", b, c); \
	fprintf(stderr, "%s\n\n", a); \
	raise(SIGTRAP); \
} while (0);
	

#if defined(NDEBUG)
#define ASSERT(test) ((void)0)
#else
#define ASSERT(test) if(!(test))__ASSERT(#test,__FILE__,__LINE__)
#endif

#endif /* __misc_assert_h */
