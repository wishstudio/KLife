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
	void freehandAction();
	void lineAction();
	void rectangleAction();
	void circleAction();

	void rectChanged();
	void scrollChanged(int);
	void scrollReleased();

signals:
	void coordinateChanged(const BigInteger &x, const BigInteger &y);

protected:
	void resizeEvent(QResizeEvent *);

private:
	void resetViewPoint();
	void viewResized();
	void scaleView(int scaleDelta, size_t anchor_x, size_t anchor_y);
	void setGrid(int gridx, int gridy, int state, CanvasPainter *painter = NULL);
	void setLine(int x1, int y1, int x2, int y2, int state, CanvasPainter *painter = NULL);
	void setRectangle(int x1, int y1, int x2, int y2, int state, CanvasPainter *painter = NULL);
	void setCircle(int x1, int y1, int x2, int y2, int state, CanvasPainter *painter = NULL);
	bool eventFilter(QObject *obj, QEvent *event);

	QWidget *m_canvas;
	QScrollBar *m_vertScroll, *m_horiScroll;

	BigInteger m_mouseLastX, m_mouseLastY;
	BigInteger m_rect_x1, m_rect_y1, m_rect_x2, m_rect_y2, m_rect_w, m_rect_h;
	// The coordinate of the upper-left grid of the view in the whole pattern (not scaled)
	BigInteger m_view_x, m_view_y;
	int m_scroll_last_x, m_scroll_last_y;

	// These integer coordinates represent the offset to the upper-left grid of the view (not scaled)
	bool m_drawing;
	int m_draw_start_x, m_draw_start_y;
	enum {DrawFreehand, DrawLine, DrawRectangle, DrawCircle} m_editMode;
	int m_mouseMove_last_x, m_mouseMove_last_y;

	int m_vertEdgeSpacing, m_horiEdgeSpacing;
	int m_vertGridCount, m_horiGridCount;
	size_t m_scale, m_scalePixel;
};

#endif
