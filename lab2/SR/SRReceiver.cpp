#include "stdafx.h"
#include "Global.h"
#include "SRReceiver.h"


SRReceiver::SRReceiver() :expectSequenceNumberRcvd(0), seqlen(8), base(0), winlen(4)
{
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE;i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for (int i = 0; i < winlen; i++) {
		rcvPck blank;
		blank.flag = false;
		blank.winPck.seqnum = -1;
		window.push_back(blank);
	}                                //����������������Ϊ�հ���flag��Ϊfalse�������������

}

SRReceiver::~SRReceiver()
{
}

void SRReceiver::receive(const Packet& packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);
	int offseqnum = (packet.seqnum - this->base + this->seqlen) % this->seqlen;

	//���У�����ȷ��ͬʱ�յ����ĵ�����ڽ��մ��ڷ�Χ��
	if (checkSum == packet.checksum && offseqnum < this->winlen && window.at(offseqnum).flag == false) {
		window.at(offseqnum).flag = true;
		window.at(offseqnum).winPck = packet;
		fprintf(SR_rcver_info, "���շ� base=%d, offseqnum=%d\n", this->base, offseqnum);
		//���ô���״̬����Ϊtrue��ռ��
		fprintf(SR_rcver_info, "���շ�����:[ ");
		for (int i = 0; i < (signed)window.size(); i++) {
			if (window.at(i).flag == true)
				fprintf(SR_rcver_info, "%dY ", (this->base + i) % seqlen);
			else 
				fprintf(SR_rcver_info, "%dN ", (this->base + i) % seqlen);
		}
		fprintf(SR_rcver_info, "]\n");  //���յ�����ǰ�Ĵ�������

		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
		fprintf(SR_rcver_info, "���շ���ȷ�յ����ͷ��ı���\n");
		while (window.front().flag == true) {
			Message msg;
			memcpy(msg.data, window.front().winPck.payload, sizeof(window.front().winPck.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			this->base = (this->base + 1) % seqlen;
			rcvPck blank;
			blank.flag = false;
			blank.winPck.seqnum = -1;
			window.pop_front();
			window.push_back(blank); //�������һ���һ��
		}

		fprintf(SR_rcver_info, "���շ������󴰿�:[ ");
		for (int i = 0; i < (signed)window.size(); i++) {
			if (window.at(i).flag == true)
				printf("%dY ", (this->base + i) % seqlen);
			else printf("%dN ", (this->base + i) % seqlen);
		}
		fprintf(SR_rcver_info, "]\n");  //���ձ��ĺ�Ĵ�������

		lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		fprintf(SR_rcver_info, "���շ�����ȷ�ϱ���\n");
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�


	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
			fprintf(SR_rcver_info, "���շ�û����ȷ�յ����ͷ��ı���,����У�����\n");
		}
		else {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������ȷ��", packet);
			fprintf(SR_rcver_info, "���շ�û����ȷ�յ����ͷ��ı���,������ȷ��\n");
			lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
			fprintf(SR_rcver_info, "���շ����·����ϴε�ȷ�ϱ���\n");
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢����ȷ�ϱ��ĵ�ACK
		}
	}
}
