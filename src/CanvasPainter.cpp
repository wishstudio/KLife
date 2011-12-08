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
#include "AlgorithmManager.h"
#include "BigInteger.h"
#include "CanvasPainter.h"

CanvasPainter::CanvasPainter(QPaintDevice *device, const BigInteger &view_x, const BigInteger &view_y, int x1, int x2, int y1, int y2, int scalePixel)
	: QPainter(device), m_scalePixel(scalePixel), m_x(x1), m_y(y1)
{
	// Draw background
	fillRect(0, 0, device->width(), device->height(), QColor(0x80, 0x80, 0x80));

	// Draw grid
	AlgorithmManager::algorithm()->paint(this, view_x + x1, view_y + y1, x2 - x1 + 1, y2 - y1 + 1);

	// Draw grid line
	if (scalePixel > 2)
	{
		// Draw vertical grid line
		for (int i = x1; i < x2; i++)
		{
			setPen((i + 1 + view_x) % 10 == 0? QColor(0x70, 0x70, 0x70): QColor(0x50, 0x50, 0x50));
			drawLine((i + 1) * scalePixel - 1, y1 * scalePixel, (i + 1) * scalePixel - 1, (y2 + 1) * scalePixel - 1);
		}
		// Draw horizontal grid line
		for (int i = y1; i < y2; i++)
		{
			setPen((i + 1 + view_y) % 10 == 0? QColor(0x70, 0x70, 0x70): QColor(0x50, 0x50, 0x50));
			drawLine(x1 * scalePixel, (i + 1) * scalePixel - 1, (x2 + 1) * scalePixel - 1, (i + 1) * scalePixel - 1);
		}
	}
}

CanvasPainter::~CanvasPainter()
{
}

void CanvasPainter::drawGrid(int x, int y, int state)
{
	QColor fillColor = state? Qt::white: QColor(0x30, 0x30, 0x30);
	fillRect((m_x + x) * m_scalePixel, (m_y + y) * m_scalePixel, m_scalePixel, m_scalePixel, fillColor);
}