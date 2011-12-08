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
#include "MainCanvas.h"

MainCanvas::MainCanvas(QWidget *parent)
	: QWidget(parent), m_x(0), m_y(0), m_scale(16), m_rect_x1(0), m_rect_y1(0), m_rect_x2(0), m_rect_y2(0)
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
	m_canvas->setMouseTracking(false);
	m_canvas->installEventFilter(this);

	m_vertScroll = new QScrollBar(Qt::Vertical);
	connect(m_vertScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollChanged(int)));
	connect(m_vertScroll, SIGNAL(sliderReleased()), this, SLOT(scrollReleased()));

	m_horiScroll = new QScrollBar(Qt::Horizontal);
	connect(m_horiScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollChanged(int)));
	connect(m_horiScroll, SIGNAL(sliderReleased()), this, SLOT(scrollReleased()));

	rectChanged();
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

MainCanvas::~MainCanvas()
{
	delete m_canvas;
}

void MainCanvas::rectChanged()
{
	int x, y, w, h;
	AlgorithmManager::algorithm()->getRect(&x, &y, &w, &h);
	if (AlgorithmManager::algorithm()->isVerticalInfinity())
	{
		m_vertScroll->setMinimum(-100);
		m_vertScroll->setMaximum(100);
		m_vertScroll->setSliderPosition(0);
		m_vertScroll->setPageStep(42);
	}
	else
	{
		m_vertScroll->setMinimum(y - m_canvas->height() / m_scale + 1);
		m_vertScroll->setMaximum(y + h - 1);
		m_vertScroll->setSliderPosition(0);
	}

	if (AlgorithmManager::algorithm()->isHorizontalInfinity())
	{
		m_horiScroll->setMinimum(-100);
		m_horiScroll->setMaximum(100);
		m_horiScroll->setSliderPosition(0);
		m_horiScroll->setPageStep(42);
	}
	else
	{
		m_horiScroll->setMinimum(x - m_canvas->width() / m_scale + 1);
		m_horiScroll->setMaximum(x + w - 1);
		m_horiScroll->setSliderPosition(0);
	}
	m_rect_x1 = x;
	m_rect_y1 = y;
	m_rect_x2 = x + w - 1;
	m_rect_y2 = y + h - 1;
}

void MainCanvas::resizeEvent(QResizeEvent *)
{
	rectChanged();
}

void MainCanvas::scrollChanged(int)
{
	QScrollBar *scrollBar = qobject_cast<QScrollBar *>(sender());
	if (!scrollBar->isSliderDown() && AlgorithmManager::algorithm()->isInfinity(scrollBar->orientation()))
	{
		if (scrollBar->sliderPosition() == 0)
			return;
		scrollBar->setSliderPosition(0);
	}
	// Vertical
	if (!AlgorithmManager::algorithm()->isVerticalInfinity())
		m_y = m_vertScroll->sliderPosition();
	// Horizontal
	if (!AlgorithmManager::algorithm()->isHorizontalInfinity())
		m_x = m_horiScroll->sliderPosition();
	emit update();
}

void MainCanvas::scrollReleased()
{
	scrollChanged(qobject_cast<QScrollBar *>(sender())->sliderPosition());
}

bool MainCanvas::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == m_canvas)
	{
		switch (event->type())
		{
		case QEvent::Paint:
		{
			QPainter painter(m_canvas);
			painter.fillRect(m_canvas->rect(), QColor(0x80, 0x80, 0x80));
			// Draw grid
			int y1 = 0, y2 = m_canvas->height() / m_scale;
			if (!AlgorithmManager::algorithm()->isVerticalInfinity())
			{
				y1 = qMax(y1, m_rect_y1 - m_y);
				y2 = qMin(y2, m_rect_y2 - m_y);
			}
			int x1 = 0, x2 = m_canvas->width() / m_scale;
			if (!AlgorithmManager::algorithm()->isHorizontalInfinity())
			{
				x1 = qMax(x1, m_rect_x1 - m_x);
				x2 = qMin(x2, m_rect_x2 - m_x);
			}
			for (int x = x1; x <= x2; x++)
				for (int y = y1; y <= y2; y++)
				{
					QColor fillColor;
					if (AlgorithmManager::algorithm()->grid(x + m_x, y + m_y))
						fillColor = Qt::white;
					else
						fillColor = QColor(0x30, 0x30, 0x30);
					painter.fillRect(x * m_scale, y * m_scale, m_scale, m_scale, fillColor);
				}
			// Draw vertical grid line
			//int j = 0;
			for (int i = x1; i < x2; i++)
			{
				painter.setPen((i + 1 + m_x) % 10 == 0? QColor(0x70, 0x70, 0x70): QColor(0x50, 0x50, 0x50));
				painter.drawLine((i + 1) * m_scale - 1, y1 * m_scale, (i + 1) * m_scale - 1, (y2 + 1) * m_scale - 1);
			}
			// Draw horizontal grid line
			for (int i = y1; i < y2; i++)
			{
				painter.setPen((i + 1 + m_y) % 10 == 0? QColor(0x70, 0x70, 0x70): QColor(0x50, 0x50, 0x50));
				painter.drawLine(x1 * m_scale, (i + 1) * m_scale - 1, (x2 + 1) * m_scale - 1, (i + 1) * m_scale - 1);
			}
			break;
		}

		case QEvent::MouseMove:
		case QEvent::MouseButtonPress:
		{
			QMouseEvent *e = static_cast<QMouseEvent *>(event);
			int x = m_x + e->x() / m_scale, y = m_y + e->y() / m_scale;
			bool inRange = true;
			if (!AlgorithmManager::algorithm()->isVerticalInfinity())
				inRange &= m_rect_x1 <= x && x <= m_rect_x2;
			if (!AlgorithmManager::algorithm()->isHorizontalInfinity())
				inRange &= m_rect_y1 <= y && y <= m_rect_y2;
			if (inRange)
				AlgorithmManager::algorithm()->setGrid(x, y, 1);
			break;
		}

		case QEvent::Wheel:
		{
			QWheelEvent *e = static_cast<QWheelEvent *>(event);
			if (e->orientation() == Qt::Vertical)
				QApplication::sendEvent(m_vertScroll, e);
			else
				QApplication::sendEvent(m_horiScroll, e);
			break;
		}

		default:; // get rid of gcc warnings
		}
	}
	return QWidget::eventFilter(obj, event);
}
