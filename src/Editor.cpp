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

#include <QActionGroup>
#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QToolBar>

#include "AbstractAlgorithm.h"
#include "AlgorithmManager.h"
#include "CanvasPainter.h"
#include "DataChannel.h"
#include "Editor.h"

static const uint maxScalePixel = 4;
// Scale ratio is: 2^scale: 2^scalePixel
// Max scale: 1: 16

Editor::Editor(QWidget *parent)
	: QWidget(parent), m_rect_x1(0), m_rect_y1(0), m_rect_x2(0), m_rect_y2(0), m_view_x(0), m_view_y(0),
	  m_scroll_last_x(0), m_scroll_last_y(0),
	  m_drawing(false), m_editMode(DrawFreehand),
	  m_scale(0), m_scalePixel(maxScalePixel)
{
/*	QToolBar *toolbar = new QToolBar();
    KAction *stepAction = new KAction(this);
	stepAction->setText(i18n("Run a single step"));
	stepAction->setIcon(KIcon("arrow-right"));
	stepAction->setToolTip("Run a single step");
	stepAction->setShortcut(Qt::Key_Space);
	static_cast<KXmlGuiWindow *>(parent)->actionCollection()->addAction("step", stepAction);
	connect(stepAction, SIGNAL(triggered()), AlgorithmManager::self(), SLOT(runStep()));
	toolbar->addAction(stepAction);

	toolbar->addSeparator();

	QActionGroup *editTools = new QActionGroup(this);
	KAction *freehandAction = new KAction(this);
	freehandAction->setText(i18n("Draw freehand"));
	freehandAction->setIcon(KIcon("draw-brush"));
	freehandAction->setToolTip(i18n("Draw freehand"));
	freehandAction->setCheckable(true);
	freehandAction->setChecked(true);
	static_cast<KXmlGuiWindow *>(parent)->actionCollection()->addAction("draw-freehand", freehandAction);
	connect(freehandAction, SIGNAL(triggered()), this, SLOT(freehandAction()));
	editTools->addAction(freehandAction);
	toolbar->addAction(freehandAction);

	KAction *lineAction = new KAction(this);
	lineAction->setText(i18n("Draw line"));
	lineAction->setIcon(KIcon("draw-line"));
	lineAction->setToolTip(i18n("Draw line"));
	lineAction->setCheckable(true);
	static_cast<KXmlGuiWindow *>(parent)->actionCollection()->addAction("draw-line", lineAction);
	connect(lineAction, SIGNAL(triggered()), this, SLOT(lineAction()));
	editTools->addAction(lineAction);
	toolbar->addAction(lineAction);

	KAction *rectangleAction = new KAction(this);
	rectangleAction->setText(i18n("Draw rectangle"));
	rectangleAction->setIcon(KIcon("draw-rectangle"));
	rectangleAction->setToolTip(i18n("Draw rectangle"));
	rectangleAction->setCheckable(true);
	static_cast<KXmlGuiWindow *>(parent)->actionCollection()->addAction("draw-rectangle", rectangleAction);
	connect(rectangleAction, SIGNAL(triggered()), this, SLOT(rectangleAction()));
	editTools->addAction(rectangleAction);
	toolbar->addAction(rectangleAction);

	KAction *circleAction = new KAction(this);
	circleAction->setText(i18n("Draw circle"));
	circleAction->setIcon(KIcon("draw-circle"));
	circleAction->setToolTip(i18n("Draw circle"));
	circleAction->setCheckable(true);
	static_cast<KXmlGuiWindow *>(parent)->actionCollection()->addAction("draw-circle", circleAction);
	connect(circleAction, SIGNAL(triggered()), this, SLOT(circleAction()));
	editTools->addAction(circleAction);
    toolbar->addAction(circleAction);*/

	m_canvas = new QWidget();
	m_canvas->setMouseTracking(true);
	m_canvas->installEventFilter(this);

	m_vertScroll = new QScrollBar(Qt::Vertical);
	connect(m_vertScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollChanged(int)));
	connect(m_vertScroll, SIGNAL(sliderReleased()), this, SLOT(scrollReleased()));

	m_horiScroll = new QScrollBar(Qt::Horizontal);
	connect(m_horiScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollChanged(int)));
	connect(m_horiScroll, SIGNAL(sliderReleased()), this, SLOT(scrollReleased()));

	rectChanged();
	resetViewPoint();
	connect(AlgorithmManager::self(), SIGNAL(rectChanged()), this, SLOT(rectChanged()));
	connect(AlgorithmManager::self(), SIGNAL(gridChanged()), m_canvas, SLOT(update()));

	QGridLayout *layout = new QGridLayout();
	layout->setVerticalSpacing(0);
	layout->setHorizontalSpacing(0);
