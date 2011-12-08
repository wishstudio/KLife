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

#ifndef BIGINTEGER_H
#define BIGINTEGER_H

#include <gmp.h>

class QString;
class BigInteger
{
public:
	BigInteger(int num = 0);
	BigInteger(const BigInteger &num);
	BigInteger(const QString &num);
	~BigInteger();

	BigInteger& operator = (int num);
	BigInteger& operator = (const BigInteger &num);

	operator int() const;
	operator QString() const;

	BigInteger operator + (const BigInteger &num) const;
	BigInteger operator + (int num) const;
	BigInteger& operator += (const BigInteger &num);
	BigInteger& operator += (int num);

	BigInteger operator - (const BigInteger &num) const;
	BigInteger operator - (int num) const;
	BigInteger& operator -= (const BigInteger &num);
	BigInteger& operator -= (int num);

	BigInteger operator * (int num) const;
	BigInteger operator % (int num) const;

	bool operator == (const BigInteger &num) const;
	bool operator != (const BigInteger &num) const;
	bool operator < (const BigInteger &num) const;
	bool operator <= (const BigInteger &num) const;
	bool operator > (const BigInteger &num) const;
	bool operator >= (const BigInteger &num) const;

private:
	mpz_t data;
};

#endif
