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

#include <QFile>
#include <QFileInfo>

#include "AbstractAlgorithm.h"
#include "AbstractFileFormat.h"
#include "AlgorithmManager.h"
#include "FileFormatManager.h"

Q_GLOBAL_STATIC(FileFormatManager, globalFileFormatManager)

FileFormatManager *FileFormatManager::self()
{
    return globalFileFormatManager();
}

void FileFormatManager::registerFileFormat(AbstractFileFormat *format)
{
	QList<QString> f = format->supportedFormats();
    globalFileFormatManager()->filters.append(";;");
    globalFileFormatManager()->filters.append(format->formatName() + " (");
	foreach (QString fm, f)
	{
        globalFileFormatManager()->formats[fm] = format;
        globalFileFormatManager()->filters.append(" *." + fm);
        globalFileFormatManager()->allExtensions.append(" *." + fm);
	}
    globalFileFormatManager()->filters.append(")");
}

QString FileFormatManager::fileFilter()
{
    return tr("All supported formats") + "(" + globalFileFormatManager()->allExtensions + ")" + globalFileFormatManager()->filters;
}

bool FileFormatManager::readFile(QString fileName)
{
	QFileInfo info(fileName);
	QString suffix = info.suffix();
	AbstractFileFormat *format;
    if (globalFileFormatManager()->formats.count(suffix))
        format = globalFileFormatManager()->formats[suffix];
	else
		return false;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	AlgorithmManager::algorithm()->clearGrid();
	bool ret = format->readDevice(&file, AlgorithmManager::algorithm());
	file.close();
	return ret;
}
