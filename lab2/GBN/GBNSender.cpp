#include "stdafx.h"
#include "Global.h"
#include "GBNSender.h"
#include<deque>


GBNSender::GBNSender() :expectSequenceNumberSend(0), waitingState(false), base(0), winlen(4), seqlen(8)
{
}

GBNSender::~GBNSender()
{
}

bool GBNSender::getWaitingState() {
	if (window.size() == winlen)
		this->waitingState = 1;
	else this->waitingState = 0;
	return this->waitingState;
} 
bool GBNSender::send(const Message& message) {
	if (this->getWaitingState()) { //�����ͷ�������ʱ���ܾ�����
		return false;
	}
	this->packetWaitingAck.acknum = -1;
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	window.push_back(packetWaitingAck);
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck);
	fprintf(GBN_sender_info, "���ͷ����� seqnum=%d\n", this->packetWaitingAck.seqnum);
	if (this->base == this->expectSequenceNumberSend) 
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base); //�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % this->seqlen;

	return true;

}


void GBNSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int offacknum = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;

	if (checkSum == ackPkt.checksum && offacknum < window.size()) {
		
		fprintf(GBN_sender_info, "���ͷ���������֮ǰ base=%d,size=%d \n", this->base, window.size());
		//printf("���ͷ�����:[ ");
		fprintf(GBN_sender_info, "���ͷ�����:[");
		for (int i = 0; i < this->winlen; i++) {
			printf("%d ", (this->base + i) % this->seqlen);
			fprintf(GBN_sender_info, "%d ", (this->base + i) % this->seqlen);
		}
		//printf("]\n");  //����ACKǰ�Ĵ�������
		fprintf(GBN_sender_info, "]\n");

		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��\n", ackPkt);
		fprintf(GBN_sender_info, "���ͷ���ȷ�յ�ȷ�� acknum=%d\n", ackPkt.acknum);

		pns->stopTimer(SENDER, this->base);   //����ÿ�ο����Ķ�ʱ��������baseΪ��׼�ģ�������base�رն�ʱ��
		while (this->base != (ackPkt.acknum + 1) % this->seqlen) {//��������
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}  //���ѳɹ����յ�ACK��֮ǰ���˳����У��൱�ڽ������𽥻�����ACK+1��λ��
		
		//printf("���ͷ������󴰿�:[ ");
		fprintf(GBN_sender_info, "���ͷ������󴰿�:[");
		for (int i = 0; i < this->winlen; i++) {
			//printf("%d ", (this->base + i) % this->seqlen);
			fprintf(GBN_sender_info, "%d ", (this->base + i) % this->seqlen);
		}
		//printf("]\n\n");  //���ճɹ���Ĵ���ֵ
		fprintf(GBN_sender_info, "]\n");
		if (window.size() != 0) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//�Ի�׼������ſ�����ʱ��
		}
	}
	else if (checkSum != ackPkt.checksum) {
		pUtils->printPacket("���ͷ�û����ȷ�յ��ñ���ȷ��,����У�����", ackPkt);
		fprintf(GBN_sender_info, "���ͷ�û����ȷ�յ��ñ���ȷ��,����У�����\n");
	}
	else {
		pUtils->printPacket("���ͷ�����ȷ�յ����ñ���ȷ��", ackPkt);
		fprintf(GBN_sender_info, "���ͷ�����ȷ�յ����ñ���ȷ��\n");
	}
		
}


void GBNSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);								//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);	//�����������ͷ���ʱ��
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->packetWaitingAck);
	fprintf(GBN_sender_info, "���ͷ���ʱ��ʱ�䵽��base=%d,size=%d,\n", this->base, window.size());
	fprintf(GBN_sender_info, "���ͷ��ط����ڱ��ģ�[");
	for (int i = 0; i < window.size(); i++) {
		//pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط����ڱ���", window.at(i));
		fprintf(GBN_sender_info, "%d ", window.at(i).seqnum);
		pns->sendToNetworkLayer(RECEIVER, window.at(i));
	}
	fprintf(GBN_sender_info, "]\n");
}