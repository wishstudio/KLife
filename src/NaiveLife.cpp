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

#include <cstdlib>

#include <QMutex>

#include "AlgorithmManager.h"
#include "CanvasPainter.h"
#include "NaiveLife.h"
#include "Utils.h"

REGISTER_ALGORITHM(NaiveLife)

static const size_t BLOCK_DEPTH = 3;
static const size_t BLOCK_SIZE = 1 << BLOCK_DEPTH;

// node flags
#define CHANGED       0
#define KEEP          1
#define UP_CHANGED    2
#define UP_NZ         3
#define DOWN_CHANGED  4
#define DOWN_NZ       5
#define LEFT_CHANGED  6
#define LEFT_NZ       7
#define RIGHT_CHANGED 8
#define RIGHT_NZ      9

struct Block
{
	unsigned char data[BLOCK_SIZE][BLOCK_SIZE];
	int flag;
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
};

NaiveLife::NaiveLife()
	: m_running(false), m_readLock(new QMutex()), m_writeLock(new QMutex()), m_x(0), m_y(0)
{
	setAcceptInfinity(false);
	m_emptyNode.resize(BLOCK_DEPTH + 1);
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
	computeBlockNZFlag(block);
	while (++depth <= m_depth)
		computeNodeFlag(stack[depth], depth);
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
	m_depth = BLOCK_DEPTH + 1;
	m_root->ul = m_root->ur = m_root->dl = m_root->dr = emptyNode(m_depth - 1);
	m_root->flag = 0;
	m_readLock->unlock();
	m_writeLock->unlock();
	emit gridChanged();
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
	computeNodeFlag(m_root->ul, m_depth);
	computeNodeFlag(m_root->ur, m_depth);
	computeNodeFlag(m_root->dl, m_depth);
	computeNodeFlag(m_root->dr, m_depth);
	computeNodeFlag(m_root, m_depth + 1);
	BigInteger offset = BigInteger::exp2(m_depth - 1);
	m_x -= offset;
	m_y -= offset;
	m_depth++;
}

inline void NaiveLife::computeBlockNZFlag(Block *block)
{
	CLR_BIT(block->flag, UP_NZ);
	CLR_BIT(block->flag, DOWN_NZ);
	CLR_BIT(block->flag, LEFT_NZ);
	CLR_BIT(block->flag, RIGHT_NZ);
	for (size_t i = 0; i < BLOCK_SIZE; i++)
	{
		if (block->data[0][i])
			block->flag |= BIT(UP_NZ);
		if (block->data[BLOCK_SIZE - 1][i])
			block->flag |= BIT(DOWN_NZ);
		if (block->data[i][0])
			block->flag |= BIT(LEFT_NZ);
		if (block->data[i][BLOCK_SIZE - 1])
			block->flag |= BIT(RIGHT_NZ);
	}
}

