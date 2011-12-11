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

#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>

#include "BigInteger.h"

class QScrollBar;
class Editor: public QWidget
{
	Q_OBJECT

public:
	Editor(QWidget *parent = 0);
	virtual ~Editor();

	void setViewPoint(const BigInteger &x, const BigInteger &y);

public slots:
	void rectChanged();
	void scrollChanged(int);
	void scrollReleased();

protected:
	void resizeEvent(QResizeEvent *);

private:
	void resetViewPoint();
	void viewResized();
	void scaleView(int scale, int anchor_x, int anchor_y);
	bool eventFilter(QObject *obj, QEvent *event);

	QWidget *m_canvas;
	QScrollBar *m_vertScroll, *m_horiScroll;

	BigInteger m_rect_x1, m_rect_y1, m_rect_x2, m_rect_y2, m_rect_w, m_rect_h;
	BigInteger m_view_x, m_view_y;
	int m_vertEdgeSpacing, m_horiEdgeSpacing;
	int m_scale, m_scalePixel, m_vertGridCount, m_horiGridCount;
};

#endif
