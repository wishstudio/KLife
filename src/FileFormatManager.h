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

#ifndef FILEFORMATMANAGER_H
#define FILEFORMATMANAGER_H

#include <QMap>

#include "Utils.h"

class AbstractFileFormat;
class FileFormatManager: public QObject
{
	Q_OBJECT

public:
	static FileFormatManager *self();
	static void registerFileFormat(AbstractFileFormat *format);
	static QString fileFilter();
	static bool readFile(QString fileName);

private:
	QString filters, allExtensions;
	QMap<QString, AbstractFileFormat *> formats;
};

#define REGISTER_FILEFORMAT(fileFormatClass) REGISTER_FACTORY_STATIC_CLASS(FileFormat, FileFormatManager, fileFormatClass)

#endif
