
#define _CRT_SECURE_NO_WARNINGS
#pragma once


#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include<deque>

FILE* SR_rcver_info = fopen("SR_rcver_info.txt", "w");

struct rcvPck {
	bool flag;       //ָʾ��λ���Ƿ�ռ�ã�ture��ʾռ��
	Packet winPck;   //������ݰ�
};

class SRReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	int base;                       //��ǰ���ڻ����
	int winlen;                     //���ڴ�С
	int seqlen;                     //��ſ��
	deque<rcvPck> window;           //��������
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���

public:
	SRReceiver();
	virtual ~SRReceiver();

public:

	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����
};


#endif

