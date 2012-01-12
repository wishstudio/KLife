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

#include <QMutex>

#include "AlgorithmManager.h"
#include "CanvasPainter.h"
#include "NaiveLife.h"
#include "Utils.h"

REGISTER_ALGORITHM(NaiveLife)

// node flags
#define CHANGED       0  // This node has changed since last iteration
#define KEEP          1  // Prevent this node from being deleted when deleting previous iteration
#define UP_CHANGED    2  // The top row has changed since last iteration
#define DOWN_CHANGED  3  // The bottom row has changed since last iteration
#define LEFT_CHANGED  4  // The left column has changed since last iteration
#define RIGHT_CHANGED 5  // The right column has changed since last iteration

struct Block
{
public:
	static const size_t DEPTH = 3;
	static const size_t SIZE = 1 << DEPTH;

	int flag;
	quint64 population;

	inline void clear()
	{
		memset(data, 0, sizeof data);
	}

	inline void set(int x, int y, int state)
	{
		data[y][x] = state;
	}

	inline int get(int x, int y) const
	{
		return data[y][x];
	}

private:
	uchar data[Block::SIZE][Block::SIZE];
};

// 00 01
// 10 11
#define ul child[0]
#define ur child[1]
#define dl child[2]
#define dr child[3]
struct Node
{
	Node *child[4];
	int flag;
	quint64 population;
};

NaiveLife::NaiveLife()
	: m_running(false), m_readLock(new QMutex()), m_writeLock(new QMutex()), m_x(0), m_y(0), m_generation(0)
{
	setAcceptInfinity(false);
	m_emptyNode.resize(Block::DEPTH + 1);
	for (size_t i = 0; i < Block::DEPTH; i++)
		m_emptyNode[i] = NULL;
	m_emptyNode[Block::DEPTH] = reinterpret_cast<Node *>(newBlock());
	reinterpret_cast<Block *>(m_emptyNode[Block::DEPTH])->flag = 0;
	// run() requires m_depth >= Block::DEPTH + 2
	m_depth = Block::DEPTH + 2;
	m_root = newNode(m_depth);
}

NaiveLife::~NaiveLife()
{
	delete m_readLock;
	delete m_writeLock;
	deleteNode(m_root, m_depth);
	for (int i = Block::DEPTH; i < m_emptyNode.size(); i++)
		deleteNode(m_emptyNode[i], i);
}

void NaiveLife::setReceiveRect(const BigInteger &x, const BigInteger &y, quint64 w, quint64 h)
{
	mc_x = x;
	mc_y = y;
	mc_w = w;
	mc_h = h;
}

void NaiveLife::receive(DataChannel *channel)
{
	m_writeLock->lock();
	m_readLock->lock();
	// out of range
	// TODO: optimization
	BigInteger x1 = mc_x - m_x, y1 = mc_y - m_y, x2 = x1 + BigInteger(mc_w - 1), y2 = y1 + BigInteger(mc_h - 1);
	while (x1.sgn() < 0 || x2.sgn() < 0 || x2.bitCount() > m_depth || y1.sgn() < 0 || y2.sgn() < 0 || y2.bitCount() > m_depth)
	{
		expand();
		x1 = mc_x - m_x;
		y1 = mc_y - m_y;
		x2 = x1 + BigInteger(mc_w - 1);
		y2 = y1 + BigInteger(mc_h - 1);
	}

	size_t endDepth = qMax<size_t>(qMax(bitlen(mc_w), bitlen(mc_h)), Block::DEPTH);
	Node *e = emptyNode(m_depth);
	receiveGrid(channel, m_root, e, e, e, false, false, false, m_depth, endDepth, x1, y1);
	m_readLock->unlock();
	m_writeLock->unlock();
	emit gridChanged();
}

