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

#ifndef CANVASPAINTER_H
#define CANVASPAINTER_H

#include <QRgb>
#include <QPainter>

class BigInteger;
class CanvasPainter: public QPainter
{
public:
	CanvasPainter(QPaintDevice *device, const BigInteger &view_x, const BigInteger &view_y, int x1, int x2, int y1, int y2, int scale, int scalePixel);
	virtual ~CanvasPainter();

	// This function is very time consuming
	// So trying inline for some speed improvement
	inline void drawGrid(int x, int y, int state)
	{
		m_data[y * m_w + x] = state? qRgb(0xFF, 0xFF, 0xFF): qRgb(0x30, 0x30, 0x30);
	}

	inline void fillBlack()
	{
		memset(m_data, 0x30, m_w * m_h * sizeof(QRgb));
	}

private:
	QRgb *m_data;
	int m_scalePixel, m_x, m_y, m_w, m_h;
};

#endif
