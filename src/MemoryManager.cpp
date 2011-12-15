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
/*
#include <cstdlib>

#include "MemoryManager.h"

MemoryManager::MemoryManager()
	: head(NULL)
{
	memset(heads, 0, sizeof heads);
}

MemoryManager::~MemoryManager()
{
	while (head)
	{
		MemoryPool *next = head->next;
		delete head;
		head = next;
	}
}

MemoryPool *MemoryManager::newPool()
{
	if (head)
	{
		MemoryPool *ret = head;
		head = head->next;
		return ret;
	}
	return (MemoryPool *) malloc(sizeof(MemoryPool));
}

void MemoryManager::deletePool(MemoryPool *pool)
{
	pool->next = head;
	head = pool;
}

template <typename T>
T *MemoryManager::newObject()
{
	size_t size = sizeof(T);
	if (heads[size])
	{
		T *ret = heads[size];
		heads[size] = heads[size]->next;
		return ret;
	}
	MemoryChunk *p = newPool(), *ret = ((char *) p) + POOL_SIZE - POOL_SIZE % size - size;
	if (p == ret)
		return p;
	p->next = NULL;
	for (;;)
	{
		MemoryChunk *q = ((char *) p) + size;
		if (q == ret)
		{
			heads[size] = p;
			return ret;
		}
		q->next = p;
		p = q;
	}
}

template <typename T>
void MemoryManager::deleteObject(T *object)
{
	size_t size = sizeof(T);
	((MemoryChunk *) object)->next = heads[size];
	heads[size] = object;
}
*/
