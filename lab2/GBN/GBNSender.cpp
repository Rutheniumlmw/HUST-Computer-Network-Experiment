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
	if (this->getWaitingState()) { //当发送方窗口满时，拒绝发送
		return false;
	}
	this->packetWaitingAck.acknum = -1;
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	window.push_back(packetWaitingAck);
	pUtils->printPacket("发送方发送报文", this->packetWaitingAck);
	fprintf(GBN_sender_info, "发送方发送 seqnum=%d\n", this->packetWaitingAck.seqnum);
	if (this->base == this->expectSequenceNumberSend) 
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base); //启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % this->seqlen;

	return true;

}


void GBNSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int offacknum = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;

	if (checkSum == ackPkt.checksum && offacknum < window.size()) {
		
		fprintf(GBN_sender_info, "发送方滑动窗口之前 base=%d,size=%d \n", this->base, window.size());
		//printf("发送方窗口:[ ");
		fprintf(GBN_sender_info, "发送方窗口:[");
		for (int i = 0; i < this->winlen; i++) {
			printf("%d ", (this->base + i) % this->seqlen);
			fprintf(GBN_sender_info, "%d ", (this->base + i) % this->seqlen);
		}
		//printf("]\n");  //接收ACK前的窗口序列
		fprintf(GBN_sender_info, "]\n");

		pUtils->printPacket("发送方正确收到确认\n", ackPkt);
		fprintf(GBN_sender_info, "发送方正确收到确认 acknum=%d\n", ackPkt.acknum);

		pns->stopTimer(SENDER, this->base);   //由于每次开启的定时器都是以base为基准的，所以以base关闭定时器
		while (this->base != (ackPkt.acknum + 1) % this->seqlen) {//滑动窗口
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}  //将已成功接收的ACK及之前的退出队列，相当于将窗口逐渐滑动到ACK+1的位置
		
		//printf("发送方滑动后窗口:[ ");
		fprintf(GBN_sender_info, "发送方滑动后窗口:[");
		for (int i = 0; i < this->winlen; i++) {
			//printf("%d ", (this->base + i) % this->seqlen);
			fprintf(GBN_sender_info, "%d ", (this->base + i) % this->seqlen);
		}
		//printf("]\n\n");  //接收成功后的窗口值
		fprintf(GBN_sender_info, "]\n");
		if (window.size() != 0) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//以基准包的序号开启计时器
		}
	}
	else if (checkSum != ackPkt.checksum) {
		pUtils->printPacket("发送方没有正确收到该报文确认,数据校验错误", ackPkt);
		fprintf(GBN_sender_info, "发送方没有正确收到该报文确认,数据校验错误\n");
	}
	else {
		pUtils->printPacket("发送方已正确收到过该报文确认", ackPkt);
		fprintf(GBN_sender_info, "发送方已正确收到过该报文确认\n");
	}
		
}


void GBNSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);								//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);	//重新启动发送方定时器
	pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->packetWaitingAck);
	fprintf(GBN_sender_info, "发送方定时器时间到，base=%d,size=%d,\n", this->base, window.size());
	fprintf(GBN_sender_info, "发送方重发窗口报文：[");
	for (int i = 0; i < window.size(); i++) {
		//pUtils->printPacket("发送方定时器时间到，重发窗口报文", window.at(i));
		fprintf(GBN_sender_info, "%d ", window.at(i).seqnum);
		pns->sendToNetworkLayer(RECEIVER, window.at(i));
	}
	fprintf(GBN_sender_info, "]\n");
}