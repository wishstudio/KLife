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

static const size_t BLOCK_DEPTH = 3;
static const size_t BLOCK_SIZE = 1 << BLOCK_DEPTH;

// node flags
#define CHANGED       0  // This node has changed since last iteration
#define KEEP          1  // Prevent this node from being deleted when deleting previous iteration
#define UP_CHANGED    2  // The top row has changed since last iteration
#define UP_ACTIVE     3  // The top row has active cells
#define DOWN_CHANGED  4  // The bottom row has changed since last iteration
#define DOWN_ACTIVE   5  // The bottom row has active cells
#define LEFT_CHANGED  6  // The left column has changed since last iteration
#define LEFT_ACTIVE   7  // The left column has active cells
#define RIGHT_CHANGED 8  // The right column has changed since last iteration
#define RIGHT_ACTIVE  9  // The right column has active cells

struct Block
{
	unsigned char data[BLOCK_SIZE][BLOCK_SIZE];
	int flag;
	quint64 population;
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
	m_emptyNode.resize(BLOCK_DEPTH + 1);
	for (size_t i = 0; i < BLOCK_DEPTH; i++)
		m_emptyNode[i] = NULL;
	m_emptyNode[BLOCK_DEPTH] = reinterpret_cast<Node *>(newBlock());
	reinterpret_cast<Block *>(m_emptyNode[BLOCK_DEPTH])->flag = 0;
	// initially a 16x16 block
	m_root = newNode(BLOCK_DEPTH + 1);
	m_depth = BLOCK_DEPTH + 1;
}

NaiveLife::~NaiveLife()
{
	delete m_readLock;
	delete m_writeLock;
	deleteNode(m_root, m_depth);
	for (int i = BLOCK_DEPTH; i < m_emptyNode.size(); i++)
		deleteNode(m_emptyNode[i], i);
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
	while (depth > BLOCK_DEPTH)
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
		ret = reinterpret_cast<Block *>(p)->data[my_y.lowbits(BLOCK_DEPTH)][my_x.lowbits(BLOCK_DEPTH)];
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
	while (depth > BLOCK_DEPTH)
	{
		stack[depth] = p;
		int cid = (my_y.bit(depth - 1) << 1) | my_x.bit(depth - 1);
		depth--;
		if (p->child[cid] == emptyNode(depth))
		{
			if (depth == BLOCK_DEPTH)
				p->child[cid] = reinterpret_cast<Node *>(newBlock());
			else
				p->child[cid] = newNode(depth);
		}
		p = p->child[cid];
	}
	Block *block = reinterpret_cast<Block *>(p);
	int sx = my_x.lowbits(BLOCK_DEPTH), sy = my_y.lowbits(BLOCK_DEPTH);
	if (block->data[sy][sx] && !state)
		block->population--;
	else if (!block->data[sy][sx] && state)
		block->population++;
	block->data[sy][sx] = state;
	block->flag |= BIT(CHANGED);
	if (sy == 0)
		block->flag |= BIT(UP_CHANGED);
	if (sy == BLOCK_SIZE - 1)
		block->flag |= BIT(DOWN_CHANGED);
	if (sx == 0)
		block->flag |= BIT(LEFT_CHANGED);
	if (sx == BLOCK_SIZE - 1)
		block->flag |= BIT(RIGHT_CHANGED);
	computeBlockActiveFlag(block);
	while (++depth <= m_depth)
		computeNodeInfo(stack[depth], depth);
	m_writeLock->unlock();
	emit gridChanged();
}

