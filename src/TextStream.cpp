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

#include "BigInteger.h"
#include "TextStream.h"

TextStream::TextStream(QIODevice *device)
	: QTextStream(device)
{
}

QChar TextStream::peek()
{
	if (back.size())
		return back.top();
	QChar ret = getChar();
	ungetChar(ret);
	return ret;
}

QChar TextStream::getChar()
{
	if (back.size())
		return back.pop();
	QChar ret;
	*dynamic_cast<QTextStream *>(this) >> ret;
	return ret;
}

void TextStream::ungetChar(QChar ch)
{
	back.push(ch);
}

void TextStream::skipWhiteSpace()
{
	if (back.size())
	{
		QChar ch = back.pop();
		while (ch.isSpace() && back.size())
			ch = back.pop();
		if (!ch.isSpace())
		{
			ungetChar(ch);
			return;
		}
	}
	QTextStream::skipWhiteSpace();
}

void TextStream::skipLine()
{
	while (getChar() != QChar('\n'));
}

TextStream& TextStream::operator >> (QChar &ch)
{
	skipWhiteSpace();
	ch = getChar();
	return *this;
}

TextStream& TextStream::operator >> (QString &str)
{
	skipWhiteSpace();
	str.clear();
	QChar ch;
	while (!(ch = getChar()).isSpace())
		str.append(ch);
	ungetChar(ch);
	return *this;
}

TextStream& TextStream::operator >> (BigInteger &num)
{
	skipWhiteSpace();
	QString str;
	QChar ch = getChar();
	if (ch == QTextStream::locale().negativeSign() || ch == QTextStream::locale().positiveSign())
	{
		ch = getChar();
		str.append(ch);
	}
	while (ch.isDigit())
	{
		str.append(ch.digitValue());
		ch = getChar();
	}
	ungetChar(ch);
	if (!str.size() || !str[0].isDigit())
		num = 0;
	else
		num = str;
	return *this;
}

TextStream& TextStream::operator >> (int &num)
{
	skipWhiteSpace();
	QChar ch = getChar();
	num = 0;
	bool neg = false;
	if (ch == QTextStream::locale().negativeSign() || ch == QTextStream::locale().positiveSign())
	{
		neg = true;
		ch = getChar();
	}
	while (ch.isDigit())
	{
		num = num * 10 + ch.digitValue();
		ch = getChar();
	}
	ungetChar(ch);
	if (neg)
		num = -num;
	return *this;
}
