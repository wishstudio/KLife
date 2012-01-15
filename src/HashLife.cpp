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

#include "AlgorithmManager.h"
#include "HashLife.h"
#include "RuleLife.h"
#include "TreeUtils.h"

REGISTER_ALGORITHM(HashLife)

#define ul child[0]
#define ur child[1]
#define dl child[2]
#define dr child[3]

struct Block
{
	static const size_t DEPTH = 1;
	static const size_t SIZE = 1 << DEPTH;

	Block *next;

	unsigned char child[4];
	int population;

	Block() {}
	Block(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3)
	{
		child[0] = c0;
		child[1] = c1;
		child[2] = c2;
		child[3] = c3;
		population = (c0 > 0) + (c1 > 0) + (c2 > 0) + (c3 > 0);
	}

	inline int get(int x, int y)
	{
		return child[(y << 1) | x];
	}

	inline bool visible(HashLife *, size_t depth)
	{
		return population > 0;
	}

	static inline ulong hash(unsigned char c0, unsigned char c1, unsigned char c2, unsigned c3)
	{
		return 5 * c0 + 17 * c1 + 257 * c2 + 65537 * c3;
	}

	static inline void runStep(Rule *rule, size_t step, unsigned char &rul, unsigned char &rur, unsigned char &rdl, unsigned char &rdr, Block *bul, Block *bur, Block *bdl, Block *bdr)
	{
		// TODO
		Q_UNUSED(step);
		if (rule->type() == Rule::Life)
		{
			// * * * 0
			// * x * 0
			// * * * 0
			// 0 0 0 0
			int cul = bul->ul + bul->ur + bul->dl + bur->ul + bur->dl + bdl->ul + bdl->ur + bdr->ul;
			rul = reinterpret_cast<RuleLife *>(rule)->nextState(bul->dr, cul);
			// 0 * * *
			// 0 * x *
			// 0 * * *
			// 0 0 0 0
			int cur = bul->ur + bul->dr + bur->ul + bur->ur + bur->dr + bdl->ur + bdr->ul + bdr->ur;
			rur = reinterpret_cast<RuleLife *>(rule)->nextState(bur->dl, cur);
			// 0 0 0 0
			// * * * 0
			// * x * 0
			// * * * 0
			int cdl = bul->dl + bul->dr + bur->dl + bdl->ul + bdl->dl + bdl->dr + bdr->ul + bdr->dl;
			rdl = reinterpret_cast<RuleLife *>(rule)->nextState(bdl->ur, cdl);
			// 0 0 0 0
			// 0 * * *
			// 0 * x *
			// 0 * * *
			int cdr = bul->dr + bur->dl + bur->dr + bdl->ur + bdl->dr + bdr->ur + bdr->dl + bdr->dr;
			rdr = reinterpret_cast<RuleLife *>(rule)->nextState(bdr->ul, cdr);
		}
	}
};

struct Node
{
	Node *next;

	Node *child[4];
	Node *result;

	Node() {}
	Node(Node *c0, Node *c1, Node *c2, Node *c3)
	{
		child[0] = c0;
		child[1] = c1;
		child[2] = c2;
		child[3] = c3;
		result = NULL;
	}

	inline bool visible(HashLife *algorithm, size_t depth)
	{
		return this != algorithm->emptyNode(depth);
	}

	static inline ulong hash(Node *c0, Node *c1, Node *c2, Node *c3)
	{
		return 5 * reinterpret_cast<ulong>(c0) + 17 * reinterpret_cast<ulong>(c1) + 257 * reinterpret_cast<ulong>(c2) + 65537 * reinterpret_cast<ulong>(c3);
	}
};

template <typename T>
class HashTable
{
public:
	typedef __typeof__(T::child[0]) subtype;

	HashTable()
	{
		memset(data, 0, sizeof data);
	}

	~HashTable()
	{
		for (ulong i = 0; i < SIZE; i++)
			for (T *p = data[i]; p;)
			{
				T *q = p;
				p = q->next;
				delete q;
			}
	}

	T *get(subtype c0, subtype c1, subtype c2, subtype c3)
	{
		uint h = T::hash(c0, c1, c2, c3) % SIZE;
		for (T *p = data[h]; p; p = p->next)
			if (p->child[0] == c0 && p->child[1] == c1 && p->child[2] == c2 && p->child[3] == c3)
				return p;
		T *p = new T(c0, c1, c2, c3);
		p->next = data[h];
		data[h] = p;
		return p;
	}

private:
	static const ulong SIZE = 1000003UL;

	T *data[SIZE];
};

HashLife::HashLife()
	: m_readLock(new QMutex()), m_writeLock(new QMutex()), m_running(false),
	  m_blockHash(new HashTable<Block>()), m_nodeHash(new HashTable<Node>()),
	  m_x(0), m_y(0), m_generation(0)
{
	Node *e = reinterpret_cast<Node *>(m_blockHash->get(0, 0, 0, 0));
	m_emptyNode.resize(Block::DEPTH + 1);
	for (size_t i = 0; i < Block::DEPTH; i++)
		m_emptyNode[i] = NULL;
	m_emptyNode[Block::DEPTH] = e;
	m_depth = Block::DEPTH + 1;
	m_root = m_nodeHash->get(e, e, e, e);
}

HashLife::~HashLife()
{
	delete m_readLock;
	delete m_writeLock;
	delete m_blockHash;
	delete m_nodeHash;
}

