/*
 *  $Id: vector.h,v 1.2 1997/12/16 01:52:10 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __misc_vector_h
#define __misc_vector_h

#include <assert.h>
#include <malloc.h>

template<class T> class Vector
{
private:
	int _size;
	int _array_size;
	int _increment;
	T *_array;

public:
	Vector(void);
	~Vector(void);
	void add(T &item);
        int size(void);
        T & remove(void);
        T & operator[](int);
        T & top(void);
};


#endif /* __misc_vector_h */

