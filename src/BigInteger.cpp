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
#include "Config.h"

BigInteger::BigInteger()
{
//	mpz_init(data);
}

// we believe sizeof(long int) >= sizeof(qint32)
BigInteger::BigInteger(qint32 num)
{
//	mpz_init_set_si(data, (signed long int) num);
}

BigInteger::BigInteger(quint32 num)
{
//	mpz_init_set_ui(data, (unsigned long int) num);
}

BigInteger::BigInteger(qint64 num)
{
/*#if SIZEOF_SIGNED_LONG == 8
	mpz_init_set_si(data, static_cast<signed long int>(num));
#elif SIZEOF_SIGNED_LONG == 4
	mpz_init_set_si(data, static_cast<signed long int>(num >> 32));
	mpz_mul_2exp(data, data, 32);
	mpz_add_ui(data, data, static_cast<signed long int>(num));
#else
#error "SIZEOF_SIGNED_LONG is a unhandled case"
#endif*/
}

BigInteger::BigInteger(quint64 num)
{
/*#if SIZEOF_UNSIGNED_LONG == 8
	mpz_init_set_ui(data, static_cast<unsigned long int>(num));
#elif SIZEOF_UNSIGNED_LONG == 4
	mpz_init_set_ui(data, static_cast<unsigned long int>(num >> 32));
	mpz_mul_2exp(data, data, 32);
	mpz_add_ui(data, data, static_cast<unsigned long int>(num));
#else
#error "SIZEOF_UNSIGNED_LONG is a unhandled case"
#endif*/
}

BigInteger::BigInteger(const BigInteger &num)
{
//	mpz_init_set(data, num.data);
}

BigInteger::BigInteger(const QString &num)
{
//	mpz_init_set_str(data, num.toAscii(), 10);
}

BigInteger::~BigInteger()
{
//	mpz_clear(data);
}

// Same as above
BigInteger& BigInteger::operator = (qint32 num)
{
//	mpz_set_si(data, (signed long int) num);
    return *this;
}

BigInteger& BigInteger::operator = (quint32 num)
{
//	mpz_set_si(data, (unsigned long int) num);
    return *this;
}

BigInteger& BigInteger::operator = (qint64 num)
{
/*#if SIZEOF_SIGNED_LONG == 8
	mpz_set_si(data, static_cast<signed long int>(num));
#elif SIZEOF_SIGNED_LONG == 4
	mpz_set_si(data, static_cast<signed long int>(num >> 32));
	mpz_mul_2exp(data, data, 32);
	mpz_add_ui(data, data, static_cast<signed long int>(num));
#else
#error "SIZEOF_SIGNED_LONG is a unhandled case"
#endif*/
	return *this;
}

BigInteger& BigInteger::operator = (quint64 num)
{
/*#if SIZEOF_UNSIGNED_LONG == 8
	mpz_set_ui(data, static_cast<unsigned long int>(num));
#elif SIZEOF_UNSIGNED_LONG == 4
	mpz_set_ui(data, static_cast<unsigned long int>(num >> 32));
	mpz_mul_2exp(data, data, 32);
	mpz_add_ui(data, data, static_cast<unsigned long int>(num));
#else
#error "SIZEOF_UNSIGNED_LONG is a unhandled case"
#endif*/
	return *this;
}

BigInteger& BigInteger::operator = (const BigInteger &num)
{
//	mpz_set(data, num.data);
	return *this;
}

BigInteger::operator int() const
{
//	return mpz_get_si(data);
    return 0;
}

BigInteger::operator QString() const
{
//	char *num = mpz_get_str(NULL, 10, data);
//	QString ret(num);
//	delete num;
//	return ret;
    return QString();
}

BigInteger BigInteger::exp2(int exp)
{
//	BigInteger ret;
//	ret.setBit(exp);
//	return ret;
    return BigInteger();
}

int BigInteger::sgn() const
{
//	return mpz_sgn(data);
    return 0;
}

size_t BigInteger::bitCount() const
{
//	return mpz_sizeinbase(data, 2);
    return 0;
}

int BigInteger::bit(size_t id) const
{
//	return mpz_tstbit(data, (mp_bitcnt_t) id);
    return 0;
}

void BigInteger::setBit(size_t id)
{
//	mpz_setbit(data, (mp_bitcnt_t) id);
}

BigInteger BigInteger::operator + (const BigInteger &num) const
{
//	BigInteger ret;
//	mpz_add(ret.data, data, num.data);
//	return ret;
    return BigInteger();
}

BigInteger BigInteger::operator + (int num) const
{
//	BigInteger ret;
//	if (num > 0)
//		mpz_add_ui(ret.data, data, (unsigned long int) num);
//	else
//		mpz_sub_ui(ret.data, data, (unsigned long int) -num);
//	return ret;
    return BigInteger();
}

