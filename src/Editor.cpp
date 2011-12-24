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

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QToolBar>

#include <KAction>
#include <KActionCollection>
#include <KIcon>
#include <KXmlGuiWindow>

#include "AbstractAlgorithm.h"
#include "AlgorithmManager.h"
#include "CanvasPainter.h"
#include "Editor.h"

static const size_t maxScalePixel = 4;
// Scale ratio is: 2^scale: 2^scalePixel
// Max scale: 1: 16

Editor::Editor(QWidget *parent)
	: QWidget(parent), m_rect_x1(0), m_rect_y1(0), m_rect_x2(0), m_rect_y2(0), m_view_x(0), m_view_y(0),
	  m_scroll_last_x(0), m_scroll_last_y(0),
	  m_scale(0), m_scalePixel(maxScalePixel)
{
	QToolBar *toolbar = new QToolBar();
	KAction *stepAction = new KAction(this);
	stepAction->setText("Step");
	stepAction->setIcon(KIcon("arrow-right"));
	stepAction->setToolTip("Run a single step");
	stepAction->setShortcut(Qt::Key_Space);
	static_cast<KXmlGuiWindow *>(parent)->actionCollection()->addAction("step", stepAction);
	connect(stepAction, SIGNAL(triggered()), AlgorithmManager::self(), SLOT(runStep()));
	toolbar->addAction(stepAction);

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
	layout->addWidget(toolbar, 0, 0);
	layout->addWidget(m_canvas, 1, 0);
	layout->addWidget(m_vertScroll, 1, 1);
	layout->addWidget(m_horiScroll, 2, 0);
	setLayout(layout);
}

Editor::~Editor()
{
	delete m_canvas;
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
					m_view_y += scrollBar->sliderPosition();
			else
				if (m_scroll_last_x)
					m_scroll_last_x = 0;
				else
					m_view_x += scrollBar->sliderPosition();
			scrollBar->setSliderPosition(0);
		}
		else
		{
			if (scrollBar->orientation() == Qt::Vertical)
			{
				m_view_y -= m_scroll_last_y - scrollBar->sliderPosition();
				m_scroll_last_y = scrollBar->sliderPosition();
			}
			else
			{
				m_view_x -= m_scroll_last_x - scrollBar->sliderPosition();
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

void Editor::scaleView(int scaleDelta, size_t anchor_x, size_t anchor_y)
{
	if (scaleDelta)
	{
		size_t old_scale = m_scale, old_scalePixel = m_scalePixel;
		if (scaleDelta > 0)
		{
			size_t d = qMin(m_scale, static_cast<size_t>(scaleDelta));
			m_scale -= d;
			m_scalePixel = qMin(maxScalePixel, m_scalePixel + scaleDelta - d);
		}
		else
		{
			size_t d = qMin(m_scalePixel, static_cast<size_t>(-scaleDelta));
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

void Editor::setGrid(int gridx, int gridy, int state, bool previewMode)
{
	if (previewMode)
	{
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

void Editor::setLine(int x1, int y1, int x2, int y2, int state, bool previewMode)
{
	// Bresenham's line algorithm
	// See http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
	int dx = qAbs(x2 - x1), dy = qAbs(y2 - y1);
	int sx = x1 < x2? 1: -1, sy = y1 < y2? 1: -1;
	int err = dx - dy;
	forever
	{
		setGrid(x1, y1, state, previewMode);
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

bool Editor::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == m_canvas)
	{
		switch (event->type())
		{
		case QEvent::Paint:
		{
			size_t scale = qMax(m_scale, 0UL);
			// Calculate grid size
			int y1 = 0, y2 = m_vertGridCount;
			if (!AlgorithmManager::algorithm()->isVerticalInfinity())
			{
				y1 = qMax(y1, (int) ((m_rect_y1 >> scale) - m_view_y));
				y2 = qMin(y2, (int) ((m_rect_y2 >> scale) - m_view_y));
			}
			int x1 = 0, x2 = m_horiGridCount;
			if (!AlgorithmManager::algorithm()->isHorizontalInfinity())
			{
				x1 = qMax(x1, (int) ((m_rect_x1 >> scale) - m_view_x));
				x2 = qMin(x2, (int) ((m_rect_x2 >> scale) - m_view_x));
			}
			CanvasPainter painter(m_canvas, m_view_x, m_view_y, x1, x2, y1, y2, qMax(m_scale, 0UL), m_scalePixel);
			break;
		}

		case QEvent::MouseMove:
		case QEvent::MouseButtonPress:
		{
			QMouseEvent *e = static_cast<QMouseEvent *>(event);
			BigInteger x = (m_view_x + (e->x() >> m_scalePixel)) << m_scale;
			BigInteger y = (m_view_y + (e->y() >> m_scalePixel)) << m_scale;
			if (!m_scale)
			{
				if (e->buttons() & Qt::LeftButton)
				{
					// Mouse event is not sent per pixel
					// Emulate lines to fill the gap
					if (e->type() == QEvent::MouseButtonPress)
						setGrid(e->x() >> m_scalePixel, e->y() >> m_scalePixel, 1);
					else
						setLine(m_mouseMove_last_x, m_mouseMove_last_y, e->x() >> m_scalePixel, e->y() >> m_scalePixel, 1);
				}
			}
			m_mouseMove_last_x = e->x() >> m_scalePixel;
			m_mouseMove_last_y = e->y() >> m_scalePixel;
			emit coordinateChanged(x, y);
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
