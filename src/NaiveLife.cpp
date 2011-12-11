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

#include <QMutex>

#include "AlgorithmManager.h"
#include "CanvasPainter.h"
#include "NaiveLife.h"

REGISTER_ALGORITHM(NaiveLife)

NaiveLife::NaiveLife()
	: m_lock(new QMutex()), m_grid(NULL), m_x(0), m_y(0), m_w(0), m_h(0)
{
}

#define GRID(x, y) ((size_t) (x) * (size_t) m_h + (size_t) (y))
int NaiveLife::grid(const BigInteger &x, const BigInteger &y)
{
	m_lock->lock();
	int my_x = x - m_x, my_y = y - m_y;
	int ret = m_grid[GRID(my_x, my_y)];
	m_lock->unlock();
	return ret;
}

void NaiveLife::setGrid(const BigInteger &x, const BigInteger &y, int state)
{
	m_lock->lock();
	int my_x = x - m_x, my_y = y - m_y;
	m_grid[GRID(my_x, my_y)] = state;
	m_lock->unlock();
	emit gridChanged();
}

void NaiveLife::clearGrid()
{
	memset(m_grid, 0, m_gridSize);
	emit gridChanged();
}

void NaiveLife::getRect(BigInteger *x, BigInteger *y, BigInteger *w, BigInteger *h)
{
	*x = m_x;
	*y = m_y;
	*w = m_w;
	*h = m_h;
}

void NaiveLife::setRect(const BigInteger &x, const BigInteger &y, const BigInteger &w, const BigInteger &h)
{
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	m_gridSize = (size_t) m_w * (size_t) m_h * sizeof(int);
	m_grid = (int *) realloc(m_grid, m_gridSize);
	emit rectChanged();
	clearGrid();
}

void NaiveLife::paint(CanvasPainter *painter, const BigInteger &x, const BigInteger &y, int w, int h)
{
	int my_x = x - m_x, my_y = y - m_y;
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
			painter->drawGrid(i, j, m_grid[GRID(my_x + i, my_y + j)]);
}

void NaiveLife::runStep()
{
	start();
}

void NaiveLife::run()
{
	int *now = (int *) malloc(m_gridSize);
	const int dx[8] = {-1, -1, -1, 0, 1, 1,  1,  0};
	const int dy[8] = {-1,  0,  1, 1, 1, 0, -1, -1};
	for (int i = 0; i < m_w; i++)
		for (int j = 0; j < m_h; j++)
		{
			int cnt = 0;
			for (int d = 0; d < 8; d++)
			{
				int x = i + dx[d], y = j + dy[d];
				if (x >= 0 && x < m_w && y >= 0 && y < m_h)
					cnt += m_grid[GRID(x, y)];
			}
			now[GRID(i, j)] = (cnt == 3 || (m_grid[GRID(i, j)] && cnt == 2));
		}
	int *backup = m_grid;
	m_lock->lock();
	m_grid = now;
	m_lock->unlock();
	delete backup;
	emit gridChanged();
}