void NaiveLife::receiveGrid(DataChannel *channel, Node *&node_ul, Node *&node_ur, Node *&node_dl, Node *&node_dr, bool ok_ur, bool ok_dl, bool ok_dr, size_t depth, size_t endDepth, const BigInteger &x, const BigInteger &y)
{
	if (depth == endDepth)
	{
		quint64 sx = x.lowbits<quint64>(depth), sy = y.lowbits<quint64>(depth);
		quint64 x1 = sx, y1 = sy;
		int state;
		quint64 cnt;
		channel->receive(&state, &cnt);
		while (state == DATACHANNEL_EOLN)
		{
			y1 += cnt;
			channel->receive(&state, &cnt);
		}
		if (state == DATACHANNEL_EOF)
			return;
		forever
		{
			receiveGrid(channel, node_ul, node_ur, node_dl, node_dr, depth, x1, y1, state, cnt);
			while (state == DATACHANNEL_EOLN)
			{
				y1 += cnt;
				x1 = sx;
				channel->receive(&state, &cnt);
			}
			if (state == DATACHANNEL_EOF)
				return;
		}
	}
	else
	{
		Node *e = emptyNode(depth);
		if (node_ul == e)
			node_ul = newNode(depth);
		if (node_ur == e && ok_ur)
			node_ur = newNode(depth);
		if (node_dl == e && ok_dl)
			node_dl = newNode(depth);
		if (node_dr == e && ok_dr)
			node_dr = newNode(depth);
		switch ((y.bit(depth - 1) << 1) | x.bit(depth - 1))
		{
		case 0:
			//  ul ur  0  0
			//  dl dr  0  0
			//   0  0  0  0
			//   0  0  0  0
			receiveGrid(channel, node_ul->ul, node_ul->ur, node_ul->dl, node_ul->dr, true, true, true, depth - 1, endDepth, x, y);
			break;

		case 1:
			//   0 ul ur  0
			//   0 dl dr  0
			//   0  0  0  0
			//   0  0  0  0
			receiveGrid(channel, node_ul->ur, node_ur->ul, node_ul->dr, node_ur->dl, ok_ur, true, ok_ur, depth - 1, endDepth, x, y);
			break;

		case 2:
			//   0  0  0  0
			//  ul ur  0  0
			//  dl dr  0  0
			//   0  0  0  0
			receiveGrid(channel, node_ul->dl, node_ul->dr, node_dl->ul, node_dl->ur, true, ok_dl, ok_dl, depth - 1, endDepth, x, y);
			break;

		case 3:
			//   0  0  0  0
			//   0 ul ur  0
			//   0 dl dr  0
			//   0  0  0  0
			receiveGrid(channel, node_ul->dr, node_ur->dl, node_dl->ur, node_dr->ul, ok_ur, ok_dl, ok_dr, depth - 1, endDepth, x, y);
			break;
		}
		computeNodeInfo(node_ul, depth);
		computeNodeInfo(node_ur, depth);
		computeNodeInfo(node_dl, depth);
		computeNodeInfo(node_dr, depth);
	}
}

inline void NaiveLife::receiveGrid(DataChannel *channel, Node *&node_ul, Node *&node_ur, Node *&node_dl, Node *&node_dr, size_t depth, quint64 x, quint64 y, int &state, quint64 &cnt)
{
	quint64 len = Q_UINT64_C(1) << depth;
	if (y < len)
	{
		if (x < len)
		{
			receiveGrid(channel, node_ul, depth, x, y, state, cnt);
			if (state >= 0)
				receiveGrid(channel, node_ur, depth, 0, y, state, cnt);
		}
		else
			receiveGrid(channel, node_ur, depth, x - len, y, state, cnt);
	}
	else
	{
		if (x < len)
		{
			receiveGrid(channel, node_dl, depth, x, y - len, state, cnt);
			if (state >= 0)
				receiveGrid(channel, node_dr, depth, 0, y - len, state, cnt);
		}
		else
			receiveGrid(channel, node_dr, depth, x - len, y - len, state, cnt);
	}
}