void NaiveLife::fillRect(const BigInteger &x, const BigInteger &y, int w, int h, int state)
{
	// out of range
	BigInteger x1 = x - m_x, y1 = y - m_y, x2 = x1 + (w - 1), y2 = y1 + (h - 1);
	// TODO optimization
	while (x1.sgn() < 0 || x2.bitCount() > m_depth || y1.sgn() < 0 || y2.bitCount() > m_depth)
	{
		expand();
		x1 = x - m_x;
		y1 = y - m_y;
		x2 = x1 + (w - 1);
		y2 = y1 + (h - 1);
	}

	// Step 1
	size_t depth = m_depth;
	Node **node_ul, **node_ur, **node_dl, **node_dr;
	Node *rec_ul[m_depth + 1], *rec_ur[m_depth + 1], *rec_dl[m_depth + 1], *rec_dr[m_depth + 1];
	walkDown(x1, y1, w, h, node_ul, node_ur, node_dl, node_dr, depth, rec_ul, rec_ur, rec_dl, rec_dr, true);

	// Step 2
	int sx1 = x1.lowbits(depth), sy1 = y1.lowbits(depth);
	fillRect(*node_ul, *node_ur, *node_dl, *node_dr, sx1, sy1, sx1 + w - 1, sy1 + h - 1, state, depth);

	for (size_t d = depth + 1; d <= m_depth; d++)
	{
		computeNodeInfo(rec_ul[d], d);
		computeNodeInfo(rec_ur[d], d);
		computeNodeInfo(rec_dl[d], d);
		computeNodeInfo(rec_dr[d], d);
	}

	emit gridChanged();
}

inline void NaiveLife::fillRect(Node *&node_ul, Node *&node_ur, Node *&node_dl, Node *&node_dr, int x1, int y1, int x2, int y2, int state, size_t depth)
{
	int len = 1 << depth;
	if (x1 < len && y1 < len)
		fillRect(node_ul, x1, y1, qMin(x2, len - 1), qMin(y2, len - 1), state, depth);
	if (x2 >= len && y1 < len)
		fillRect(node_ur, qMax(x1 - len, 0), y1, x2 - len, qMin(y2, len - 1), state, depth);
	if (x1 < len && y2 >= len)
		fillRect(node_dl, x1, qMax(y1 - len, 0), qMin(x2, len - 1), y2 - len, state, depth);
	if (x2 >= len && y2 >= len)
		fillRect(node_dr, qMax(x1 - len, 0), qMax(y1 - len, 0), x2 - len, y2 - len, state, depth);
}

void NaiveLife::fillRect(Node *&node, int x1, int y1, int x2, int y2, int state, size_t depth)
{
	if (depth == BLOCK_DEPTH)
	{
		if (node == emptyNode(depth))
			node = reinterpret_cast<Node *>(newBlock());
		Block *block = reinterpret_cast<Block *>(node);
		block->flag = BIT(CHANGED);
		for (int x = x1; x <= x2; x++)
			for (int y = y1; y <= y2; y++)
			{
				if (block->data[y][x] != state)
				{
					if (y == 0)
						SET_BIT(block->flag, UP_CHANGED);
					if (y == BLOCK_SIZE - 1)
						SET_BIT(block->flag, DOWN_CHANGED);
					if (x == 0)
						SET_BIT(block->flag, LEFT_CHANGED);
					if (x == BLOCK_SIZE - 1)
						SET_BIT(block->flag, RIGHT_CHANGED);
					if (!block->data[y][x] && state)
						block->population++;
					else
						block->population--;
					block->data[y][x] = state;
				}
			}
		computeBlockActiveFlag(block);
	}
	else
	{
		if (node == emptyNode(depth))
			node = newNode(depth);
		fillRect(node->ul, node->ur, node->dl, node->dr, x1, y1, x2, y2, state, depth - 1);
		computeNodeInfo(node, depth);
	}
}

