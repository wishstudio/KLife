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

#include "AbstractAlgorithm.h"
#include "BigInteger.h"
#include "FileFormatManager.h"
#include "RLEFormat.h"
#include "TextStream.h"
#include "Utils.h"

REGISTER_FILEFORMAT(RLEFormat)

QString RLEFormat::formatName() const
{
	return "RLE Image";
}

QList<QString> RLEFormat::supportedFormats() const
{
	return QList<QString>() << "rle";
}

bool RLEFormat::readDevice(QIODevice *device, AbstractAlgorithm *algorithm)
{
	TextStream S(device);
	BigInteger x1, y1;
	int w = 0, h = 0, x = 0, y = 0;
	while (!S.atEnd())
	{
		QChar ch;
		S >> ch;
		if (ch == QChar('#'))
			S.skipLine();
		else if (ch == 'x' || ch == 'X')
		{
			S >> ch;
			if (ch != '=')
				return false;
			S >> w;
			S >> ch;
			if (ch != ',')
				return false;
			S >> ch;
			if (ch != 'y' && ch != 'Y')
				return false;
			S >> ch;
			if (ch != '=')
				return false;
			S >> h;
			S.skipLine();
			algorithm->setRect(x1, y1, w, h);
		}
		else if (ch == '$')
		{
			y++;
			x = 0;
		}
		else if (ch == '!')
		{
			if (x == w && y == h - 1)
				return true;
			else
				return false;
		}
		else if (ch == 'o' || ch == 'b')
		{
			if (x == w)
				return false;
			if (ch == 'o')
				algorithm->setGrid(x1 + x, y1 + y, 1);
			x++;
		}
		else
		{
			S.ungetChar(ch);
			int cnt;
			S >> cnt;
			if (cnt <= 0)
				return false;
			S >> ch;
			if (ch == QChar('o'))
			{
				algorithm->fillRect(x1 + x, y1 + y, cnt, 1, 1);
				x += cnt;
			}
			else if (ch == QChar('b'))
				x += cnt;
			else if (ch == QChar('$'))
			{
				y += cnt;
				x = 0;
			}
			else
				return false;
		}
	}
	return false;
}
