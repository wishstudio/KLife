/*
 *   Copyright (C) 2011 by Xiangyan Sun <wishstudio@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 3, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <cstdlib>

const size_t CHUNK_SIZE = 8192 - 16;

struct MemoryChunk
{
	MemoryChunk *next;
};

class MemoryManager
{
public:
	MemoryManager();
	~MemoryManager();

	template <typename T>
	T *newObject()
	{
		size_t size = sizeof(T);
		if (heads[size])
		{
			T *ret = reinterpret_cast<T *>(heads[size]);
			heads[size] = heads[size]->next;
			return ret;
		}
		T *p = reinterpret_cast<T *>(newChunk());
		reinterpret_cast<MemoryChunk *>(p)->next = NULL;
		char *end = reinterpret_cast<char *>(p) + CHUNK_SIZE;
		for (;;)
		{
			T *q = p + 1;
			if (reinterpret_cast<char *>(q) + sizeof(T) > end)
			{
				heads[size] = reinterpret_cast<MemoryChunk *>(p)->next;
				return p;
			}
			reinterpret_cast<MemoryChunk *>(q)->next = reinterpret_cast<MemoryChunk *>(p);
			p = q;
		}
	}

	template <typename T>
	void deleteObject(T *object)
	{
		size_t size = sizeof(T);
		reinterpret_cast<MemoryChunk *>(object)->next = heads[size];
		heads[size] = reinterpret_cast<MemoryChunk *>(object);
	}

private:
	MemoryChunk *newChunk();
	void deleteChunk(MemoryChunk *chunk);

	MemoryChunk *head, *heads[CHUNK_SIZE];
};

#endif
