#pragma once
#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"

FILE* GBN_rcver_info = fopen("GBN_rcver_info.txt", "w");

class GBNReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	int seqlen;                     //��ſ��
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���

public:
	GBNReceiver();
	virtual ~GBNReceiver();

public:
	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����
};

#endif