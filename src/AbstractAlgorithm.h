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

#ifndef ABSTRACTALGORITHM_H
#define ABSTRACTALGORITHM_H

#include <QThread>

class BigInteger;
class CanvasPainter;
class AbstractAlgorithm: public QThread
{
	Q_OBJECT

public:
	virtual QString name() = 0;
	virtual int grid(const BigInteger &x, const BigInteger &y) = 0;
	virtual void setGrid(const BigInteger &x, const BigInteger &y, int state) = 0;
	virtual void clearGrid() = 0;
	virtual bool isVerticalInfinity() = 0;
	virtual bool isHorizontalInfinity() = 0;
	virtual bool isInfinity(Qt::Orientation orientation) { return orientation == Qt::Vertical? isVerticalInfinity(): isHorizontalInfinity(); }
	virtual void getRect(BigInteger *x, BigInteger *y, BigInteger *w, BigInteger *h) = 0;
	virtual void setRect(const BigInteger &x, const BigInteger &y, const BigInteger &w, const BigInteger &h) = 0;
	virtual void paint(CanvasPainter *painter, const BigInteger &x, const BigInteger &y, int w, int h) = 0;
	virtual void runStep() = 0;

signals:
	void rectChanged();
	void gridChanged();
};

#endif