void NaiveLife::receiveGrid(DataChannel *channel, Node *&node, size_t depth, quint64 x, quint64 y, int &state, quint64 &cnt)
{
	if (depth == Block::DEPTH)
	{
		do
		{
			quint64 d = qMin<quint64>(Block::SIZE - x, cnt);
			if (state)
			{
				if (node == emptyNode(depth))
					node = reinterpret_cast<Node *>(newBlock());
				Block *block = reinterpret_cast<Block *>(node);
				for (unsigned int i = x; i < x + d; i++)
					if (block->get(i, y) != state)
					{
						if (!block->get(i, y))
							block->population++;
						else
							block->population--;
						block->set(i, y, state);
						if (y == 0)
							SET_BIT(block->flag, UP_CHANGED);
						if (y == Block::SIZE - 1)
							SET_BIT(block->flag, DOWN_CHANGED);
						if (i == 0)
							SET_BIT(block->flag, LEFT_CHANGED);
						if (i == Block::SIZE - 1)
							SET_BIT(block->flag, RIGHT_CHANGED);
					}
				SET_BIT(block->flag, CHANGED);
			}
			cnt -= d;
			if (!cnt)
				channel->receive(&state, &cnt);
			x += d;
		}
		while (x < Block::SIZE && state >= 0);
	}
	else
	{
		quint64 len = Q_UINT64_C(1) << depth;
		while (x < len && state == 0)
		{
			quint64 d = qMin(len - x, cnt);
			cnt -= d;
			if (!cnt)
				channel->receive(&state, &cnt);
			x += d;
		}
		if (x < len && state > 0)
		{
			if (node == emptyNode(depth))
				node = newNode(depth);
			receiveGrid(channel, node->ul, node->ur, node->dl, node->dr, depth - 1, x, y, state, cnt);
			computeNodeInfo(node, depth);
		}
	}
}

int NaiveLife::grid(const BigInteger &x, const BigInteger &y)
{
	m_readLock->lock();
	BigInteger my_x = x - m_x, my_y = y - m_y;
	// out of range
	if (my_x.sgn() < 0 || my_x.bitCount() > m_depth || my_y.sgn() < 0 || my_y.bitCount() > m_depth)
	{
		m_readLock->unlock();
		return 0;
	}
	Node *p = m_root;
	size_t depth = m_depth;
	while (depth > Block::DEPTH)
	{
		if (p == emptyNode(depth))
			break;
		p = p->child[(my_y.bit(depth - 1) << 1) | my_x.bit(depth - 1)];
		depth--;
	}
	int ret;
	if (p == NULL)
		ret = 0;
	else
		ret = reinterpret_cast<Block *>(p)->get(my_x.lowbits<int>(Block::DEPTH), my_y.lowbits<int>(Block::DEPTH));
	m_readLock->unlock();
	return ret;
}

void NaiveLife::setGrid(const BigInteger &x, const BigInteger &y, int state)
{
	if (m_running)
		return;
	m_writeLock->lock();
	BigInteger my_x = x - m_x, my_y = y - m_y;
	// out of range
	// TODO: optimization
	while (my_x.sgn() < 0 || my_x.bitCount() > m_depth || my_y.sgn() < 0 || my_y.bitCount() > m_depth)
	{
		expand();
		my_x = x - m_x;
		my_y = y - m_y;
	}
	Node *p = m_root, *stack[m_depth + 1];
	size_t depth = m_depth;
	while (depth > Block::DEPTH)
	{
		stack[depth] = p;
		int cid = (my_y.bit(depth - 1) << 1) | my_x.bit(depth - 1);
		depth--;
		if (p->child[cid] == emptyNode(depth))
		{
			if (depth == Block::DEPTH)
				p->child[cid] = reinterpret_cast<Node *>(newBlock());
			else
				p->child[cid] = newNode(depth);
		}
		p = p->child[cid];
	}
	Block *block = reinterpret_cast<Block *>(p);
	int sx = my_x.lowbits<int>(Block::DEPTH), sy = my_y.lowbits<int>(Block::DEPTH);
	if (block->get(sx, sy) && !state)
		block->population--;
	else if (!block->get(sx, sy) && state)
		block->population++;
	block->set(sx, sy, state);
	block->flag |= BIT(CHANGED);
	if (sy == 0)
		block->flag |= BIT(UP_CHANGED);
	if (sy == Block::SIZE - 1)
		block->flag |= BIT(DOWN_CHANGED);
	if (sx == 0)
		block->flag |= BIT(LEFT_CHANGED);
	if (sx == Block::SIZE - 1)
		block->flag |= BIT(RIGHT_CHANGED);
	while (++depth <= m_depth)
		computeNodeInfo(stack[depth], depth);
	m_writeLock->unlock();
	emit gridChanged();
}

