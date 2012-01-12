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

#ifndef ALGORITHMMANAGER_H
#define ALGORITHMMANAGER_H

#include <QObject>

#include "Utils.h"

class AbstractAlgorithm;
class Rule;
class AlgorithmManager: public QObject
{
	Q_OBJECT

public:
	ABSTRACT_FACTORY(Algorithm)

	AlgorithmManager();

	static AlgorithmManager *self();
	static Rule *rule() { return self()->m_rule; }
	static void setRule(Rule *rule);
	static AbstractAlgorithm *algorithm() { return self()->m_algorithm; }
	static void registerAlgorithm(AbstractAlgorithmFactory *algorithmFactory);

public slots:
	void runStep();

signals:
	void ruleChanged();
	void rectChanged();
	void gridChanged();

private:
	AbstractAlgorithm *m_algorithm;
	Rule *m_rule;

	QList<AbstractAlgorithmFactory *> m_factory;
};

#define REGISTER_ALGORITHM(algorithmClass) REGISTER_FACTORY_CLASS(Algorithm, AlgorithmManager, algorithmClass)

#endif
