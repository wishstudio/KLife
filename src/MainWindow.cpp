/*
 *   Copyright (C) 2011,2012 by Xiangyan Sun <wishstudio@gmail.com>
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

#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>

#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KStandardAction>
#include <KStatusBar>

#include "AbstractAlgorithm.h"
#include "AlgorithmManager.h"
#include "Editor.h"
#include "FileFormatManager.h"
#include "MainWindow.h"
#include "RuleLife.h"

MainWindow::MainWindow(QWidget *parent)
	: KXmlGuiWindow(parent)
{
	statusBar()->setSizeGripEnabled(true);
	setupActions();

	{
		QWidget *form = new QWidget();
		form->setStyleSheet("font-family: Monospace; font-size: 11px;");
		form->setMinimumWidth(200);
		QFormLayout *layout = new QFormLayout();
		layout->setMargin(0);
		layout->setSpacing(0);
		m_generation = new QLabel();
		m_generation->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		layout->addRow(new QLabel(i18n("Generation:")), m_generation);
		m_population = new QLabel();
		m_population->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		layout->addRow(new QLabel(i18n("Population:")), m_population);
		form->setLayout(layout);
		statusBar()->addWidget(form);
	}
	{
		QWidget *form = new QWidget();
		form->setStyleSheet("font-family: Monospace; font-size: 11px;");
		form->setMinimumWidth(130);
		QFormLayout *layout = new QFormLayout();
		layout->setMargin(0);
		layout->setSpacing(0);
		m_coordinate_x = new QLabel();
		m_coordinate_x->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		layout->addRow(new QLabel("X:"), m_coordinate_x);
		m_coordinate_y = new QLabel();
		m_coordinate_y->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		layout->addRow(new QLabel("Y:"), m_coordinate_y);
		form->setLayout(layout);
		statusBar()->addWidget(form);
	}
	connect(AlgorithmManager::self(), SIGNAL(gridChanged()), this, SLOT(gridChanged()));
	{
		QWidget *form = new QWidget();
		form->setStyleSheet("font-family: Monospace; font-size: 11px;");
		form->setMinimumWidth(200);
		QFormLayout *layout = new QFormLayout();
		layout->setMargin(0);
		layout->setSpacing(0);
		m_rule = new QLabel();
		m_rule->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		layout->addRow(new QLabel(i18n("Rule:")), m_rule);
		layout->addRow(new QWidget());
		form->setLayout(layout);
		statusBar()->addWidget(form);
	}
	connect(AlgorithmManager::self(), SIGNAL(ruleChanged()), this, SLOT(ruleChanged()));

	// TODO
	AlgorithmManager::setRule(new RuleLife("3", "23"));
	AlgorithmManager::algorithm()->setInfinity(true, true);
	gridChanged();

	m_editor = new Editor(this);
	connect(m_editor, SIGNAL(coordinateChanged(const BigInteger &, const BigInteger &)), this, SLOT(coordinateChanged(const BigInteger &, const BigInteger &)));
	setCentralWidget(m_editor);
}

void MainWindow::coordinateChanged(const BigInteger &x, const BigInteger &y)
{
	m_coordinate_x->setText(x);
	m_coordinate_y->setText(y);
}

void MainWindow::gridChanged()
{
	m_generation->setText(AlgorithmManager::algorithm()->generation());
	m_population->setText(AlgorithmManager::algorithm()->population());
}

void MainWindow::ruleChanged()
{
	m_rule->setText(QString("%1(%2)").arg(AlgorithmManager::rule()->string(), AlgorithmManager::rule()->name()));
}

void MainWindow::newAction()
{
	AlgorithmManager::algorithm()->clearGrid();
}

void MainWindow::openAction()
{
	QString fileName = QFileDialog::getOpenFileName(this, i18n("Open pattern"), QString(), FileFormatManager::fileFilter());
	if (!FileFormatManager::readFile(fileName))
		return;
}

void MainWindow::setupActions()
{
	KAction *newAction = new KAction(this);
	newAction->setText(i18n("&New pattern..."));
	newAction->setIcon(KIcon("document-new"));
	newAction->setShortcut(Qt::CTRL + Qt::Key_N);
	actionCollection()->addAction("new", newAction);
	connect(newAction, SIGNAL(triggered()), this, SLOT(newAction()));

	KAction *openAction = new KAction(this);
	openAction->setText(i18n("&Open pattern..."));
	openAction->setIcon(KIcon("document-open"));
	openAction->setShortcut(Qt::CTRL + Qt::Key_O);
	actionCollection()->addAction("open", openAction);
	connect(openAction, SIGNAL(triggered()), this, SLOT(openAction()));

	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

	setupGUI();
}