void NaiveLife::clearGrid()
{
	m_writeLock->lock();
	m_readLock->lock();
	deleteNode(m_root->ul, m_depth - 1);
	deleteNode(m_root->ur, m_depth - 1);
	deleteNode(m_root->dl, m_depth - 1);
	deleteNode(m_root->dr, m_depth - 1);
	m_depth = Block::DEPTH + 2;
	m_root->ul = m_root->ur = m_root->dl = m_root->dr = emptyNode(m_depth - 1);
	m_root->flag = 0;
	m_x = 0;
	m_y = 0;
	m_generation = 0;
	m_readLock->unlock();
	m_writeLock->unlock();
	emit gridChanged();
}

BigInteger NaiveLife::generation() const
{
	return m_generation;
}

BigInteger NaiveLife::population() const
{
	return m_root->population;
}

void NaiveLife::rectChange(const BigInteger &, const BigInteger &, const BigInteger &, const BigInteger &)
{
	emit rectChanged();
	clearGrid();
}

void NaiveLife::expand()
{
	{
		Node *tmp = newNode(m_depth);
		tmp->dr = m_root->ul;
		m_root->ul = tmp;
	}
	{
		Node *tmp = newNode(m_depth);
		tmp->dl = m_root->ur;
		m_root->ur = tmp;
	}
	{
		Node *tmp = newNode(m_depth);
		tmp->ur = m_root->dl;
		m_root->dl = tmp;
	}
	{
		Node *tmp = newNode(m_depth);
		tmp->ul = m_root->dr;
		m_root->dr = tmp;
	}
	computeNodeInfo(m_root->ul, m_depth);
	computeNodeInfo(m_root->ur, m_depth);
	computeNodeInfo(m_root->dl, m_depth);
	computeNodeInfo(m_root->dr, m_depth);
	computeNodeInfo(m_root, m_depth + 1);
	BigInteger offset = BigInteger::exp2(m_depth - 1);
	m_x -= offset;
	m_y -= offset;
	m_depth++;
}

