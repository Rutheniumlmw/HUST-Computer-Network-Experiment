#pragma once
#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include<deque>

FILE* GBN_sender_info = fopen("GBN_sender_info.txt", "w");

class GBNSender :public RdtSender
{

private:
	int expectSequenceNumberSend;	// 下一个发送序号 
	bool waitingState;				// 是否处于等待Ack的状态
	int base;                       //当前窗口基序号
	int winlen;                     //窗口大小
	int seqlen;                     //序号宽度
	deque<Packet> window;           //窗口队列
	Packet packetWaitingAck;		//已发送并等待Ack的数据包

public:
	bool send(const Message& message);
	void receive(const Packet& ackPkt);
	void timeoutHandler(int seqNum);
	bool getWaitingState();

public:
	GBNSender();
	virtual ~GBNSender();
};
#endif