BigInteger BigInteger::operator + (uint num) const
{
    return BigInteger();
}

BigInteger BigInteger::operator + (ulong num) const
{
//	BigInteger ret;
//	mpz_add_ui(ret.data, data, num);
//	return ret;
    return BigInteger();
}

BigInteger& BigInteger::operator += (const BigInteger &num)
{
//	mpz_add(data, data, num.data);
    return *this;
}

BigInteger& BigInteger::operator += (int num)
{
//	if (num > 0)
//		mpz_add_ui(data, data, (unsigned long int) num);
//	else
//		mpz_sub_ui(data, data, (unsigned long int) -num);
    return *this;
}

BigInteger BigInteger::operator - (const BigInteger &num) const
{
//	BigInteger ret;
//	mpz_sub(ret.data, data, num.data);
//	return ret;
    return BigInteger();
}

BigInteger BigInteger::operator - (int num) const
{
//	BigInteger ret;
//	if (num > 0)
//		mpz_sub_ui(ret.data, data, (unsigned long int) num);
//	else
//		mpz_add_ui(ret.data, data, (unsigned long int) -num);
//	return ret;
    return BigInteger();
}

BigInteger BigInteger::operator - (uint num) const
{
    return BigInteger();
}

BigInteger BigInteger::operator - (ulong num) const
{
//	BigInteger ret;
//	mpz_sub_ui(ret.data, data, num);
//	return ret;
    return BigInteger();
}

BigInteger& BigInteger::operator -= (const BigInteger &num)
{
//	mpz_sub(data, data, num.data);
    return *this;
}

BigInteger& BigInteger::operator -= (int num)
{
//	if (num > 0)
//		mpz_sub_ui(data, data, (unsigned long int) num);
//	else
//		mpz_add_ui(data, data, (unsigned long int) -num);
    return *this;
}

BigInteger BigInteger::operator * (int num) const
{
//	BigInteger ret;
//	mpz_mul_si(ret.data, data, (long int) num);
//	return ret;
    return BigInteger();
}

BigInteger BigInteger::operator / (int num) const
{
//	BigInteger ret;
//	if (num > 0)
//		mpz_div_ui(ret.data, data, (unsigned long int) num);
//	else
//	{
//		mpz_div_ui(ret.data, data, (unsigned long int) -num);
//		mpz_neg(ret.data, ret.data);
//	}
//  return ret;
    return BigInteger();
}

BigInteger BigInteger::operator << (int num) const
{
    return BigInteger();
}

BigInteger BigInteger::operator << (uint num) const
{
//	BigInteger ret;
//	mpz_mul_2exp(ret.data, data, num);
//  return ret;
    return BigInteger();
}

BigInteger& BigInteger::operator <<= (uint num)
{
//	mpz_mul_2exp(data, data, num);
	return *this;
}

BigInteger BigInteger::operator >> (int num) const
{
    return BigInteger();
}

BigInteger BigInteger::operator >> (uint num) const
{
//	BigInteger ret;
//	mpz_div_2exp(ret.data, data, num);
//	return ret;
    return BigInteger();
}

BigInteger& BigInteger::operator >>= (uint num)
{
//	mpz_div_2exp(data, data, num);
    return *this;
}

bool BigInteger::operator == (const BigInteger &num) const
{
//	return mpz_cmp(data, num.data) == 0;
    return 0;
}

bool BigInteger::operator == (int num) const
{
//	return mpz_cmp_si(data, num) == 0;
    return 0;
}

bool BigInteger::operator != (const BigInteger &num) const
{
//	return mpz_cmp(data, num.data) != 0;
    return 0;
}

bool BigInteger::operator != (int num) const
{
//	return mpz_cmp_si(data, num) != 0;
    return 0;
}

bool BigInteger::operator < (const BigInteger &num) const
{
//	return mpz_cmp(data, num.data) < 0;
    return 0;
}

bool BigInteger::operator < (int num) const
{
//	return mpz_cmp_si(data, num) < 0;
    return 0;
}

bool BigInteger::operator <= (const BigInteger &num) const
{
//	return mpz_cmp(data, num.data) <= 0;
    return 0;
}

bool BigInteger::operator <= (int num) const
{
//	return mpz_cmp_si(data, num) <= 0;
    return 0;
}

bool BigInteger::operator > (const BigInteger &num) const
{
//	return mpz_cmp(data, num.data) > 0;
    return 0;
}

bool BigInteger::operator > (int num) const
{
//	return mpz_cmp_si(data, num) > 0;
    return 0;
}

bool BigInteger::operator >= (const BigInteger &num) const
{
//	return mpz_cmp(data, num.data) >= 0;
    return 0;
}

bool BigInteger::operator >= (int num) const
{
//	return mpz_cmp_si(data, num) >= 0;
    return 0;
}