inline void NaiveLife::computeNodeInfo(Node *node, size_t depth)
{
	int ul_flag, ur_flag, dl_flag, dr_flag;
	if (depth == Block::DEPTH + 1)
	{
		node->population = reinterpret_cast<Block *>(node->ul)->population
				+ reinterpret_cast<Block *>(node->ur)->population
				+ reinterpret_cast<Block *>(node->dl)->population
				+ reinterpret_cast<Block *>(node->dr)->population;
		ul_flag = reinterpret_cast<Block *>(node->ul)->flag;
		ur_flag = reinterpret_cast<Block *>(node->ur)->flag;
		dl_flag = reinterpret_cast<Block *>(node->dl)->flag;
		dr_flag = reinterpret_cast<Block *>(node->dr)->flag;
	}
	else
	{
		node->population = node->ul->population + node->ur->population + node->dl->population + node->dr->population;
		ul_flag = node->ul->flag;
		ur_flag = node->ur->flag;
		dl_flag = node->dl->flag;
		dr_flag = node->dr->flag;
	}
	node->flag = TEST_BIT(ul_flag, CHANGED) | TEST_BIT(ur_flag, CHANGED) | TEST_BIT(dl_flag, CHANGED) | TEST_BIT(dr_flag, CHANGED);

	node->flag |= TEST_BIT(ul_flag, UP_CHANGED);
	node->flag |= TEST_BIT(ul_flag, LEFT_CHANGED);

	node->flag |= TEST_BIT(ur_flag, UP_CHANGED);
	node->flag |= TEST_BIT(ur_flag, RIGHT_CHANGED);

	node->flag |= TEST_BIT(dl_flag, DOWN_CHANGED);
	node->flag |= TEST_BIT(dl_flag, LEFT_CHANGED);

	node->flag |= TEST_BIT(dr_flag, DOWN_CHANGED);
	node->flag |= TEST_BIT(dr_flag, RIGHT_CHANGED);
}

inline Block *NaiveLife::newBlock()
{
	Block *ret = newObject<Block>();
	ret->clear();
	ret->flag = 0;
	ret->population = 0;
	return ret;
}

inline Node *NaiveLife::newNode(size_t depth)
{
	Node *ret = newObject<Node>();
	ret->ul = ret->ur = ret->dl = ret->dr = emptyNode(depth - 1);
	ret->flag = 0;
	ret->population = 0;
	return ret;
}

Node *&NaiveLife::emptyNode(size_t depth)
{
	if (static_cast<size_t>(m_emptyNode.size()) > depth)
		return m_emptyNode[depth];
	else
	{
		m_emptyNode.push_back(newNode(depth));
		return m_emptyNode.last();
	}
}

void NaiveLife::deleteNode(Node *node, size_t depth)
{
	if (node == emptyNode(depth))
		return;
	if (depth == Block::DEPTH)
	{
		Block *bnode = reinterpret_cast<Block *>(node);
		if (TEST_BIT(bnode->flag, KEEP))
			CLR_BIT(bnode->flag, KEEP);
		else
			deleteObject(bnode);
	}
	else if (TEST_BIT(node->flag, KEEP))
		CLR_BIT(node->flag, KEEP);
	else
	{
		deleteNode(node->ul, depth - 1);
		deleteNode(node->ur, depth - 1);
		deleteNode(node->dl, depth - 1);
		deleteNode(node->dr, depth - 1);
		deleteObject(node);
	}
}

