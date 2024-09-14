/*
 *  $Id: vector.cc,v 1.3 1998/10/26 03:03:40 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#include "misc/assert.h"
#include "misc/vector.h"

template<class T> Vector<T>::Vector(void)
{
	// fprintf(stderr, "+ template<class T> Vector<T>::Vector()\n");
	_array_size = 0;
	_size = 0;
	_array = NULL;
	_increment = 10;
}

template<class T> Vector<T>::~Vector(void)
{
	// fprintf(stderr, "- template<class T> Vector<T>::~Vector()\n");
	free(_array);
}

template<class T> void Vector<T>::add(T &item)
{
	if (_size == _array_size) {
		_array_size += _increment;
		_array = (T *)realloc(_array, _array_size * sizeof(T *));
		ASSERT(_array != NULL);
	}
	_array[_size++] = item;
}

template<class T> T & Vector<T>::remove(void)
{
	ASSERT(_size > 0);
        _size--;
        return _array[_size];
}

template<class T> int Vector<T>::size(void)
{
	return _size;
}

template<class T> T & Vector<T>::operator[](int idx)
{
	ASSERT(idx >= 0);
        ASSERT(idx < _size);
        return _array[idx];
}

template<class T> T & Vector<T>::top(void)
{
	ASSERT(_size > 0);
	return _array[_size - 1];
}

/*
 *  template instantiation
 */
#include "libvfs/fs.h"
template class Vector<FileSystem *>;

#include "libvfs/file.h"
template class Vector<FileSystem * (*)(File *)>;

