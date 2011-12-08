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

#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KStandardAction>

#include "AbstractAlgorithm.h"
#include "AlgorithmManager.h"
#include "MainCanvas.h"
#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
	: KXmlGuiWindow(parent)
{
	AlgorithmManager::algorithm()->setRect(0, 0, 100, 100);
	canvas = new MainCanvas(this);
	setCentralWidget(canvas);
	setupActions();
}

MainWindow::~MainWindow()
{
	delete canvas;
}

void MainWindow::setupActions()
{
	KAction *newAction = new KAction(this);
	newAction->setText(i18n("&New..."));
	newAction->setIcon(KIcon("document-new"));
	newAction->setShortcut(Qt::CTRL + Qt::Key_N);
	actionCollection()->addAction("new", newAction);
	//connect(newAction, SIGNAL(triggered()), textArea, SLOT(clear()));

	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

	setupGUI();
}
