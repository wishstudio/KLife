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

#include <QString>

#include "BigInteger.h"

BigInteger::BigInteger(int num)
{
	mpz_init_set_si(data, (long int) num);
}

BigInteger::BigInteger(const BigInteger &num)
{
	mpz_init_set(data, num.data);
}

BigInteger::BigInteger(const QString &num)
{
	mpz_init_set_str(data, num.toAscii(), 10);
}

BigInteger::~BigInteger()
{
	mpz_clear(data);
}

BigInteger& BigInteger::operator = (int num)
{
	mpz_set_si(data, (long int) num);
	return *this;
}

BigInteger& BigInteger::operator = (const BigInteger &num)
{
	mpz_set(data, num.data);
	return *this;
}

BigInteger::operator int() const
{
	return mpz_get_si(data);
}

BigInteger::operator QString() const
{
	char *num = mpz_get_str(NULL, 10, data);
	QString ret(num);
	delete num;
	return ret;
}

BigInteger BigInteger::exp2(int exp)
{
	BigInteger ret;
	ret.setBit(exp);
	return ret;
}

int BigInteger::sgn() const
{
	return mpz_sgn(data);
}

size_t BigInteger::bitCount() const
{
	return mpz_sizeinbase(data, 2);
}

int BigInteger::bit(size_t id) const
{
	return mpz_tstbit(data, (mp_bitcnt_t) id);
}

void BigInteger::setBit(size_t id)
{
	mpz_setbit(data, (mp_bitcnt_t) id);
}

int BigInteger::lowbits(size_t count) const
{
	return mpz_getlimbn(data, 0) & ((1 << count) - 1);
}

BigInteger BigInteger::operator + (const BigInteger &num) const
{
	BigInteger ret;
	mpz_add(ret.data, data, num.data);
	return ret;
}

BigInteger BigInteger::operator + (int num) const
{
	BigInteger ret;
	if (num > 0)
		mpz_add_ui(ret.data, data, (unsigned long int) num);
	else
		mpz_sub_ui(ret.data, data, (unsigned long int) -num);
	return ret;
}

BigInteger& BigInteger::operator += (const BigInteger &num)
{
	mpz_add(data, data, num.data);
	return *this;
}

BigInteger& BigInteger::operator += (int num)
{
	if (num > 0)
		mpz_add_ui(data, data, (unsigned long int) num);
	else
		mpz_sub_ui(data, data, (unsigned long int) -num);
	return *this;
}

BigInteger BigInteger::operator - (const BigInteger &num) const
{
	BigInteger ret;
	mpz_sub(ret.data, data, num.data);
	return ret;
}

BigInteger BigInteger::operator - (int num) const
{
	BigInteger ret;
	if (num > 0)
		mpz_sub_ui(ret.data, data, (unsigned long int) num);
	else
		mpz_add_ui(ret.data, data, (unsigned long int) -num);
	return ret;
}

BigInteger& BigInteger::operator -= (const BigInteger &num)
{
	mpz_sub(data, data, num.data);
	return *this;
}

BigInteger& BigInteger::operator -= (int num)
{
	if (num > 0)
		mpz_sub_ui(data, data, (unsigned long int) num);
	else
		mpz_add_ui(data, data, (unsigned long int) -num);
	return *this;
}

BigInteger BigInteger::operator * (int num) const
{
	BigInteger ret;
	mpz_mul_si(ret.data, data, (long int) num);
	return ret;
}

BigInteger BigInteger::operator / (int num) const
{
	BigInteger ret;
	if (num > 0)
		mpz_div_ui(ret.data, data, (unsigned long int) num);
	else
	{
		mpz_div_ui(ret.data, data, (unsigned long int) -num);
		mpz_neg(ret.data, ret.data);
	}
	return ret;
}

bool BigInteger::operator == (const BigInteger &num) const
{
	return mpz_cmp(data, num.data) == 0;
}

bool BigInteger::operator == (int num) const
{
	return mpz_cmp_si(data, num) == 0;
}

bool BigInteger::operator != (const BigInteger &num) const
{
	return mpz_cmp(data, num.data) != 0;
}

bool BigInteger::operator != (int num) const
{
	return mpz_cmp_si(data, num) != 0;
}

bool BigInteger::operator < (const BigInteger &num) const
{
	return mpz_cmp(data, num.data) < 0;
}

bool BigInteger::operator < (int num) const
{
	return mpz_cmp_si(data, num) < 0;
}

bool BigInteger::operator <= (const BigInteger &num) const
{
	return mpz_cmp(data, num.data) <= 0;
}

bool BigInteger::operator <= (int num) const
{
	return mpz_cmp_si(data, num) <= 0;
}

bool BigInteger::operator > (const BigInteger &num) const
{
	return mpz_cmp(data, num.data) > 0;
}

bool BigInteger::operator > (int num) const
{
	return mpz_cmp_si(data, num) > 0;
}

bool BigInteger::operator >= (const BigInteger &num) const
{
	return mpz_cmp(data, num.data) >= 0;
}

bool BigInteger::operator >= (int num) const
{
	return mpz_cmp_si(data, num) >= 0;
}
