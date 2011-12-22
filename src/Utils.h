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

#ifndef UTILS_H
#define UTILS_H

// Bit manipulation utils
#define BIT(b) (1 << (b))
#define TEST_BIT(x, b) ((x) & BIT(b))
#define SET_BIT(x, b) ((x) = (x) | BIT(b))
#define CLR_BIT(x, b) ((x) = (x) & ~BIT(b))

extern int bitlen(int num);

// Factory manipulation utils
#define ABSTRACT_FACTORY(baseClassName) \
class Abstract##baseClassName##Factory \
{ \
public: \
	virtual ~Abstract##baseClassName##Factory() {} \
	virtual Abstract##baseClassName *create##baseClassName() = 0; \
}; \
template<typename baseClassName##Class> \
class baseClassName##Factory: public Abstract##baseClassName##Factory \
{ \
public: \
	virtual Abstract##baseClassName *create##baseClassName() { return new baseClassName##Class(); } \
};

#define REGISTER_FACTORY_CLASS(baseClassName, factoryClassName, className) \
class ___##baseClassName##className##Factory: public factoryClassName::Abstract##baseClassName##Factory \
{ \
public: \
	___##baseClassName##className##Factory() \
	{ \
		factoryClassName::register##baseClassName(this); \
	} \
	virtual Abstract##baseClassName *create##baseClassName() { return new className(); } \
} ___##factoryClassName##className##Factory_INSTANCE;

#endif