inline void NaiveLife::computeNodeFlag(Node *node, size_t depth)
{
	if (depth == BLOCK_DEPTH + 1)
	{
		Block *node_ul = reinterpret_cast<Block *>(node->ul);
		Block *node_ur = reinterpret_cast<Block *>(node->ur);
		Block *node_dl = reinterpret_cast<Block *>(node->dl);
		Block *node_dr = reinterpret_cast<Block *>(node->dr);
		node->flag = TEST_BIT(node_ul->flag, CHANGED) | TEST_BIT(node_ur->flag, CHANGED) | TEST_BIT(node_dl->flag, CHANGED) | TEST_BIT(node_dr->flag, CHANGED);

		node->flag |= TEST_BIT(node_ul->flag, UP_CHANGED);
		node->flag |= TEST_BIT(node_ul->flag, UP_NZ);
		node->flag |= TEST_BIT(node_ul->flag, LEFT_CHANGED);
		node->flag |= TEST_BIT(node_ul->flag, LEFT_NZ);

		node->flag |= TEST_BIT(node_ur->flag, UP_CHANGED);
		node->flag |= TEST_BIT(node_ur->flag, UP_NZ);
		node->flag |= TEST_BIT(node_ur->flag, RIGHT_CHANGED);
		node->flag |= TEST_BIT(node_ur->flag, RIGHT_NZ);

		node->flag |= TEST_BIT(node_dl->flag, DOWN_CHANGED);
		node->flag |= TEST_BIT(node_dl->flag, DOWN_NZ);
		node->flag |= TEST_BIT(node_dl->flag, LEFT_CHANGED);
		node->flag |= TEST_BIT(node_dl->flag, LEFT_NZ);

		node->flag |= TEST_BIT(node_dr->flag, DOWN_CHANGED);
		node->flag |= TEST_BIT(node_dr->flag, DOWN_NZ);
		node->flag |= TEST_BIT(node_dr->flag, RIGHT_CHANGED);
		node->flag |= TEST_BIT(node_dr->flag, RIGHT_NZ);
	}
	else
	{
		Node *node_ul = node->ul, *node_ur = node->ur, *node_dl = node->dl, *node_dr = node->dr;
		node->flag = TEST_BIT(node_ul->flag, CHANGED) | TEST_BIT(node_ur->flag, CHANGED) | TEST_BIT(node_dl->flag, CHANGED) | TEST_BIT(node_dr->flag, CHANGED);

		node->flag |= TEST_BIT(node_ul->flag, UP_CHANGED);
		node->flag |= TEST_BIT(node_ul->flag, UP_NZ);
		node->flag |= TEST_BIT(node_ul->flag, LEFT_CHANGED);
		node->flag |= TEST_BIT(node_ul->flag, LEFT_NZ);

		node->flag |= TEST_BIT(node_ur->flag, UP_CHANGED);
		node->flag |= TEST_BIT(node_ur->flag, UP_NZ);
		node->flag |= TEST_BIT(node_ur->flag, RIGHT_CHANGED);
		node->flag |= TEST_BIT(node_ur->flag, RIGHT_NZ);

		node->flag |= TEST_BIT(node_dl->flag, DOWN_CHANGED);
		node->flag |= TEST_BIT(node_dl->flag, DOWN_NZ);
		node->flag |= TEST_BIT(node_dl->flag, LEFT_CHANGED);
		node->flag |= TEST_BIT(node_dl->flag, LEFT_NZ);

		node->flag |= TEST_BIT(node_dr->flag, DOWN_CHANGED);
		node->flag |= TEST_BIT(node_dr->flag, DOWN_NZ);
		node->flag |= TEST_BIT(node_dr->flag, RIGHT_CHANGED);
		node->flag |= TEST_BIT(node_dr->flag, RIGHT_NZ);
	}
}

Block *NaiveLife::newBlock()
{
	Block *ret = newObject<Block>();
	memset(ret->data, 0, sizeof ret->data);
	ret->flag = 0;
	return ret;
}

Node *NaiveLife::newNode(size_t depth)
{
	Node *ret = newObject<Node>();
	ret->ul = ret->ur = ret->dl = ret->dr = emptyNode(depth - 1);
	ret->flag = 0;
	return ret;
}

