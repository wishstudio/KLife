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

#include <cstdlib>
#include <cstring>

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
		MemoryChunk *next = head->next;
		delete head;
		head = next;
	}
}

MemoryChunk *MemoryManager::newChunk()
{
	if (head)
	{
		MemoryChunk *ret = head;
		head = head->next;
		return ret;
	}
	return static_cast<MemoryChunk *>(malloc(CHUNK_SIZE));
}

void MemoryManager::deleteChunk(MemoryChunk *chunk)
{
	chunk->next = head;
	head = chunk;
}