//	layout->addWidget(toolbar, 0, 0);
	layout->addWidget(m_canvas, 1, 0);
	layout->addWidget(m_vertScroll, 1, 1);
	layout->addWidget(m_horiScroll, 2, 0);
	setLayout(layout);
}

Editor::~Editor()
{
	delete m_canvas;
}

void Editor::freehandAction()
{
	m_editMode = DrawFreehand;
}

void Editor::lineAction()
{
	m_editMode = DrawLine;
}

void Editor::rectangleAction()
{
	m_editMode = DrawRectangle;
}

void Editor::circleAction()
{
	m_editMode = DrawCircle;
}

void Editor::rectChanged()
{
	BigInteger x, y;
	AlgorithmManager::algorithm()->getRect(&x, &y, &m_rect_w, &m_rect_h);
	m_rect_x1 = x;
	m_rect_y1 = y;
	m_rect_x2 = x + m_rect_w - 1;
	m_rect_y2 = y + m_rect_h - 1;
	viewResized();
}

void Editor::viewResized()
{
	m_vertGridCount = m_canvas->height() >> m_scalePixel;
	m_horiGridCount = m_canvas->width() >> m_scalePixel;
	m_vertEdgeSpacing = m_horiEdgeSpacing = 5;
	disconnect(m_vertScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollChanged(int)));
	if (AlgorithmManager::algorithm()->isVerticalInfinity())
	{
		m_vertScroll->setMinimum(-100);
		m_vertScroll->setMaximum(100);
		m_vertScroll->setSliderPosition(0);
	}
	else
	{
		m_vertScroll->setMinimum(0);
		m_vertScroll->setMaximum(m_rect_h - m_vertGridCount + m_vertEdgeSpacing * 2);
		m_vertScroll->setSliderPosition(0);
	}
	connect(m_vertScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollChanged(int)));

	disconnect(m_horiScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollChanged(int)));
	if (AlgorithmManager::algorithm()->isHorizontalInfinity())
	{
		m_horiScroll->setMinimum(-100);
		m_horiScroll->setMaximum(100);
		m_horiScroll->setSliderPosition(0);
	}
	else
	{
		m_horiScroll->setMinimum(0);
		m_horiScroll->setMaximum(m_rect_w - m_horiGridCount + m_horiEdgeSpacing * 2);
		m_horiScroll->setSliderPosition(0);
	}
	connect(m_horiScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollChanged(int)));
	resetViewPoint();
}

void Editor::resizeEvent(QResizeEvent *)
{
	viewResized();
	emit update();
}

