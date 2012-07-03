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

#include "DataChannel.h"

Q_GLOBAL_STATIC(DataChannel, globalDataChannel)

DataChannel::DataChannel()
	: m_idle(true)
{
}

void DataChannel::transfer(DataSender *sender, DataReceiver *receiver)
{
    globalDataChannel()->prepareTransfer(sender, receiver);
    globalDataChannel()->start();
    sender->send(globalDataChannel());
}

DataChannel *DataChannel::transferTo(DataReceiver *receiver)
{
    globalDataChannel()->prepareTransfer(NULL, receiver);
    globalDataChannel()->start();
    return globalDataChannel();
}

DataChannel *DataChannel::transferFrom(DataSender *sender)
{
    globalDataChannel()->prepareTransfer(sender, NULL);
    globalDataChannel()->start();
    return globalDataChannel();
}

void DataChannel::send(int state, quint64 cnt)
{
	if (sendEnd)
		qFatal("DataChannel::send(): data is already ended.");
	if (bufLen[flipSend] == BUFFER_SIZE)
	{
		mutex.lock();
		flipSend = !flipSend;
		if (flipSend == flipReceive)
			bufReadOut.wait(&mutex);
		else
			bufFillIn.wakeAll();
		bufLen[flipSend] = 0;
		mutex.unlock();
	}
	bufState[flipSend][bufLen[flipSend]] = state;
	bufCnt[flipSend][bufLen[flipSend]] = cnt;
	bufLen[flipSend]++;
	if (state == DATACHANNEL_EOF)
	{
		mutex.lock();
		flipSend = !flipSend;
		if (flipSend == flipReceive)
			bufReadOut.wait(&mutex);
		else
			bufFillIn.wakeAll();
		mutex.unlock();
		if (m_receiver) // Main thread is sender
		{
			wait();
			return;
		}
		sendEnd = true;
	}
}

void DataChannel::receive(int *state, quint64 *cnt)
{
	if (receiveEnd)
		qFatal("DataChannel::receive(): End of data!");
	if (bufPos == bufLen[flipReceive])
	{
		mutex.lock();
		flipReceive = !flipReceive;
		if (flipSend == flipReceive)
			bufFillIn.wait(&mutex);
		else
			bufReadOut.wakeAll();
		bufPos = 0;
		mutex.unlock();
	}
	*state = bufState[flipReceive][bufPos];
	*cnt = bufCnt[flipReceive][bufPos];
	if (*state == DATACHANNEL_EOF)
	{
		if (!m_receiver) // Main thread is receiver
		{
			wait();
			return;
		}
		receiveEnd = true;
	}
	bufPos++;
}

void DataChannel::lock()
{
	if (!m_idle.fetchAndStoreAcquire(false))
		qFatal("DataChannel::lock(): DataChannel is in use!");
}

void DataChannel::unlock()
{
	m_idle.fetchAndStoreRelease(true);
}

void DataChannel::prepareTransfer(DataSender *sender, DataReceiver *receiver)
{
    globalDataChannel()->lock();
	m_sender = sender;
	m_receiver = receiver;
	bufPos = bufLen[0] = bufLen[1] = 0;
	flipSend = 0;
	flipReceive = 1;
	sendEnd = receiveEnd = false;
}

void DataChannel::run()
{
	if (m_receiver)
		m_receiver->receive(this);
	else
		m_sender->send(this);
	unlock();
}
