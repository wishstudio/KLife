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
	: m_atEnd(false), m_success(true), m_size(0), m_pos(0), m_device(device)
{
	nextChar();
}

bool TextStream::atEnd()
{
	return m_pos > m_size && m_atEnd;
}

inline bool TextStream::isSpace(char ch)
{
	return ch == ' ' || ch == '\t' || ch == '\n';
}

inline void TextStream::nextChar()
{
	if (m_pos == m_size)
	{
		if (m_atEnd)
		{
			m_char = EOF;
			return;
		}
		m_size = m_device->read(m_buf, TEXTSTREAM_BUFFER_SIZE);
		m_atEnd = m_device->atEnd();
		m_pos = 0;
	}
	m_char = m_buf[m_pos++];
}

void TextStream::skipWhiteSpace()
{
	while (isSpace(m_char))
		nextChar();
}

void TextStream::skipLine()
{
	while (m_char != '\n')
		nextChar();
}

TextStream& TextStream::operator >> (char &ch)
{
	skipWhiteSpace();
	ch = m_char;
	nextChar();
	m_success = true;
	return *this;
}

TextStream& TextStream::operator >> (int &num)
{
	skipWhiteSpace();
	num = 0;
	bool neg = false;
	if (m_char == '+' || m_char == '-')
	{
		neg = m_char == '-';
		nextChar();
	}
	if (m_char < '0' || m_char > '9')
	{
		m_success = false;
		return *this;
	}
	while (m_char >= '0' && m_char <= '9')
	{
		num = num * 10 + m_char - '0';
		nextChar();
	}
	if (neg)
		num = -num;
	m_success = true;
	return *this;
}
