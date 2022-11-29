
#include "stdafx.h"
#include "Global.h"
#include "SRSender.h"


SRSender::SRSender() :expectSequenceNumberSend(0), waitingState(false), base(0), winlen(4), seqlen(8)
{
}


SRSender::~SRSender()
{
}



bool SRSender::getWaitingState() {
	if (window.size() == winlen)
		this->waitingState = 1;
	else this->waitingState = 0;
	return this->waitingState;
}


bool SRSender::send(const Message& message) {
	if (this->getWaitingState()) { //�����ͷ�������ʱ���ܾ�����
		return false;
	}

	this->packetWaitingAck.acknum = -1; //���Ը��ֶ�
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	waitPck tempPck;
	tempPck.flag = false;
	tempPck.winPck = packetWaitingAck;
	window.push_back(tempPck);          //�������Ҵ�ȷ�ϵİ����봰�ڶ���
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck);
	fprintf(SR_sender_info, "���ͷ����� seqnum=%d\n", this->packetWaitingAck.seqnum);

	pns->startTimer(SENDER, Configuration::TIME_OUT, this->expectSequenceNumberSend);			//�������ͷ���ʱ������ʱû����һ����������Ӧ�Ķ�ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % this->seqlen;

	return true;
}

void SRSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int offseqnum = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;

	if (checkSum == ackPkt.checksum && (offseqnum < window.size()) && window.at(offseqnum).flag == false) {
		window.at(offseqnum).flag = true; //����յ�
		pns->stopTimer(SENDER, ackPkt.acknum); //�ص���ʱ��

		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		fprintf(SR_sender_info, "���ͷ���ȷ�յ�ȷ�� acknum=%d\n", ackPkt.acknum);//�����ļ�
		fprintf(SR_sender_info, "���ͷ���������֮ǰ base=%d,size=%d,\n", this->base, window.size());
		fprintf(SR_sender_info, "���ͷ����ڣ�[");
		//printf("���ͷ����ڣ�[");
		for (int i = 0; i < this->winlen; i++) {
			if (i < window.size()) {
				if (window.at(i).flag == true) {
					fprintf(SR_sender_info, "%dY ", (this->base + i) % seqlen);
					//printf("%dY ", (this->base + i) % seqlen);
				}
				else {
					fprintf(SR_sender_info, "%dN ", (this->base + i) % seqlen);
					//printf("%dN ", (this->base + i) % seqlen);
				}
					
			}
			else {
				fprintf(SR_sender_info, "%d ", (this->base + i) % seqlen);
				//printf("%d ", (this->base + i) % seqlen);
			}
		}
		fprintf(SR_sender_info, "]\n");  //����ACKǰ�Ĵ�������
		//printf("]\n"); 
		//pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);

		while (window.size() != 0 && window.front().flag == true) {//��������
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}  //�����ڻ�����û���յ�����ACK��λ��

		fprintf(SR_sender_info, "���ͷ������󴰿�:[ ");
		//printf("���ͷ������󴰿�:[ ");
		for (int i = 0; i < this->winlen; i++) {
			if (i < window.size()) {
				if (window.at(i).flag == true)
					fprintf(SR_sender_info, "%dY ", (this->base + i) % seqlen);
				else fprintf(SR_sender_info, "%dN ", (this->base + i) % seqlen);
			}
			else fprintf(SR_sender_info, "%d ", (this->base + i) % seqlen);
		}
		fprintf(SR_sender_info, "]\n");  //����ACK��Ĵ�������
	}
	else if (checkSum != ackPkt.checksum) {
		pUtils->printPacket("���ͷ�û����ȷ�յ��ñ���ȷ��,����У�����", ackPkt);
		fprintf(SR_sender_info, "���ͷ�û����ȷ�յ��ñ���ȷ��,����У�����\n");
	}
		
	else {
		pUtils->printPacket("���ͷ�����ȷ�յ����ñ���ȷ��", ackPkt);
		fprintf(SR_sender_info, "���ͷ�����ȷ�յ����ñ���ȷ�� acknum=%d \n", ackPkt.acknum);
	}
		
		
}

void SRSender::timeoutHandler(int seqNum) {
	int offseqnum = (seqNum - this->base + this->seqlen) % this->seqlen;
	pns->stopTimer(SENDER, seqNum);  									//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, window.at(offseqnum).winPck);
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط�����", window.at(offseqnum).winPck);
	fprintf(SR_sender_info, "���ͷ���ʱ��ʱ�䵽���ط����� seqnum=%d��base=%d\n", seqNum, this->base);
	/*pns->stopTimer(SENDER, seqNum);
	int offset = (seqNum - this->base + this->seqlen) % this->seqlen;
	//���·������ݰ�
	if (offset < window.size()) {
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط�����", window.at(offset).winPck);
		fprintf(SR_sender_info, "���ͷ���ʱ��ʱ�䵽���ط�����seqnum=%d��base=%d\n", seqNum, this->base);
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
		pns->sendToNetworkLayer(RECEIVER, window.at(offset).winPck);
	}
	else {
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ñ����ѵõ�ȷ��", window.at(offset).winPck);
		fprintf(SR_sender_info, "���ͷ���ʱ��ʱ�䵽�����ͷ���ʱ��ʱ�䵽���ñ����ѵõ�ȷ��seqnum=%d��base=%d\n", seqNum, this->base);
	}*/
}