#include "stdafx.h"
#include "Global.h"
#include "TCPSender.h"
#include<deque>


TCPSender::TCPSender() :expectSequenceNumberSend(0), waitingState(false), base(0), winlen(4), seqlen(8), Rdnum(0)
{
}


TCPSender::~TCPSender()
{
}

bool TCPSender::getWaitingState() {
	if (window.size() == winlen)
		this->waitingState = true;
	else this->waitingState = false;
	return this->waitingState;
}

bool TCPSender::send(const Message& message) {
	if (this->getWaitingState()) {
		return false;
	}
	this->packetWaitingAck.acknum = -1; //���Ը��ֶ�
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	window.push_back(packetWaitingAck);               //�������͵İ����봰�ڶ���
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck);
	fprintf(TCP_sender_info, "���ͷ�����seqnum=%d\n", this->packetWaitingAck.seqnum);
	if (this->base == this->expectSequenceNumberSend) {
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck.seqnum);			//�������ͷ���ʱ��
		printf("���ͷ�������ʱ�� seqnum=%d\n", this->packetWaitingAck.seqnum);
	}
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % this->seqlen;

	return true;
}

void TCPSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int offacknum = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;

	if (checkSum == ackPkt.checksum && offacknum < window.size()) {
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		fprintf(TCP_sender_info, "���ͷ���������֮ǰ base=%d,size=%d \n", this->base, window.size());
		fprintf(TCP_sender_info, "���ͷ�����:[");
		for (int i = 0; i < this->winlen; i++) {
			printf("%d ", (this->base + i) % this->seqlen);
			fprintf(TCP_sender_info, "%d ", (this->base + i) % this->seqlen);
		}
		fprintf(TCP_sender_info, "]\n");
		/*
		while (this->base != (ackPkt.acknum + 1) % this->seqlen) {//��������
			pns->stopTimer(SENDER, this->base);
			fprintf(TCP_sender_info, "�رշ��ͷ���ʱ�� seqnum=%d\n", this->base);
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}
		*/
		pns->stopTimer(SENDER, this->base);   //����ÿ�ο����Ķ�ʱ��������baseΪ��׼�ģ�������base�رն�ʱ��
		while (this->base != (ackPkt.acknum + 1) % this->seqlen) {//��������
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}  //���ѳɹ����յ�ACK��֮ǰ���˳����У��൱�ڽ������𽥻�����ACK+1��λ��

		fprintf(TCP_sender_info, "���ͷ������󴰿�:[");
		for (int i = 0; i < this->winlen; i++) {
			//printf("%d ", (this->base + i) % this->seqlen);
			fprintf(TCP_sender_info, "%d ", (this->base + i) % this->seqlen);
		}
		fprintf(TCP_sender_info, "]\n");

		this->Rdnum = 0;  //�յ���ȷ��ʱ���������������
		if (window.size() != 0) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//�Ի�׼������ſ�����ʱ��
			printf("�������ͷ���ʱ��seqnum=%d\n", window.front().seqnum);
		}

	}
	else if (checkSum != ackPkt.checksum) {
		pUtils->printPacket("���ͷ�û����ȷ�յ��ñ���ȷ��,����У�����", ackPkt);
	}
		
	else if(ackPkt.acknum == (this->base - 1 + this->seqlen) % this->seqlen){
		pUtils->printPacket("���ͷ�����ȷ�յ����ñ���ȷ��", ackPkt);
		this->Rdnum ++ ;
		if (this->Rdnum == 3 && window.size() > 0) {
			pUtils->printPacket("���ͷ����������ش����ƣ��ش����緢����û��ȷ�ϵı��Ķ�", window.front());
			fprintf(TCP_sender_info, "�����ش� seqnum=%d\n", window.front().seqnum);
			pns->sendToNetworkLayer(RECEIVER, window.front());
			this->Rdnum = 0;
		}
	}
}

void TCPSender::timeoutHandler(int seqNum) {
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", window.front());
	fprintf(TCP_sender_info, "���ͷ���ʱ��ʱ�䵽 base=%d size=%d,\n", this->base, window.size());
	//���·������ݰ�
	pns->stopTimer(SENDER, window.front().seqnum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, window.front().seqnum);			//�����������ͷ���ʱ��
	fprintf(TCP_sender_info, "�����������ͷ���ʱ�� seqnum=%d\n", window.front().seqnum);
	pns->sendToNetworkLayer(RECEIVER, window.front());
}