Node *NaiveLife::emptyNode(size_t depth)
{
	if (static_cast<size_t>(m_emptyNode.size()) > depth)
		return m_emptyNode[depth];
	else
	{
		Node *ret = newNode(depth);
		m_emptyNode.push_back(ret);
		return ret;
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

// Some magic in paint() to get rid of BigInteger manipulation:
// The drawing is divided into 2 steps:
// 1) walk down 4 nodes cover the drawing area
// We have to guarantee coordinate (x, y) is in node ul during the process.
// Firstly set ul to root, and other nodes to the magic empty node, like
//  ul  0
//   0  0
// Then walk down the tree, we continually divide ul into 4 smaller parts
// Because 2^(level-1) is larger than w and h so the needed child of 4 nodes
// are unique, when depth > level
//
// 2) draw these 4 nodes
// We can guarantee the coordinates fits in int now.
void NaiveLife::paint(CanvasPainter *painter, const BigInteger &x, const BigInteger &y, int w, int h)
{
	m_readLock->lock();
	// Draw background
	painter->fillGrid(0, 0, w, h, 0);

	// Fit into range
	BigInteger x1 = x - m_x, y1 = y - m_y, x2 = x1 + (w - 1), y2 = y1 + (h - 1);

	if (x2.sgn() < 0 || (x1.sgn() > 0 && x1.bitCount() > m_depth) || y2.sgn() < 0 || (y1.sgn() > 0 && y1.bitCount() > m_depth))
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
	BigInteger len = BigInteger::exp2(m_depth);
	if (x2.bitCount() > m_depth)
		w = len - x1;
	if (y2.bitCount() > m_depth)
		h = len - y1;

	// Step 1
	size_t endDepth = qMax(static_cast<size_t>(qMax(bitlen(w), bitlen(h))), BLOCK_DEPTH), d = m_depth;
	Node *node_ul = m_root, *node_ur = emptyNode(m_depth), *node_dl = emptyNode(m_depth), *node_dr = emptyNode(m_depth);
	while (d > endDepth)
	{
		switch ((y1.bit(d - 1) << 1) | x1.bit(d - 1))
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
		d--;
	}

	// Step 2
	int sx1 = x1.lowbits(d), sy1 = y1.lowbits(d);
	drawNode(painter, node_ul, node_ur, node_dl, node_dr, sx1, sy1, sx1 + w - 1, sy1 + h - 1, d, offset_x - sx1, offset_y - sy1);
	m_readLock->unlock();
}

inline void NaiveLife::drawNode(CanvasPainter *painter, Node *node_ul, Node *node_ur, Node *node_dl, Node *node_dr, int x1, int y1, int x2, int y2, size_t depth, int offset_x, int offset_y)
{
	int len = 1 << depth;
	if (x1 < len && y1 < len)
		drawNode(painter, node_ul, x1, y1, qMin(x2, len), qMin(y2, len), depth, offset_x, offset_y);
	if (x2 >= len && y1 < len)
		drawNode(painter, node_ur, qMax(x1 - len, 0), y1, x2 - len, qMin(y2, len), depth, offset_x + len, offset_y);
	if (x1 < len && y2 >= len)
		drawNode(painter, node_dl, x1, qMax(y1 - len, 0), qMin(x2, len), y2 - len, depth, offset_x, offset_y + len);
	if (x2 >= len && y2 >= len)
		drawNode(painter, node_dr, qMax(x1 - len, 0), qMax(y1 - len, 0), x2 - len, y2 - len, depth, offset_x + len, offset_y + len);
}

void NaiveLife::drawNode(CanvasPainter *painter, Node *node, int x1, int y1, int x2, int y2, size_t depth, int offset_x, int offset_y)
{
	if (node == emptyNode(depth))
		painter->fillGrid(offset_x + x1, offset_y + y1, x2 - x1 + 1, y2 - y1 + 1, 0);
	else if (depth == BLOCK_DEPTH)
	{
		for (int x = x1; x <= x2; x++)
			for (int y = y1; y <= y2; y++)
				painter->drawGrid(offset_x + x, offset_y + y, reinterpret_cast<Block *>(node)->data[y][x]);
	}
	else
		drawNode(painter, node->ul, node->ur, node->dl, node->dr, x1, y1, x2, y2, depth - 1, offset_x, offset_y);
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
			p = reinterpret_cast<Node *>(newBlock());
			Block *block = reinterpret_cast<Block *>(p);
			block->flag = 0;
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
			computeBlockNZFlag(block);
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
			computeNodeFlag(p, depth);
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
	if (TEST_BIT(m_root->flag, UP_NZ) || TEST_BIT(m_root->flag, DOWN_NZ) || TEST_BIT(m_root->flag, LEFT_NZ) || TEST_BIT(m_root->flag, RIGHT_NZ))
	{
		m_readLock->lock();
		expand();
		m_readLock->unlock();
	}
	Node *p = newNode(m_depth), *empty = emptyNode(m_depth);
	runNode(p, m_root, empty, empty, empty, empty, empty, empty, empty, empty, m_depth);
	m_readLock->lock();
	deleteNode(m_root, m_depth);
	m_root = p;
	m_readLock->unlock();
	m_writeLock->unlock();
	m_running = false;
	emit gridChanged();
}
