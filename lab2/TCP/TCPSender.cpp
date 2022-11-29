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
	this->packetWaitingAck.acknum = -1; //忽略该字段
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	window.push_back(packetWaitingAck);               //将待发送的包加入窗口队列
	pUtils->printPacket("发送方发送报文", this->packetWaitingAck);
	fprintf(TCP_sender_info, "发送方发送seqnum=%d\n", this->packetWaitingAck.seqnum);
	if (this->base == this->expectSequenceNumberSend) {
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck.seqnum);			//启动发送方定时器
		printf("发送方启动定时器 seqnum=%d\n", this->packetWaitingAck.seqnum);
	}
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % this->seqlen;

	return true;
}

void TCPSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int offacknum = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;

	if (checkSum == ackPkt.checksum && offacknum < window.size()) {
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		fprintf(TCP_sender_info, "发送方滑动窗口之前 base=%d,size=%d \n", this->base, window.size());
		fprintf(TCP_sender_info, "发送方窗口:[");
		for (int i = 0; i < this->winlen; i++) {
			printf("%d ", (this->base + i) % this->seqlen);
			fprintf(TCP_sender_info, "%d ", (this->base + i) % this->seqlen);
		}
		fprintf(TCP_sender_info, "]\n");
		/*
		while (this->base != (ackPkt.acknum + 1) % this->seqlen) {//滑动窗口
			pns->stopTimer(SENDER, this->base);
			fprintf(TCP_sender_info, "关闭发送方定时器 seqnum=%d\n", this->base);
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}
		*/
		pns->stopTimer(SENDER, this->base);   //由于每次开启的定时器都是以base为基准的，所以以base关闭定时器
		while (this->base != (ackPkt.acknum + 1) % this->seqlen) {//滑动窗口
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}  //将已成功接收的ACK及之前的退出队列，相当于将窗口逐渐滑动到ACK+1的位置

		fprintf(TCP_sender_info, "发送方滑动后窗口:[");
		for (int i = 0; i < this->winlen; i++) {
			//printf("%d ", (this->base + i) % this->seqlen);
			fprintf(TCP_sender_info, "%d ", (this->base + i) % this->seqlen);
		}
		fprintf(TCP_sender_info, "]\n");

		this->Rdnum = 0;  //收到正确包时，将冗余记数重置
		if (window.size() != 0) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);//以基准包的序号开启计时器
			printf("启动发送方定时器seqnum=%d\n", window.front().seqnum);
		}

	}
	else if (checkSum != ackPkt.checksum) {
		pUtils->printPacket("发送方没有正确收到该报文确认,数据校验错误", ackPkt);
	}
		
	else if(ackPkt.acknum == (this->base - 1 + this->seqlen) % this->seqlen){
		pUtils->printPacket("发送方已正确收到过该报文确认", ackPkt);
		this->Rdnum ++ ;
		if (this->Rdnum == 3 && window.size() > 0) {
			pUtils->printPacket("发送方启动快速重传机制，重传最早发送且没被确认的报文段", window.front());
			fprintf(TCP_sender_info, "快速重传 seqnum=%d\n", window.front().seqnum);
			pns->sendToNetworkLayer(RECEIVER, window.front());
			this->Rdnum = 0;
		}
	}
}

void TCPSender::timeoutHandler(int seqNum) {
	pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", window.front());
	fprintf(TCP_sender_info, "发送方定时器时间到 base=%d size=%d,\n", this->base, window.size());
	//重新发送数据包
	pns->stopTimer(SENDER, window.front().seqnum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, window.front().seqnum);			//重新启动发送方定时器
	fprintf(TCP_sender_info, "重新启动发送方定时器 seqnum=%d\n", window.front().seqnum);
	pns->sendToNetworkLayer(RECEIVER, window.front());
}