void HashLife::setGrid(const BigInteger &x, const BigInteger &y, int state)
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
	int cid_stack[m_depth + 1];
	size_t depth = m_depth;
	while (depth > Block::DEPTH)
	{
		stack[depth] = p;
		int cid = (my_y.bit(depth - 1) << 1) | my_x.bit(depth - 1);
		cid_stack[depth] = cid;
		depth--;
		p = p->child[cid];
	}
	Block *block = reinterpret_cast<Block *>(p);
	int cid = (my_y.lowbits<int>(Block::DEPTH) << 1) | my_x.lowbits<int>(Block::DEPTH);
	if (block->child[cid] != state)
	{
		unsigned char c[4];
		c[0] = block->child[0];
		c[1] = block->child[1];
		c[2] = block->child[2];
		c[3] = block->child[3];
		c[cid] = state;
		p = reinterpret_cast<Node *>(m_blockHash->get(c[0], c[1], c[2], c[3]));
		while (depth++ < m_depth)
		{
			Node *c[4], *node = stack[depth];
			c[0] = node->child[0];
			c[1] = node->child[1];
			c[2] = node->child[2];
			c[3] = node->child[3];
			c[cid_stack[depth]] = p;
			p = m_nodeHash->get(c[0], c[1], c[2], c[3]);
		}
		m_root = p;
	}
	m_writeLock->unlock();
	emit gridChanged();
}

void HashLife::paint(CanvasPainter *painter, const BigInteger &x, const BigInteger &y, int w, int h, size_t scale)
{
	m_readLock->lock();
	treePaint<HashLife, Block, Node>(this, painter, x, y, w, h, scale, m_x, m_y, m_depth, m_root, emptyNode(m_depth));
	m_readLock->unlock();
}

BigInteger HashLife::generation() const
{
	return m_generation;
}

BigInteger HashLife::population() const
{
	return 0;
}

Node *HashLife::emptyNode(size_t depth)
{
	if (depth >= static_cast<size_t>(m_emptyNode.size()))
	{
		Node *sub = emptyNode(depth - 1);
		Node *node = m_nodeHash->get(sub, sub, sub, sub);
		m_emptyNode.push_back(node);
		return node;
	}
	else
		return m_emptyNode[depth];
}

void HashLife::expand()
{
	Node *e = emptyNode(m_depth - 1);
	Node *nul = m_nodeHash->get(e, e, e, m_root->ul);
	Node *nur = m_nodeHash->get(e, e, m_root->ur, e);
	Node *ndl = m_nodeHash->get(e, m_root->dl, e, e);
	Node *ndr = m_nodeHash->get(m_root->dr, e, e, e);
	m_root = m_nodeHash->get(nul, nur, ndl, ndr);
	BigInteger offset = BigInteger::exp2(m_depth - 1);
	m_x -= offset;
	m_y -= offset;
	m_depth++;
}

Node *HashLife::runNode(Node *node, size_t depth)
{
	if (node->result)
		return node->result;
	if (depth == Block::DEPTH + 1)
	{
		__typeof__(Block::child[0]) nul, nur, ndl, ndr;
		Block::runStep(AlgorithmManager::rule(), 1, nul, nur, ndl, ndr, reinterpret_cast<Block *>(node->ul), reinterpret_cast<Block *>(node->ur), reinterpret_cast<Block *>(node->dl), reinterpret_cast<Block *>(node->dr));
		// TODO: save result to hash table and return
		return NULL;
	}
	else
	{
		// HashLife runStep routine
		// 1. Calculate 9 sub-nodes
		// 0 0 0 0 0 0 0 0
		// 0 a a b b c c 0
		// 0 a a b b c c 0
		// 0 d d e e f f 0
		// 0 d d e e f f 0
		// 0 g g h h i i 0
		// 0 g g h h i i 0
		// 0 0 0 0 0 0 0 0
		Node *a = runNode(node->ul, depth - 1);
		Node *b = runNode(m_nodeHash->get(node->ul->ur, node->ur->ul, node->ul->dr, node->ur->dl), depth - 1);
		Node *c = runNode(node->ur, depth - 1);
		Node *d = runNode(m_nodeHash->get(node->ul->dl, node->ul->dr, node->dl->ul, node->dl->ur), depth - 1);
		Node *e = runNode(m_nodeHash->get(node->ul->dr, node->ur->dl, node->dl->ur, node->dr->ul), depth - 1);
		Node *f = runNode(m_nodeHash->get(node->ur->dl, node->ur->dr, node->dr->ul, node->dr->ur), depth - 1);
		Node *g = runNode(node->dl, depth - 1);
		Node *h = runNode(m_nodeHash->get(node->dl->ur, node->dr->ul, node->dl->dr, node->dr->dl), depth - 1);
		Node *i = runNode(node->dr, depth - 1);
		if (m_increment >= depth) // no need to do more increment
		{
			Node *nul = m_nodeHash->get(a->dr, b->dl, d->ur, e->ul);
			Node *nur = m_nodeHash->get(b->dr, c->dl, e->ur, f->ul);
			Node *ndl = m_nodeHash->get(d->dr, e->dl, g->ur, h->ul);
			Node *ndr = m_nodeHash->get(e->dr, f->dl, h->ur, i->ul);
			return m_nodeHash->get(nul, nur, ndl, ndr);
		}
		// else use the full increment power
		// 2. Calculate final RESULT
		// 0   0  0  0  0  0
		// 0  ul ul ur ur  0
		// 0  ul ul ur ur  0
		// 0  dl dl dr dr  0
		// 0  dl dl dr dr  0
		// 0   0  0  0  0  0
		Node *nul = runNode(m_nodeHash->get(a, b, d, e), depth - 1);
		Node *nur = runNode(m_nodeHash->get(b, c, e, f), depth - 1);
		Node *ndl = runNode(m_nodeHash->get(d, e, g, h), depth - 1);
		Node *ndr = runNode(m_nodeHash->get(e, f, h, i), depth - 1);
		return m_nodeHash->get(nul, nur, ndl, ndr);
	}
}
