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

#include "AbstractAlgorithm.h"
#include "AlgorithmManager.h"
#include "Rule.h"

Q_GLOBAL_STATIC(AlgorithmManager, globalAlgorithmManager)

AlgorithmManager::AlgorithmManager()
{
	m_rule = NULL;
	m_algorithm = NULL;
}

AlgorithmManager *AlgorithmManager::self()
{
    return globalAlgorithmManager();
}

void AlgorithmManager::setRule(Rule *rule)
{
	if (self()->m_rule)
		delete self()->m_rule;
	self()->m_rule = rule;
	if (self()->m_algorithm && !self()->m_algorithm->acceptRule(rule))
	{
		delete self()->m_algorithm;
		self()->m_algorithm = NULL;
	}
	if (!self()->m_algorithm)
	{
		foreach (AbstractAlgorithmFactory *factory, self()->m_factory)
		{
			// TODO: Optimization
			AbstractAlgorithm *algorithm = factory->createAlgorithm();
			if (algorithm->acceptRule(rule))
			{
				connect(algorithm, SIGNAL(rectChanged()), self(), SIGNAL(rectChanged()));
				connect(algorithm, SIGNAL(gridChanged()), self(), SIGNAL(gridChanged()));
				self()->m_algorithm = algorithm;
				break;
			}
			delete algorithm;
		}
		if (!self()->m_algorithm)
			qFatal("No algorithm supports rule %s.", qPrintable(rule->string()));
		emit self()->algorithmChanged();
	}
	emit self()->ruleChanged();
}

void AlgorithmManager::registerAlgorithm(AbstractAlgorithmFactory *algorithmFactory)
{
	self()->m_factory.append(algorithmFactory);
}

void AlgorithmManager::runStep()
{
	algorithm()->runStep();
}
