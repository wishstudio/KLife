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

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>

#include "MainWindow.h"

#include <QDebug>
#include <QString>

int main(int argc, char **argv)
{
	KAboutData aboutData(
		"KLife",
		0,
		ki18n("KLife"),
		"0.1",
		ki18n("A simulator and analyzer for Conway's Game of Life and other cellular automations."),
		KAboutData::License_GPL_V3,
		ki18n("(c) 2011 Xiangyan Sun"),
		ki18n(""),
		"",
		"wishstudio@gmail.com");
	aboutData.addAuthor(ki18n("Xiangyan Sun"), ki18n("Project founder, main developer"), "wishstudio@gmail.com", "", "");
	KCmdLineArgs::init(argc, argv, &aboutData);

	KApplication app;

	MainWindow *window = new MainWindow();
	window->show();

	return app.exec();
}
