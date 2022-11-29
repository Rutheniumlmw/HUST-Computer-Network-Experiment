
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
	if (this->getWaitingState()) { //当发送方窗口满时，拒绝发送
		return false;
	}

	this->packetWaitingAck.acknum = -1; //忽略该字段
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	waitPck tempPck;
	tempPck.flag = false;
	tempPck.winPck = packetWaitingAck;
	window.push_back(tempPck);          //将发送且待确认的包加入窗口队列
	pUtils->printPacket("发送方发送报文", this->packetWaitingAck);
	fprintf(SR_sender_info, "发送方发送 seqnum=%d\n", this->packetWaitingAck.seqnum);

	pns->startTimer(SENDER, Configuration::TIME_OUT, this->expectSequenceNumberSend);			//启动发送方定时器，此时没发送一个包启动对应的定时器
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	this->expectSequenceNumberSend = (this->expectSequenceNumberSend + 1) % this->seqlen;

	return true;
}

void SRSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int offseqnum = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;

	if (checkSum == ackPkt.checksum && (offseqnum < window.size()) && window.at(offseqnum).flag == false) {
		window.at(offseqnum).flag = true; //标记收到
		pns->stopTimer(SENDER, ackPkt.acknum); //关掉定时器

		pUtils->printPacket("发送方正确收到确认", ackPkt);
		fprintf(SR_sender_info, "发送方正确收到确认 acknum=%d\n", ackPkt.acknum);//定向到文件
		fprintf(SR_sender_info, "发送方滑动窗口之前 base=%d,size=%d,\n", this->base, window.size());
		fprintf(SR_sender_info, "发送方窗口：[");
		//printf("发送方窗口：[");
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
		fprintf(SR_sender_info, "]\n");  //接收ACK前的窗口序列
		//printf("]\n"); 
		//pUtils->printPacket("发送方正确收到确认", ackPkt);

		while (window.size() != 0 && window.front().flag == true) {//滑动窗口
			window.pop_front();
			this->base = (this->base + 1) % this->seqlen;
		}  //将窗口滑动至没有收到报文ACK的位置

		fprintf(SR_sender_info, "发送方滑动后窗口:[ ");
		//printf("发送方滑动后窗口:[ ");
		for (int i = 0; i < this->winlen; i++) {
			if (i < window.size()) {
				if (window.at(i).flag == true)
					fprintf(SR_sender_info, "%dY ", (this->base + i) % seqlen);
				else fprintf(SR_sender_info, "%dN ", (this->base + i) % seqlen);
			}
			else fprintf(SR_sender_info, "%d ", (this->base + i) % seqlen);
		}
		fprintf(SR_sender_info, "]\n");  //接收ACK后的窗口序列
	}
	else if (checkSum != ackPkt.checksum) {
		pUtils->printPacket("发送方没有正确收到该报文确认,数据校验错误", ackPkt);
		fprintf(SR_sender_info, "发送方没有正确收到该报文确认,数据校验错误\n");
	}
		
	else {
		pUtils->printPacket("发送方已正确收到过该报文确认", ackPkt);
		fprintf(SR_sender_info, "发送方已正确收到过该报文确认 acknum=%d \n", ackPkt.acknum);
	}
		
		
}

void SRSender::timeoutHandler(int seqNum) {
	int offseqnum = (seqNum - this->base + this->seqlen) % this->seqlen;
	pns->stopTimer(SENDER, seqNum);  									//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, window.at(offseqnum).winPck);
	pUtils->printPacket("发送方定时器时间到，重发报文", window.at(offseqnum).winPck);
	fprintf(SR_sender_info, "发送方定时器时间到，重发报文 seqnum=%d，base=%d\n", seqNum, this->base);
	/*pns->stopTimer(SENDER, seqNum);
	int offset = (seqNum - this->base + this->seqlen) % this->seqlen;
	//重新发送数据包
	if (offset < window.size()) {
		pUtils->printPacket("发送方定时器时间到，重发报文", window.at(offset).winPck);
		fprintf(SR_sender_info, "发送方定时器时间到，重发报文seqnum=%d，base=%d\n", seqNum, this->base);
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
		pns->sendToNetworkLayer(RECEIVER, window.at(offset).winPck);
	}
	else {
		pUtils->printPacket("发送方定时器时间到，该报文已得到确认", window.at(offset).winPck);
		fprintf(SR_sender_info, "发送方定时器时间到，发送方定时器时间到，该报文已得到确认seqnum=%d，base=%d\n", seqNum, this->base);
	}*/
}