#pragma once
#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include<deque>

FILE* GBN_sender_info = fopen("GBN_sender_info.txt", "w");

class GBNSender :public RdtSender
{

private:
	int expectSequenceNumberSend;	// ��һ��������� 
	bool waitingState;				// �Ƿ��ڵȴ�Ack��״̬
	int base;                       //��ǰ���ڻ����
	int winlen;                     //���ڴ�С
	int seqlen;                     //��ſ��
	deque<Packet> window;           //���ڶ���
	Packet packetWaitingAck;		//�ѷ��Ͳ��ȴ�Ack�����ݰ�

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
