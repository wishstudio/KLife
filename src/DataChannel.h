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

#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#define BUFFER_SIZE 65536

#define DATACHANNEL_EOF    -2
#define DATACHANNEL_EOLN   -1

class BigInteger;
class DataChannel;

class DataSender
{
public:
	virtual ~DataSender() {}
	virtual void send(DataChannel *channel) = 0;
};

class DataReceiver
{
public:
	virtual ~DataReceiver() {}
	virtual void setReceiveRect(const BigInteger &x, const BigInteger &y, quint64 w, quint64 h) = 0;
	virtual void receive(DataChannel *channel) = 0;
};

class DataChannel: private QThread
{
public:
	DataChannel();

	static void transfer(DataSender *sender, DataReceiver *receiver);
	static DataChannel *transferFrom(DataSender *sender);
	static DataChannel *transferTo(DataReceiver *receiver);

	void send(int state, quint64 cnt);
	void receive(int *state, quint64 *cnt);

private:
	void lock();
	void unlock();
	void prepareTransfer(DataSender *sender, DataReceiver *receiver);
	void run();

	QAtomicInt m_idle;
	QMutex mutex;
	QWaitCondition bufReadOut, bufFillIn;
	DataSender *m_sender;
	DataReceiver *m_receiver;
	volatile int flipSend, flipReceive;
	volatile int bufState[2][BUFFER_SIZE];
	volatile quint64 bufCnt[2][BUFFER_SIZE];
	volatile int bufPos, bufLen[2];
	volatile bool sendEnd, receiveEnd;
};

#endif
