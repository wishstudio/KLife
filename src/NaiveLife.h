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

#ifndef NAIVELIFE_H
#define NAIVELIFE_H

#include "AbstractAlgorithm.h"
#include "BigInteger.h"

class QMutex;
class CanvasPainter;
class NaiveLife: public AbstractAlgorithm
{
	Q_OBJECT

public:
	NaiveLife();

	virtual QString name() { return "NaiveLife"; }
	virtual int grid(int x, int y);
	virtual void setGrid(const BigInteger &x, const BigInteger &y, int state);
	virtual void clearGrid();
	virtual bool isVerticalInfinity() { return false; }
	virtual bool isHorizontalInfinity() { return false; }
	virtual void getRect(int *x, int *y, int *w, int *h);
	virtual void setRect(const BigInteger &x, const BigInteger &y, const BigInteger &w, const BigInteger &h);
	virtual void paint(CanvasPainter *canvasPainter, const BigInteger &x, const BigInteger &y, int w, int h);
	virtual void runStep();

private:
	virtual void run();

	QMutex *m_lock;

	int *m_grid;

	size_t m_gridSize;
	BigInteger m_x, m_y, m_w, m_h;
};

#endif
