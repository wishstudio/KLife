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

#include <KGlobal>

#include "AbstractAlgorithm.h"
#include "AlgorithmManager.h"

K_GLOBAL_STATIC(AlgorithmManager, globalAlgorithmManager)

AlgorithmManager *AlgorithmManager::self()
{
	return globalAlgorithmManager;
}

void AlgorithmManager::registerAlgorithm(AbstractAlgorithmFactory *algorithmFactory)
{
	self()->m_factory.append(algorithmFactory);
	// TODO
	self()->m_algorithm = algorithmFactory->createAlgorithm();
	connect(self()->m_algorithm, SIGNAL(rectChanged()), self(), SIGNAL(rectChanged()));
	connect(self()->m_algorithm, SIGNAL(gridChanged()), self(), SIGNAL(gridChanged()));
}

void AlgorithmManager::runStep()
{
	algorithm()->runStep();
}
