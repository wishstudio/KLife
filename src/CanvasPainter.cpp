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

CanvasPainter::CanvasPainter(QPaintDevice *device, const BigInteger &view_x, const BigInteger &view_y, int x1, int x2, int y1, int y2, int scale, int scalePixel)
	: QPainter(device), m_view_x(view_x), m_view_y(view_y), m_scalePixel(scalePixel), m_x1(x1), m_x2(x2), m_y1(y1), m_y2(y2), m_w(x2 - x1 + 1), m_h(y2 - y1 + 1)
{
	// Draw background
	fillRect(0, 0, device->width(), device->height(), QColor(0x80, 0x80, 0x80));

	m_data = (QRgb *) malloc(m_w * m_h * sizeof(QRgb));
	// Draw grid
	AlgorithmManager::algorithm()->paint(this, view_x + x1, view_y + y1, m_w, m_h, scale);
}

CanvasPainter::~CanvasPainter()
{
	free(m_data);
}

void CanvasPainter::drawPattern()
{
	QImage image(reinterpret_cast<uchar *>(m_data), m_w, m_h, QImage::Format_RGB32);
	drawImage(m_x1, m_y1, image.scaled(m_w << m_scalePixel, m_h << m_scalePixel));
}

void CanvasPainter::drawGridLine()
{
	if (m_scalePixel > 1)
	{
		// Draw vertical grid line
		for (int i = m_x1; i < m_x2; i++)
		{
			setPen((i + 1 + m_view_x) % 10 == 0? QColor(0x70, 0x70, 0x70): QColor(0x50, 0x50, 0x50));
			drawLine(((i + 1) << m_scalePixel) - 1, m_y1 << m_scalePixel, ((i + 1) << m_scalePixel) - 1, ((m_y2 + 1) << m_scalePixel) - 1);
		}
		// Draw horizontal grid line
		for (int i = m_y1; i < m_y2; i++)
		{
			setPen((i + 1 + m_view_y) % 10 == 0? QColor(0x70, 0x70, 0x70): QColor(0x50, 0x50, 0x50));
			drawLine(m_x1 << m_scalePixel, ((i + 1) << m_scalePixel) - 1, ((m_x2 + 1) << m_scalePixel) - 1, ((i + 1) << m_scalePixel) - 1);
		}
	}
}