void NaiveLife::clearGrid()
{
	m_writeLock->lock();
	m_readLock->lock();
	deleteNode(m_root->ul, m_depth - 1);
	deleteNode(m_root->ur, m_depth - 1);
	deleteNode(m_root->dl, m_depth - 1);
	deleteNode(m_root->dr, m_depth - 1);
	m_depth = BLOCK_DEPTH + 1;
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

void NaiveLife::rectChange(const BigInteger &x, const BigInteger &y, const BigInteger &, const BigInteger &)
{
	m_x = x;
	m_y = y;
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

inline void NaiveLife::computeBlockActiveFlag(Block *block)
{
	CLR_BIT(block->flag, UP_ACTIVE);
	CLR_BIT(block->flag, DOWN_ACTIVE);
	CLR_BIT(block->flag, LEFT_ACTIVE);
	CLR_BIT(block->flag, RIGHT_ACTIVE);
	for (size_t i = 0; i < BLOCK_SIZE; i++)
	{
		if (block->data[0][i])
			block->flag |= BIT(UP_ACTIVE);
		if (block->data[BLOCK_SIZE - 1][i])
			block->flag |= BIT(DOWN_ACTIVE);
		if (block->data[i][0])
			block->flag |= BIT(LEFT_ACTIVE);
		if (block->data[i][BLOCK_SIZE - 1])
			block->flag |= BIT(RIGHT_ACTIVE);
	}
}

inline void NaiveLife::computeNodeInfo(Node *node, size_t depth)
{
	int ul_flag, ur_flag, dl_flag, dr_flag;
	if (depth == BLOCK_DEPTH + 1)
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
	node->flag |= TEST_BIT(ul_flag, UP_ACTIVE);
	node->flag |= TEST_BIT(ul_flag, LEFT_CHANGED);
	node->flag |= TEST_BIT(ul_flag, LEFT_ACTIVE);

	node->flag |= TEST_BIT(ur_flag, UP_CHANGED);
	node->flag |= TEST_BIT(ur_flag, UP_ACTIVE);
	node->flag |= TEST_BIT(ur_flag, RIGHT_CHANGED);
	node->flag |= TEST_BIT(ur_flag, RIGHT_ACTIVE);

	node->flag |= TEST_BIT(dl_flag, DOWN_CHANGED);
	node->flag |= TEST_BIT(dl_flag, DOWN_ACTIVE);
	node->flag |= TEST_BIT(dl_flag, LEFT_CHANGED);
	node->flag |= TEST_BIT(dl_flag, LEFT_ACTIVE);

	node->flag |= TEST_BIT(dr_flag, DOWN_CHANGED);
	node->flag |= TEST_BIT(dr_flag, DOWN_ACTIVE);
	node->flag |= TEST_BIT(dr_flag, RIGHT_CHANGED);
	node->flag |= TEST_BIT(dr_flag, RIGHT_ACTIVE);
}

Block *NaiveLife::newBlock()
{
	Block *ret = newObject<Block>();
	memset(ret->data, 0, sizeof ret->data);
	ret->flag = 0;
	ret->population = 0;
	return ret;
}

Node *NaiveLife::newNode(size_t depth)
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
	if (depth == BLOCK_DEPTH)
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

// Magic in walkDOwn() to get rid of BigInteger manipulation:
// We walk down and find 4 nodes cover the drawing area
// We have to guarantee coordinate (x, y) is in node ul during the process.
// Firstly set ul to root, and other nodes to the magic empty node, like
//  ul  0
//   0  0
// Then walk down the tree, we continually divide ul into 4 smaller parts
// Because 2^(level-1) is larger than w and h so the needed childs of 4 nodes
// are unique, when depth > endDepth
// After walkdown(), we can guarantee all the coordinates fit in ints.
inline void NaiveLife::walkDown(const BigInteger &x, const BigInteger &y, int w, int h, Node **&node_ul, Node **&node_ur, Node **&node_dl, Node **&node_dr, size_t &depth, Node *rec_ul[], Node *rec_ur[], Node *rec_dl[], Node *rec_dr[], bool record)
{
	node_ul = &m_root;
	node_ur = &emptyNode(m_depth);
	node_dl = &emptyNode(m_depth);
	node_dr = &emptyNode(m_depth);
	size_t endDepth = qMax(static_cast<size_t>(qMax(bitlen(w), bitlen(h))), BLOCK_DEPTH);
	while (depth > endDepth)
	{
		if (record)
		{
			rec_ul[depth] = *node_ul;
			rec_ur[depth] = *node_ur;
			rec_dl[depth] = *node_dl;
			rec_dr[depth] = *node_dr;
		}
		switch ((y.bit(depth - 1) << 1) | x.bit(depth - 1))
		{
		case 0:
			//  ul ur  0  0
			//  dl dr  0  0
			//   0  0  0  0
			//   0  0  0  0
			node_ur = &(*node_ul)->ur;
			node_dl = &(*node_ul)->dl;
			node_dr = &(*node_ul)->dr;
			node_ul = &(*node_ul)->ul;
			break;

		case 1:
			//   0 ul ur  0
			//   0 dl dr  0
			//   0  0  0  0
			//   0  0  0  0
			node_dl = &(*node_ul)->dr;
			node_ul = &(*node_ul)->ur;
			node_dr = &(*node_ur)->dl;
			node_ur = &(*node_ur)->ul;
			break;

		case 2:
			//   0  0  0  0
			//  ul ur  0  0
			//  dl dr  0  0
			//   0  0  0  0
			node_ur = &(*node_ul)->dr;
			node_ul = &(*node_ul)->dl;
			node_dr = &(*node_dl)->ur;
			node_dl = &(*node_dl)->ul;
			break;

		case 3:
			//   0  0  0  0
			//   0 ul ur  0
			//   0 dl dr  0
			//   0  0  0  0
			node_ul = &(*node_ul)->dr;
			node_ur = &(*node_ur)->dl;
			node_dl = &(*node_dl)->ur;
			node_dr = &(*node_dr)->ul;
			break;
		}
		depth--;
	}
}

void NaiveLife::paint(CanvasPainter *painter, const BigInteger &x, const BigInteger &y, int w, int h, size_t scale)
{
	m_readLock->lock();
	// Draw background
	painter->fillGrid(0, 0, w, h, 0);

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
		size_t depth = m_depth - scale;
		Node **node_ul, **node_ur, **node_dl, **node_dr;
		walkDown(x1, y1, w, h, node_ul, node_ur, node_dl, node_dr, depth, NULL, NULL, NULL, NULL, false);

		// Step 2
		int sx1 = x1.lowbits(depth), sy1 = y1.lowbits(depth);
		drawNode(painter, *node_ul, *node_ur, *node_dl, *node_dr, sx1, sy1, sx1 + w - 1, sy1 + h - 1, depth, scale, offset_x - sx1, offset_y - sy1);
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
	if (node == emptyNode(depth))
		painter->fillGrid(offset_x + x1, offset_y + y1, x2 - x1 + 1, y2 - y1 + 1, 0);
	else if (depth + scale == BLOCK_DEPTH)
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
								state |= reinterpret_cast<Block *>(node)->data[j][i] > 0;
					}
					else
						state = reinterpret_cast<Block *>(node)->data[y][x];
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
	if (depth == BLOCK_DEPTH)
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
			for (size_t i = 0; i < BLOCK_SIZE; i++)
				for (size_t j = 0; j < BLOCK_SIZE; j++)
				{
					int n = 0;
					for (int d = 0; d < 8; d++)
					{
						int di = static_cast<int>(i) + dy[d], dj = static_cast<int>(j) + dx[d];
						if (di < 0)
						{
							if (dj < 0)
								n += bupleft->data[BLOCK_SIZE - 1][BLOCK_SIZE - 1];
							else if (dj >= static_cast<int>(BLOCK_SIZE))
								n += bupright->data[BLOCK_SIZE - 1][0];
							else
								n += bup->data[BLOCK_SIZE - 1][dj];
						}
						else if (di >= static_cast<int>(BLOCK_SIZE))
						{
							if (dj < 0)
								n += bdownleft->data[0][BLOCK_SIZE - 1];
							else if (dj >= static_cast<int>(BLOCK_SIZE))
								n += bdownright->data[0][0];
							else
								n += bdown->data[0][dj];
						}
						else if (dj < 0)
							n += bleft->data[di][BLOCK_SIZE - 1];
						else if (dj >= static_cast<int>(BLOCK_SIZE))
							n += bright->data[di][0];
						else
							n += bnode->data[di][dj];
					}
					block->data[i][j] = ((n == 3) || (bnode->data[i][j] && n == 2));
					block->population += block->data[i][j] > 0;
					if (block->data[i][j] != bnode->data[i][j])
					{
						if (i == 0)
							SET_BIT(block->flag, UP_CHANGED);
						if (i == BLOCK_SIZE - 1)
							SET_BIT(block->flag, DOWN_CHANGED);
						if (j == 0)
							SET_BIT(block->flag, LEFT_CHANGED);
						if (j == BLOCK_SIZE - 1)
							SET_BIT(block->flag, RIGHT_CHANGED);
						SET_BIT(block->flag, CHANGED);
					}
				}
			computeBlockActiveFlag(block);
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
	if (TEST_BIT(m_root->flag, UP_ACTIVE) || TEST_BIT(m_root->flag, DOWN_ACTIVE) || TEST_BIT(m_root->flag, LEFT_ACTIVE) || TEST_BIT(m_root->flag, RIGHT_ACTIVE))
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
