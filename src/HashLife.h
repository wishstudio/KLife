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

#ifndef HASHLIFE_H
#define HASHLIFE_H

#include <QVector>

#include "AbstractAlgorithm.h"
#include "BigInteger.h"
#include "MemoryManager.h"
#include "Rule.h"

struct Block;
struct Node;
template <typename T> class HashTable;
class QMutex;
class HashLife: public AbstractAlgorithm, private MemoryManager
{
	Q_OBJECT

public:
	HashLife();
	virtual ~HashLife();

	virtual QString name() { return "HashLife"; }
	virtual bool acceptRule(Rule *rule) { return rule->type() == Rule::Life; }

	virtual void setReceiveRect(const BigInteger &x, const BigInteger &y, quint64 w, quint64 h) {}
	virtual void receive(DataChannel *channel) {}
	virtual int grid(const BigInteger &x, const BigInteger &y) { Q_UNUSED(x); Q_UNUSED(y); return 0; } // TODO: Who use this now?
	virtual void setGrid(const BigInteger &x, const BigInteger &y, int state);
	virtual void clearGrid() {}
	virtual BigInteger generation() const;
	virtual BigInteger population() const;
	virtual void paint(CanvasPainter *painter, const BigInteger &x, const BigInteger &y, int w, int h, size_t scale);
	virtual void runStep() {}

private:
	void rectChange(const BigInteger &x, const BigInteger &y, const BigInteger &w, const BigInteger &h) {}
	Node *emptyNode(size_t depth);
	void expand();
	Node *runNode(Node *node, size_t depth);

	QMutex *m_readLock, *m_writeLock;
	volatile bool m_running;

	HashTable<Block> *m_blockHash;
	HashTable<Node> *m_nodeHash;
	Node *m_root;
	size_t m_depth;
	QVector<Node *> m_emptyNode;

	BigInteger m_x, m_y;
	BigInteger m_generation;
	size_t m_increment;

	friend class Node;
};

#endif