void Editor::scrollChanged(int)
{
	QScrollBar *scrollBar = qobject_cast<QScrollBar *>(sender());
	if (AlgorithmManager::algorithm()->isInfinity(scrollBar->orientation()))
	{
		if (!scrollBar->isSliderDown())
		{
			if (scrollBar->orientation() == Qt::Vertical)
				if (m_scroll_last_y)
					m_scroll_last_y = 0;
				else
					m_view_y += BigInteger(scrollBar->sliderPosition()) << (maxScalePixel - m_scalePixel);
			else
				if (m_scroll_last_x)
					m_scroll_last_x = 0;
				else
					m_view_x += BigInteger(scrollBar->sliderPosition()) << (maxScalePixel - m_scalePixel);
			scrollBar->setSliderPosition(0);
		}
		else
		{
			if (scrollBar->orientation() == Qt::Vertical)
			{
				m_view_y -= BigInteger(m_scroll_last_y - scrollBar->sliderPosition()) << (maxScalePixel - m_scalePixel);
				m_scroll_last_y = scrollBar->sliderPosition();
			}
			else
			{
				m_view_x -= BigInteger(m_scroll_last_x - scrollBar->sliderPosition()) << (maxScalePixel - m_scalePixel);
				m_scroll_last_x = scrollBar->sliderPosition();
			}
		}
		emit update();
		return;
	}
	// FIXME: This is not correct now, fix me when adding back finite grid support
	if (scrollBar->orientation() == Qt::Vertical)
	{
		if (!AlgorithmManager::algorithm()->isVerticalInfinity())
			m_view_y = m_rect_y1 + BigInteger(m_vertScroll->sliderPosition()) - m_vertEdgeSpacing;
	}
	else
	{
		if (!AlgorithmManager::algorithm()->isHorizontalInfinity())
			m_view_x = m_rect_x1 + BigInteger(m_horiScroll->sliderPosition()) - m_horiEdgeSpacing;
	}
	emit update();
}

void Editor::scrollReleased()
{
	scrollChanged(qobject_cast<QScrollBar *>(sender())->sliderPosition());
}

void Editor::setViewPoint(const BigInteger &x, const BigInteger &y)
{
	// FIXME: This is not correct now, fix me when adding back finite grid support
	if (!AlgorithmManager::algorithm()->isVerticalInfinity())
		m_vertScroll->setSliderPosition(y - m_rect_y1 + m_vertEdgeSpacing);
	else
		m_view_y = y;

	if (!AlgorithmManager::algorithm()->isHorizontalInfinity())
		m_horiScroll->setSliderPosition(x - m_rect_x1 + m_horiEdgeSpacing);
	else
		m_view_x = x;
}

void Editor::resetViewPoint()
{
	setViewPoint(m_view_x, m_view_y);
}

void Editor::scaleView(int scaleDelta, uint anchor_x, uint anchor_y)
{
	if (scaleDelta)
	{
        uint old_scale = m_scale, old_scalePixel = m_scalePixel;
		if (scaleDelta > 0)
		{
            uint d = qMin<uint>(m_scale, scaleDelta);
			m_scale -= d;
            m_scalePixel = qMin<uint>(maxScalePixel, m_scalePixel + scaleDelta - d);
		}
		else
		{
            uint d = qMin<uint>(m_scalePixel, -scaleDelta);
            m_scalePixel -= d;
			m_scale += -scaleDelta - d;
		}
		// Formula:
		// (view_x + x / 2^scalePixel) << scale == (old_view_x + x / 2^old_scalePixel) << old_scale
		// FIXME: This is not correct now, fix me when adding back finite grid support
		m_view_y = ((m_view_y + (anchor_y >> old_scalePixel)) << old_scale >> m_scale) - (anchor_y >> m_scalePixel);
		if (!AlgorithmManager::algorithm()->isVerticalInfinity())
			m_view_y = qBound(m_rect_y1, m_view_y, m_rect_y2);

		m_view_x = ((m_view_x + (anchor_x >> old_scalePixel)) << old_scale >> m_scale) - (anchor_x >> m_scalePixel);
		if (!AlgorithmManager::algorithm()->isHorizontalInfinity())
			m_view_x = qBound(m_rect_x1, m_view_x, m_rect_x2);

		viewResized();
	}
}

void Editor::setGrid(int gridx, int gridy, int state, CanvasPainter *painter)
{
	if (painter) // preview mode
	{
		if (gridx >= 0 && gridx < painter->width() && gridy >= 0 && gridy < painter->height())
			painter->drawGrid(gridx, gridy, state);
	}
	else
	{
		BigInteger x = m_view_x + gridx;
		BigInteger y = m_view_y + gridy;
		bool inRange = true;
		if (!AlgorithmManager::algorithm()->isVerticalInfinity())
			inRange &= m_rect_x1 <= x && x <= m_rect_x2;
		if (!AlgorithmManager::algorithm()->isHorizontalInfinity())
			inRange &= m_rect_y1 <= y && y <= m_rect_y2;
		if (inRange)
			AlgorithmManager::algorithm()->setGrid(x, y, state);
	}
}

