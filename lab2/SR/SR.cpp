// SR.cpp : �������̨Ӧ�ó������ڵ㡣
//
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "SRSender.h"
#include "SRReceiver.h"
#include "Check.h"

int main3(int argc, char* argv[])
{
	RdtSender* ps = new SRSender();
	RdtReceiver* pr = new SRReceiver();
	//	pns->setRunMode(0);  //VERBOSģʽ
	pns->setRunMode(1);  //����ģʽ
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("C:\\Users\\lianxiang\\Desktop\\input.txt");
	pns->setOutputFile("C:\\Users\\lianxiang\\Desktop\\SRoutput.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	if (check("C:\\Users\\lianxiang\\Desktop\\input.txt", "C:\\Users\\lianxiang\\Desktop\\output.txt") == 0)
		std::cout << "ERROR" << endl;
	else
		std::cout << "TRUE" << endl;
	return 0;
}
