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

#ifndef TREEUTILS_H
#define TREEUTILS_H

#include "BigInteger.h"
#include "CanvasPainter.h"
#include "Utils.h"

#define ul child[0]
#define ur child[1]
#define dl child[2]
#define dr child[3]

template <typename Block, typename Node>
inline void treePaintNode(CanvasPainter *painter, Node *node_ul, Node *node_ur, Node *node_dl, Node *node_dr, int x1, int y1, int x2, int y2, size_t depth, size_t scale, int offset_x, int offset_y);

template <typename Block, typename Node>
void treePaintNode(CanvasPainter *painter, Node *node, int x1, int y1, int x2, int y2, size_t depth, size_t scale, int offset_x, int offset_y);

/// Common paint() routine for binary tree based algorithms
// Some magic in treePaint() to get rid of BigInteger manipulation:
// We walk down and find 4 nodes cover the drawing area
// We have to guarantee coordinate (x, y) is in node ul during the process.
// Firstly set ul to root, and other nodes to the magic empty node, like
//  ul  0
//   0  0
// Then walk down the tree, we continually divide ul into 4 smaller parts
// Because 2^(level-1) is larger than w and h so the needed childs of 4 nodes
// are unique, when depth > endDepth
// After this walkdown, we can guarantee all the coordinates fit in ints.
template <typename Algorithm, typename Block, typename Node>
inline void treePaint(Algorithm *algorithm, CanvasPainter *painter, const BigInteger &x, const BigInteger &y, int w, int h, size_t scale, const BigInteger &m_x, const BigInteger &m_y, size_t m_depth, Node *m_root, Node *depth_emptyNode)
{
	// Fill black background
	painter->fillBlack();

	// Fit into range
	BigInteger x1 = x - (m_x >> scale), y1 = y - (m_y >> scale), x2 = x1 + (w - 1), y2 = y1 + (h - 1);

	// Out of range
	if (x2.sgn() < 0 || (x1.sgn() > 0 && x1.bitCount() > m_depth - scale) || y2.sgn() < 0 || (y1.sgn() > 0 && y1.bitCount() > m_depth - scale))
		return;

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
		treePaintNode<Algorithm, Block, Node>(algorithm, painter, m_root, 0, 0, 0, 0, 0, scale, offset_x, offset_y);
	else
	{
		BigInteger len = BigInteger::exp2(m_depth - scale);
		if (x2.bitCount() > m_depth - scale)
			w = len - x1;
		if (y2.bitCount() > m_depth - scale)
			h = len - y1;

		// Step 1
		size_t depth = m_depth - scale, endDepth = qMax<size_t>(qMax(bitlen(w), bitlen(h)), Block::DEPTH);
		Node *node_ul = m_root, *node_ur = depth_emptyNode, *node_dl = depth_emptyNode, *node_dr = depth_emptyNode;
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
		treePaintNode<Algorithm, Block, Node>(algorithm, painter, node_ul, node_ur, node_dl, node_dr, sx1, sy1, sx1 + w - 1, sy1 + h - 1, depth, scale, offset_x - sx1, offset_y - sy1);
	}
}

template <typename Algorithm, typename Block, typename Node>
inline void treePaintNode(Algorithm *algorithm, CanvasPainter *painter, Node *node_ul, Node *node_ur, Node *node_dl, Node *node_dr, int x1, int y1, int x2, int y2, size_t depth, size_t scale, int offset_x, int offset_y)
{
	int len = 1 << depth;
	if (x1 < len && y1 < len)
		treePaintNode<Algorithm, Block, Node>(algorithm, painter, node_ul, x1, y1, qMin(x2, len - 1), qMin(y2, len - 1), depth, scale, offset_x, offset_y);
	if (x2 >= len && y1 < len)
		treePaintNode<Algorithm, Block, Node>(algorithm, painter, node_ur, qMax(x1 - len, 0), y1, x2 - len, qMin(y2, len - 1), depth, scale, offset_x + len, offset_y);
	if (x1 < len && y2 >= len)
		treePaintNode<Algorithm, Block, Node>(algorithm, painter, node_dl, x1, qMax(y1 - len, 0), qMin(x2, len - 1), y2 - len, depth, scale, offset_x, offset_y + len);
	if (x2 >= len && y2 >= len)
		treePaintNode<Algorithm, Block, Node>(algorithm, painter, node_dr, qMax(x1 - len, 0), qMax(y1 - len, 0), x2 - len, y2 - len, depth, scale, offset_x + len, offset_y + len);
}

template <typename Algorithm, typename Block, typename Node>
void treePaintNode(Algorithm *algorithm, CanvasPainter *painter, Node *node, int x1, int y1, int x2, int y2, size_t depth, size_t scale, int offset_x, int offset_y)
{
	if (depth + scale == Block::DEPTH)
	{
		Block *block = reinterpret_cast<Block *>(node);
		if (depth == 0)
			painter->drawGrid(offset_x + x1, offset_y + y1, block->visible(algorithm, depth));
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
								state |= block->get(i, j) > 0;
					}
					else
						state = block->get(x, y);
					painter->drawGrid(offset_x + x, offset_y + y, state);
				}
		}
	}
	else if (depth == 0)
		painter->drawGrid(offset_x + x1, offset_y + y1, node->visible(algorithm, depth));
	else
		treePaintNode<Algorithm, Block, Node>(algorithm, painter, node->ul, node->ur, node->dl, node->dr, x1, y1, x2, y2, depth - 1, scale, offset_x, offset_y);
}

#undef ul
#undef ur
#undef dl
#undef dr

#endif
