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

#ifndef RULELIFE_H
#define RULELIFE_H

#include "Rule.h"
#include "Utils.h"

class RuleLife: public Rule
{
public:
	RuleLife(QString b = "", QString s = "") { setBS(b, s); }

	virtual RuleType type() const { return Rule::Life; }
	virtual QString name() const { return "Life"; }
	virtual QString string() const { return QString("B%1/S%2").arg(B(), S()); }

	QString B() const;
	void setB(QString str);
	QString S() const;
	void setS(QString str);

	void setBS(QString b, QString s) { setB(b); setS(s); }

	// This function is time critical
	// So force it inlined
	inline int nextState(int original, int neighbourCount)
	{
		if (original)
			return TEST_BIT(s, neighbourCount) > 0;
		else
			return TEST_BIT(b, neighbourCount) > 0;
	}

private:
	int b, s;
};

#endif
