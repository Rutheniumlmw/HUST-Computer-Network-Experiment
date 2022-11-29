#include "stdafx.h"
#include "Global.h"
#include "GBNReceiver.h"


GBNReceiver::GBNReceiver() :expectSequenceNumberRcvd(0), seqlen(8)
{
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE;i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNReceiver::~GBNReceiver()
{
}

void GBNReceiver::receive(const Packet& packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum && this->expectSequenceNumberRcvd == packet.seqnum) {
		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
		fprintf(GBN_rcver_info, "���շ���ȷ�յ����ͷ��ı��� sequm=%d\n", packet.seqnum);
		//ȡ��Message�����ϵݽ���Ӧ�ò�
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		fprintf(GBN_rcver_info, "���շ�����ȷ�ϱ��� acknum=%d\n", lastAckPkt.acknum);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�

		this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % this->seqlen; //���������0-7֮���л�
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
			fprintf(GBN_rcver_info, "���շ�û����ȷ�յ����ͷ��ı���,����У�����\n");
		}
		else {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			fprintf(GBN_rcver_info, "���շ�û����ȷ�յ����ͷ��ı���,������Ų���\n");
		}
		pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
		fprintf(GBN_rcver_info, "���շ����·����ϴε�ȷ�ϱ��� acknum=%d\n", lastAckPkt.acknum);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���

	}
}