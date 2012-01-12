/*
 *   Copyright (C) 2012 by Xiangyan Sun <wishstudio@gmail.com>
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

#include "RuleLife.h"

static inline QString ruleToString(int rule)
{
	QString ret;
	for (int i = 0; rule; rule >>= 1, i++)
		if (rule & 1)
			ret.append('0' + i);
	return ret;
}

static inline int stringToRule(QString str)
{
	int ret = 0;
	for (int i = 0; i < str.length(); i++)
		SET_BIT(ret, str.at(i).toAscii() - '0');
	return ret;
}

QString RuleLife::B() const
{
	return ruleToString(b);
}

QString RuleLife::S() const
{
	return ruleToString(s);
}

void RuleLife::setB(QString str)
{
	b = stringToRule(str);
}

void RuleLife::setS(QString str)
{
	s = stringToRule(str);
}