// Magic in paint() to get rid of BigInteger manipulation:
// We walk down and find 4 nodes cover the drawing area
// We have to guarantee coordinate (x, y) is in node ul during the process.
// Firstly set ul to root, and other nodes to the magic empty node, like
//  ul  0
//   0  0
// Then walk down the tree, we continually divide ul into 4 smaller parts
// Because 2^(level-1) is larger than w and h so the needed childs of 4 nodes
// are unique, when depth > endDepth
// After walkdown(), we can guarantee all the coordinates fit in ints.
void NaiveLife::paint(CanvasPainter *painter, const BigInteger &x, const BigInteger &y, int w, int h, size_t scale)
{
	m_readLock->lock();

	// Fill black background
	painter->fillBlack();

	// Fit into range
	BigInteger x1 = x - (m_x >> scale), y1 = y - (m_y >> scale), x2 = x1 + (w - 1), y2 = y1 + (h - 1);

	if (x2.sgn() < 0 || (x1.sgn() > 0 && x1.bitCount() > m_depth - scale) || y2.sgn() < 0 || (y1.sgn() > 0 && y1.bitCount() > m_depth - scale))
	{
		m_readLock->unlock();
		return;
	}

	// The coordinate of the upper left grid in canvas painter
	int offset_x = 0, offset_y = 0;
	if (x1.sgn() < 0)
	{
		w += x1;
		offset_x = -x1;
		x1 = 0;
	}
	if (y1.sgn() < 0)
	{
		h += y1;
		offset_y = -y1;
		y1 = 0;
	}

	if (m_depth < scale)
		drawNode(painter, m_root, 0, 0, 0, 0, 0, scale, offset_x, offset_y);
	else
	{
		BigInteger len = BigInteger::exp2(m_depth - scale);
		if (x2.bitCount() > m_depth - scale)
			w = len - x1;
		if (y2.bitCount() > m_depth - scale)
			h = len - y1;

		// Step 1
		size_t depth = m_depth - scale, endDepth = qMax<size_t>(qMax(bitlen(w), bitlen(h)), Block::DEPTH);
		Node *node_ul = m_root, *node_ur = emptyNode(m_depth), *node_dl = emptyNode(m_depth), *node_dr = emptyNode(m_depth);
		while (depth > endDepth)
		{
			switch ((y1.bit(depth - 1) << 1) | x1.bit(depth - 1))
			{
			case 0:
				//  ul ur  0  0
				//  dl dr  0  0
				//   0  0  0  0
				//   0  0  0  0
				node_ur = node_ul->ur;
				node_dl = node_ul->dl;
				node_dr = node_ul->dr;
				node_ul = node_ul->ul;
				break;

			case 1:
				//   0 ul ur  0
				//   0 dl dr  0
				//   0  0  0  0
				//   0  0  0  0
				node_dl = node_ul->dr;
				node_ul = node_ul->ur;
				node_dr = node_ur->dl;
				node_ur = node_ur->ul;
				break;

			case 2:
				//   0  0  0  0
				//  ul ur  0  0
				//  dl dr  0  0
				//   0  0  0  0
				node_ur = node_ul->dr;
				node_ul = node_ul->dl;
				node_dr = node_dl->ur;
				node_dl = node_dl->ul;
				break;

			case 3:
				//   0  0  0  0
				//   0 ul ur  0
				//   0 dl dr  0
				//   0  0  0  0
				node_ul = node_ul->dr;
				node_ur = node_ur->dl;
				node_dl = node_dl->ur;
				node_dr = node_dr->ul;
				break;
			}
			depth--;
		}

		// Step 2
		int sx1 = x1.lowbits<int>(depth), sy1 = y1.lowbits<int>(depth);
		drawNode(painter, node_ul, node_ur, node_dl, node_dr, sx1, sy1, sx1 + w - 1, sy1 + h - 1, depth, scale, offset_x - sx1, offset_y - sy1);
	}
	m_readLock->unlock();
}

inline void NaiveLife::drawNode(CanvasPainter *painter, Node *node_ul, Node *node_ur, Node *node_dl, Node *node_dr, int x1, int y1, int x2, int y2, size_t depth, size_t scale, int offset_x, int offset_y)
{
	int len = 1 << depth;
	if (x1 < len && y1 < len)
		drawNode(painter, node_ul, x1, y1, qMin(x2, len - 1), qMin(y2, len - 1), depth, scale, offset_x, offset_y);
	if (x2 >= len && y1 < len)
		drawNode(painter, node_ur, qMax(x1 - len, 0), y1, x2 - len, qMin(y2, len - 1), depth, scale, offset_x + len, offset_y);
	if (x1 < len && y2 >= len)
		drawNode(painter, node_dl, x1, qMax(y1 - len, 0), qMin(x2, len - 1), y2 - len, depth, scale, offset_x, offset_y + len);
	if (x2 >= len && y2 >= len)
		drawNode(painter, node_dr, qMax(x1 - len, 0), qMax(y1 - len, 0), x2 - len, y2 - len, depth, scale, offset_x + len, offset_y + len);
}