void Editor::setLine(int x1, int y1, int x2, int y2, int state, CanvasPainter *painter)
{
	// Bresenham's line algorithm
	// See http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
	int dx = qAbs(x2 - x1), dy = qAbs(y2 - y1);
	int sx = x1 < x2? 1: -1, sy = y1 < y2? 1: -1;
	int err = dx - dy;
	forever
	{
		setGrid(x1, y1, state, painter);
		if (x1 == x2 && y1 == y2)
			return;
		int e2 = err * 2;
		if (e2 > -dy)
		{
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y1 += sy;
		}
	}
}

void Editor::setRectangle(int x1, int y1, int x2, int y2, int state, CanvasPainter *painter)
{
	if (x2 < x1)
		qSwap(x1, x2);
	if (y2 < y1)
		qSwap(y1, y2);
	if (painter)
	{
		x1 = qMax(0, x1);
		y1 = qMax(0, y1);
		x2 = qMin(painter->width(), x2);
		y2 = qMin(painter->height(), y2);
		for (int x = x1; x <= x2; x++)
		{
			painter->drawGrid(x, y1, state);
			painter->drawGrid(x, y2, state);
		}
		for (int y = y1 + 1; y <= y2 - 1; y++)
		{
			painter->drawGrid(x1, y, state);
			painter->drawGrid(x2, y, state);
		}
	}
	else
	{
		AlgorithmManager::algorithm()->setReceiveRect((m_view_x + x1) << m_scale, (m_view_y + y1) << m_scale, x2 - x1 + 1, y2 - y1 + 1);
		DataChannel *channel = DataChannel::transferTo(AlgorithmManager::algorithm());
		channel->send(state, x2 - x1 + 1);
		channel->send(DATACHANNEL_EOLN, 1);
		for (int y = y1 + 1; y <= y2 - 1; y++)
		{
			channel->send(state, 1);
			channel->send(0, x2 - x1 - 1);
			channel->send(state, 1);
			channel->send(DATACHANNEL_EOLN, 1);
		}
		channel->send(state, x2 - x1 + 1);
		channel->send(DATACHANNEL_EOF, 1);
	}
}

void Editor::setCircle(int x1, int y1, int x2, int y2, int state, CanvasPainter *painter)
{
	// Bresenham's circle algorithm
	// See http://en.wikipedia.org/wiki/Bresenham's_circle_algorithm
	int d = qMin(qAbs(x1 - x2), qAbs(y1 - y2)), r = d / 2;
	int cx, cy, xp = 0, xn = 0, yp = 0, yn = 0;
	if (x1 < x2)
	{
		cx = x1 + r;
		xp = d % 2;
	}
	else
	{
		cx = x1 - r;
		xn = d % 2;
	}
	if (y1 < y2)
	{
		cy = y1 + r;
		yp = d % 2;
	}
	else
	{
		cy = y1 - r;
		yn = d % 2;
	}
	int x = r, y = 0, err = -r;
	while (x >= y)
	{
		setGrid(cx - x - xn, cy - y - yn, state, painter);
		setGrid(cx - y - xn, cy - x - yn, state, painter);
		setGrid(cx + y + xp, cy - x - yn, state, painter);
		setGrid(cx + x + xp, cy - y - yn, state, painter);
		setGrid(cx - x - xn, cy + y + yp, state, painter);
		setGrid(cx - y - xn, cy + x + yp, state, painter);
		setGrid(cx + y + xp, cy + x + yp, state, painter);
		setGrid(cx + x + xp, cy + y + yp, state, painter);

		err += y;
		y++;
		err += y;

		if (err >= 0)
		{
			err -= x;
			--x;
			err -= x;
		}
	}
}

