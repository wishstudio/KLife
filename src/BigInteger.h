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

#include <QDebug>

class QDebug;
class QString;
class BigInteger
{
public:
	BigInteger();
	BigInteger(qint32 num);
	BigInteger(quint32 num);
	BigInteger(qint64 num);
	BigInteger(quint64 num);
	BigInteger(const BigInteger &num);
	BigInteger(const QString &num);
	~BigInteger();

	BigInteger& operator = (qint32 num);
	BigInteger& operator = (quint32 num);
	BigInteger& operator = (qint64 num);
	BigInteger& operator = (quint64 num);
	BigInteger& operator = (const BigInteger &num);

	operator int() const;
	operator QString() const;

	static BigInteger exp2(int exp);
	int sgn() const;
	size_t bitCount() const;
	int bit(size_t id) const;
	void setBit(size_t id);
	template <typename T>
	inline T lowbits(size_t count) const
	{
/*		T ret = 0;
		int c = 0, n = 0;
		while (count > GMP_LIMB_BITS)
		{
			ret |= mpz_getlimbn(data, n++) << c;
			count -= GMP_LIMB_BITS;
			c += GMP_LIMB_BITS;
		}
        return ret | ((mpz_getlimbn(data, n) & ((1 << count) - 1)) << c);*/
        return 0;
	}

	BigInteger operator + (const BigInteger &num) const;
	BigInteger operator + (int num) const;
    BigInteger operator + (uint num) const;
    BigInteger operator + (ulong num) const;
	BigInteger& operator += (const BigInteger &num);
	BigInteger& operator += (int num);

	BigInteger operator - (const BigInteger &num) const;
	BigInteger operator - (int num) const;
    BigInteger operator - (uint num) const;
    BigInteger operator - (ulong num) const;
	BigInteger& operator -= (const BigInteger &num);
	BigInteger& operator -= (int num);

	BigInteger operator * (int num) const;
	BigInteger operator / (int num) const;

    BigInteger operator << (int num) const;
    BigInteger operator << (uint num) const;
    BigInteger& operator <<= (uint num);

    BigInteger operator >> (int num) const;
    BigInteger operator >> (uint num) const;
    BigInteger& operator >>= (uint num);

	bool operator == (const BigInteger &num) const;
	bool operator == (int num) const;
	bool operator != (const BigInteger &num) const;
	bool operator != (int num) const;
	bool operator < (const BigInteger &num) const;
	bool operator < (int num) const;
	bool operator <= (const BigInteger &num) const;
	bool operator <= (int num) const;
	bool operator > (const BigInteger &num) const;
	bool operator > (int num) const;
	bool operator >= (const BigInteger &num) const;
	bool operator >= (int num) const;

/*	friend QDebug operator << (QDebug dbg, const BigInteger &num)
	{
		char *str = mpz_get_str(NULL, 10, num.data);
		dbg.nospace() << str;
		delete str;
		return dbg.space();
    }*/
};

#endif
