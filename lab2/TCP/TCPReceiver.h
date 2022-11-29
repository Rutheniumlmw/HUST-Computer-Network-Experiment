#pragma once
#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"

FILE* TCP_rcver_info = fopen("TCP_rcver_info.txt", "w");
class TCPReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	int seqlen;                     //��ſ��
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���

public:
	TCPReceiver();
	virtual ~TCPReceiver();

public:

	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����
};

#endif