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

#include "AbstractAlgorithm.h"

void AbstractAlgorithm::getRect(BigInteger *x, BigInteger *y, BigInteger *w, BigInteger *h)
{
	*x = m_x;
	*y = m_y;
	*w = m_w;
	*h = m_h;
}

void AbstractAlgorithm::setRect(const BigInteger &x, const BigInteger &y, const BigInteger &w, const BigInteger &h)
{
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	rectChange(x, y, w, h);
}

void AbstractAlgorithm::setVerticalInfinity(bool infinity)
{
	m_vertInfinity = infinity;
}

void AbstractAlgorithm::setHorizontalInfinity(bool infinity)
{
	m_horiInfinity = infinity;
}

bool AbstractAlgorithm::isInfinity(Qt::Orientation orientation)
{
	if (orientation == Qt::Vertical)
		return isVerticalInfinity();
	return isHorizontalInfinity();
}

void AbstractAlgorithm::setInfinity(bool vertInfinity, bool horiInfinity)
{
	setVerticalInfinity(vertInfinity);
	setHorizontalInfinity(horiInfinity);
	infinityChange();
}

void AbstractAlgorithm::setAcceptInfinity(bool acceptInfinity)
{
	m_acceptInfinity = acceptInfinity;
}
