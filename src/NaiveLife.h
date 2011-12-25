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

#ifndef NAIVELIFE_H
#define NAIVELIFE_H

#include <QVector>

#include "AbstractAlgorithm.h"
#include "BigInteger.h"
#include "MemoryManager.h"

struct Block;
struct Node;
class QMutex;
class CanvasPainter;
class NaiveLife: public AbstractAlgorithm, private MemoryManager
{
	Q_OBJECT

public:
	NaiveLife();
	virtual ~NaiveLife();

	virtual QString name() { return "NaiveLife"; }
	virtual void setReceiveRect(const BigInteger &x, const BigInteger &y, quint64 w, quint64 h);
	virtual void receive(DataChannel *channel);
	virtual int grid(const BigInteger &x, const BigInteger &y);
	virtual void setGrid(const BigInteger &x, const BigInteger &y, int state);
	virtual void clearGrid();
	virtual BigInteger generation() const;
	virtual BigInteger population() const;
	virtual void rectChange(const BigInteger &, const BigInteger &, const BigInteger &, const BigInteger &);
	virtual void paint(CanvasPainter *canvasPainter, const BigInteger &x, const BigInteger &y, int w, int h, size_t scale);
	virtual void runStep();

private:
	void receiveGrid(DataChannel *channel, Node *&node_ul, Node *&node_ur, Node *&node_dl, Node *&node_dr, bool ok_ur, bool ok_dl, bool ok_dr, size_t depth, size_t endDepth, const BigInteger &x, const BigInteger &y);
	inline void receiveGrid(DataChannel *channel, Node *&node_ul, Node *&node_ur, Node *&node_dl, Node *&node_dr, size_t depth, quint64 x, quint64 y, int &state, quint64 &cnt);
	void receiveGrid(DataChannel *channel, Node *&node, size_t depth, quint64 x, quint64 y, int &state, quint64 &cnt);
	void expand();
	inline void computeBlockActiveFlag(Block *block);
	inline void computeNodeInfo(Node *node, size_t depth);
	inline Block *newBlock();
	inline Node *newNode(size_t depth);
	Node *&emptyNode(size_t depth);
	void deleteNode(Node *node, size_t depth);
	inline void drawNode(CanvasPainter *painter, Node *node_ul, Node *node_ur, Node *node_dl, Node *node_dr, int x1, int y1, int x2, int y2, size_t depth, size_t scale, int offset_x, int offset_y);
	void drawNode(CanvasPainter *painter, Node *node, int x1, int y1, int x2, int y2, size_t depth, size_t scale, int offset_x, int offset_y);
	void runNode(Node *&p, Node *node, Node *up, Node *down, Node *left, Node *right, Node *upleft, Node *upright, Node *downleft, Node *downright, size_t depth);
	virtual void run();

	volatile bool m_running;
	QVector<Node *> m_emptyNode;
	size_t m_depth;
	Node *m_root;
	QMutex *m_readLock, *m_writeLock;

	BigInteger m_x, m_y;
	BigInteger m_generation;

	// Data Channel related
	BigInteger mc_x, mc_y;
	quint64 mc_w, mc_h;
};

#endif