bool Editor::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == m_canvas)
	{
		switch (event->type())
		{
		case QEvent::Paint:
		{
            uint scale = qMax(m_scale, 0U);
			// Calculate grid size
			int y1 = 0, y2 = m_vertGridCount;
			if (!AlgorithmManager::algorithm()->isVerticalInfinity())
			{
				y1 = qMax(y1, static_cast<int>((m_rect_y1 >> scale) - m_view_y));
				y2 = qMin(y2, static_cast<int>((m_rect_y2 >> scale) - m_view_y));
			}
			int x1 = 0, x2 = m_horiGridCount;
			if (!AlgorithmManager::algorithm()->isHorizontalInfinity())
			{
				x1 = qMax(x1, static_cast<int>((m_rect_x1 >> scale) - m_view_x));
				x2 = qMin(x2, static_cast<int>((m_rect_x2 >> scale) - m_view_x));
			}
            CanvasPainter painter(m_canvas, m_view_x, m_view_y, x1, x2, y1, y2, qMax(m_scale, 0U), m_scalePixel);
			if (m_drawing)
			{
				if (m_editMode == DrawLine)
					setLine(m_draw_start_x, m_draw_start_y, m_mouseMove_last_x, m_mouseMove_last_y, 1, &painter);
                                else if (m_editMode == DrawRectangle)
                                        setRectangle(m_draw_start_x, m_draw_start_y, m_mouseMove_last_x, m_mouseMove_last_y, 1, &painter);
				else if (m_editMode == DrawCircle)
					setCircle(m_draw_start_x, m_draw_start_y, m_mouseMove_last_x, m_mouseMove_last_y, 1, &painter);
			}
			painter.drawPattern();
			painter.drawGridLine();
			break;
		}

		case QEvent::MouseMove:
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		{
			QMouseEvent *e = static_cast<QMouseEvent *>(event);
			int x = e->x() >> m_scalePixel, y = e->y() >> m_scalePixel;
			if (!m_scale)
			{
				if (e->type() == QEvent::MouseButtonRelease)
				{
					if (m_drawing)
					{
						if (m_editMode == DrawLine)
							setLine(m_draw_start_x, m_draw_start_y, x, y, 1);
                                                else if (m_editMode == DrawRectangle)
							setRectangle(m_draw_start_x, m_draw_start_y, x, y, 1);
						else if (m_editMode == DrawCircle)
							setCircle(m_draw_start_x, m_draw_start_y, x, y, 1);
						m_drawing = false;
					}
				}
				else if (e->buttons() & Qt::LeftButton)
				{
					if (e->type() == QEvent::MouseButtonPress)
					{
						if (m_editMode == DrawFreehand)
							setGrid(x, y, 1);
						else
						{
							m_draw_start_x = x;
							m_draw_start_y = y;
						}
						m_drawing = true;
					}
					else
					{
						// MouseMove event is not sent per pixel
						// Use lines to fill the gap
						if (m_editMode == DrawFreehand)
							setLine(m_mouseMove_last_x, m_mouseMove_last_y, x, y, 1);
						else
						{
							m_mouseMove_last_x = x;
							m_mouseMove_last_y = y;
							m_canvas->update();
						}
					}
				}
			}
			m_mouseMove_last_x = x;
			m_mouseMove_last_y = y;
			emit coordinateChanged((m_view_x + x) << m_scale, (m_view_y + y) << m_scale);
			break;
		}

		case QEvent::Wheel:
		{
			QWheelEvent *e = static_cast<QWheelEvent *>(event);
			if (e->modifiers() & Qt::ControlModifier) // Scale view
			{
				scaleView(e->delta() / 120, e->x(), e->y());
				emit update();
			}
			else
			{
				if (e->orientation() == Qt::Vertical)
					QApplication::sendEvent(m_vertScroll, e);
				else
					QApplication::sendEvent(m_horiScroll, e);
			}
			BigInteger x = (m_view_x + (e->x() >> m_scalePixel)) << m_scale;
			BigInteger y = (m_view_y + (e->y() >> m_scalePixel)) << m_scale;
			emit coordinateChanged(x, y);
			break;
		}

		default:; // get rid of gcc warnings
		}
	}
	return QWidget::eventFilter(obj, event);
}
