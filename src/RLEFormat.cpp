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
#include "DataChannel.h"
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
	DataChannel *channel = NULL;
	TextStream S(device);
	BigInteger x1, y1;
	int w, h;
	while (!S.atEnd())
	{
		char ch;
		// TODO: Should be quint64!
		int cnt;
		if (S >> cnt)
		{
			if (cnt < 0)
				return false;
			S >> ch;
			if (ch == 'o')
				channel->send(1, cnt);
			else if (ch == 'b')
				channel->send(0, cnt);
			else if (ch == '$')
				channel->send(DATACHANNEL_EOLN, cnt);
		}
		else
		{
			S >> ch;
			if (ch == '#')
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
				algorithm->setReceiveRect(x1, y1, w, h);
				channel = DataChannel::transferTo(algorithm);
			}
			else if (ch == '$')
				channel->send(DATACHANNEL_EOLN, 1);
			else if (ch == '!')
			{
				channel->send(DATACHANNEL_EOF, 1);
				return true;
			}
			else if (ch == 'o')
				channel->send(1, 1);
			else if (ch == 'b')
				channel->send(0, 1);
		}
	}
	return false;
}
