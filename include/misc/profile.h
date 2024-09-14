/*
 *  $Id: profile.h,v 1.3 1998/10/26 03:03:25 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __misc_profile_h
#define __misc_profile_h

#include <stdio.h>

typedef union
{
	long long ll;
	struct {
		long	lo;
		long	hi;
	} lh;
} profile_value;

inline void tsc_read(profile_value *val)
{
	__asm__ __volatile__ (
		".byte 0x0f, 0x31\n\t"
		/*"rdtsc\n\t"*/           /* .byte 0x0f, 0x31 */
		: "=a" (val->lh.lo),  /* EAX, 0 */
		  "=d" (val->lh.hi)   /* EDX, 1 */
		: /* no inputs */
		: "eax", "edx"
          );
}

inline void tsc_diff(FILE *stream, char *fmt, profile_value *val1, profile_value *val2)
{
	long long l;

	l = val2->ll - val1->ll;
	fprintf(stream, fmt, l);
}

#endif /* __misc_profile_h */