void NaiveLife::drawNode(CanvasPainter *painter, Node *node, int x1, int y1, int x2, int y2, size_t depth, size_t scale, int offset_x, int offset_y)
{
	if (depth + scale == Block::DEPTH)
	{
		if (depth == 0)
			painter->drawGrid(offset_x + x1, offset_y + y1, reinterpret_cast<Block *>(node)->population > 0);
		else
		{
			for (int x = x1; x <= x2; x++)
				for (int y = y1; y <= y2; y++)
				{
					int state;
					if (scale)
					{
						state = 0;
						for (int i = x * (1 << scale); i < (x + 1) * (1 << scale); i++)
							for (int j = y * (1 << scale); j < (y + 1) * (1 << scale); j++)
								state |= reinterpret_cast<Block *>(node)->get(i, j) > 0;
					}
					else
						state = reinterpret_cast<Block *>(node)->get(x, y);
					painter->drawGrid(offset_x + x, offset_y + y, state);
				}
		}
	}
	else if (depth == 0)
		painter->drawGrid(offset_x + x1, offset_y + y1, node->population > 0);
	else
		drawNode(painter, node->ul, node->ur, node->dl, node->dr, x1, y1, x2, y2, depth - 1, scale, offset_x, offset_y);
}

void NaiveLife::runStep()
{
	start();
}

void NaiveLife::runNode(Node *&p, Node *node, Node *up, Node *down, Node *left, Node *right, Node *upleft, Node *upright, Node *downleft, Node *downright, size_t depth)
{
	if (depth == Block::DEPTH)
	{
		Block *bnode = reinterpret_cast<Block *>(node);
		Block *bup = reinterpret_cast<Block *>(up);
		Block *bdown = reinterpret_cast<Block *>(down);
		Block *bleft = reinterpret_cast<Block *>(left);
		Block *bright = reinterpret_cast<Block *>(right);
		Block *bupleft = reinterpret_cast<Block *>(upleft);
		Block *bupright = reinterpret_cast<Block *>(upright);
		Block *bdownleft = reinterpret_cast<Block *>(downleft);
		Block *bdownright = reinterpret_cast<Block *>(downright);
		if (TEST_BIT(reinterpret_cast<Block *>(node)->flag, CHANGED)
				|| TEST_BIT(bup->flag, DOWN_CHANGED)
				|| TEST_BIT(bdown->flag, UP_CHANGED)
				|| TEST_BIT(bleft->flag, RIGHT_CHANGED)
				|| TEST_BIT(bright->flag, LEFT_CHANGED)
				|| (TEST_BIT(bupleft->flag, DOWN_CHANGED) && TEST_BIT(bupleft->flag, RIGHT_CHANGED))
				|| (TEST_BIT(bupright->flag, DOWN_CHANGED) && TEST_BIT(bupright->flag, LEFT_CHANGED))
				|| (TEST_BIT(bdownleft->flag, UP_CHANGED) && TEST_BIT(bdownleft->flag, RIGHT_CHANGED))
				|| (TEST_BIT(bdownright->flag, UP_CHANGED) && TEST_BIT(bdownright->flag, LEFT_CHANGED)))
		{
			Block *block = newBlock();
			p = reinterpret_cast<Node *>(block);
			const int dx[8] = {-1,  0,  1, 1, 1, 0, -1, -1};
			const int dy[8] = {-1, -1, -1, 0, 1, 1,  1,  0};
			for (size_t x = 0; x < Block::SIZE; x++)
				for (size_t y = 0; y < Block::SIZE; y++)
				{
					int n = 0;
					for (int d = 0; d < 8; d++)
					{
						int sx = static_cast<int>(x) + dx[d], sy = static_cast<int>(y) + dy[d];
						if (sy < 0)
						{
							if (sx < 0)
								n += bupleft->get(Block::SIZE - 1, Block::SIZE - 1);
							else if (sx >= static_cast<int>(Block::SIZE))
								n += bupright->get(0, Block::SIZE - 1);
							else
								n += bup->get(sx, Block::SIZE - 1);
						}
						else if (sy >= static_cast<int>(Block::SIZE))
						{
							if (sx < 0)
								n += bdownleft->get(Block::SIZE - 1, 0);
							else if (sx >= static_cast<int>(Block::SIZE))
								n += bdownright->get(0, 0);
							else
								n += bdown->get(sx, 0);
						}
						else if (sx < 0)
							n += bleft->get(Block::SIZE - 1, sy);
						else if (sx >= static_cast<int>(Block::SIZE))
							n += bright->get(0, sy);
						else
							n += bnode->get(sx, sy);
					}
					block->set(x, y, ((n == 3) || (bnode->get(x, y) && n == 2)));
					block->population += block->get(x, y) > 0;
					if (block->get(x, y) != bnode->get(x, y))
					{
						if (y == 0)
							SET_BIT(block->flag, UP_CHANGED);
						if (y == Block::SIZE - 1)
							SET_BIT(block->flag, DOWN_CHANGED);
						if (x == 0)
							SET_BIT(block->flag, LEFT_CHANGED);
						if (x == Block::SIZE - 1)
							SET_BIT(block->flag, RIGHT_CHANGED);
						SET_BIT(block->flag, CHANGED);
					}
				}
		}
		else if (node != emptyNode(depth))
		{
			SET_BIT(bnode->flag, KEEP);
			p = node;
		}
	}
	else
	{
		if (TEST_BIT(node->flag, CHANGED)
				|| TEST_BIT(up->flag, DOWN_CHANGED)
				|| TEST_BIT(down->flag, UP_CHANGED)
				|| TEST_BIT(left->flag, RIGHT_CHANGED)
				|| TEST_BIT(right->flag, LEFT_CHANGED)
				|| (TEST_BIT(upleft->flag, DOWN_CHANGED) && TEST_BIT(upleft->flag, RIGHT_CHANGED))
				|| (TEST_BIT(upright->flag, DOWN_CHANGED) && TEST_BIT(upright->flag, LEFT_CHANGED))
				|| (TEST_BIT(downleft->flag, UP_CHANGED) && TEST_BIT(downleft->flag, RIGHT_CHANGED))
				|| (TEST_BIT(downright->flag, UP_CHANGED) && TEST_BIT(downright->flag, LEFT_CHANGED)))
		{
			p = newNode(depth);
			runNode(p->ul, node->ul, up->dl, node->dl, left->ur, node->ur, upleft->dr, up->dr, left->dr, node->dr, depth - 1);
			runNode(p->ur, node->ur, up->dr, node->dr, node->ul, right->ul, up->dl, upright->dl, node->dl, right->dl, depth - 1);
			runNode(p->dl, node->dl, node->ul, down->ul, left->dr, node->dr, left->ur, node->ur, downleft->ur, down->ur, depth - 1);
			runNode(p->dr, node->dr, node->ur, down->ur, node->dl, right->dl, node->ul, right->ul, down->ul, downright->ul, depth - 1);
			computeNodeInfo(p, depth);
		}
		else if (node != emptyNode(depth))
		{
			SET_BIT(node->flag, KEEP);
			p = node;
		}
	}
}

void NaiveLife::run()
{
	m_running = true;
	m_writeLock->lock();
	if (m_root->ul->population - m_root->ul->dr->population || m_root->ur->population - m_root->ur->dl->population || m_root->dl->population - m_root->dl->ur->population || m_root->dr->population - m_root->dr->ul->population)
	{
		m_readLock->lock();
		expand();
		m_readLock->unlock();
	}
	Node *new_root = emptyNode(m_depth), *empty = emptyNode(m_depth);
	runNode(new_root, m_root, empty, empty, empty, empty, empty, empty, empty, empty, m_depth);
	m_readLock->lock();
	deleteNode(m_root, m_depth);
	m_root = new_root;
	m_readLock->unlock();
	m_writeLock->unlock();
	m_generation = m_generation + 1;
	m_running = false;
	emit gridChanged();
}
