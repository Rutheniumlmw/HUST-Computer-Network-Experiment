// GBN.cpp : 定义控制台应用程序的入口点。
//
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "GBNSender.h"
#include "GBNReceiver.h"
#include "Check.h"
#include<iostream>

int main(int argc, char* argv[])
{
	RdtSender* ps = new GBNSender();
	RdtReceiver* pr = new GBNReceiver();
	//	pns->setRunMode(0);  //VERBOS模式
	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("C:\\Users\\lianxiang\\Desktop\\input.txt");
	pns->setOutputFile("C:\\Users\\lianxiang\\Desktop\\GBNoutput.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	if (check("C:\\Users\\lianxiang\\Desktop\\input.txt", "C:\\Users\\lianxiang\\Desktop\\output.txt") == 0)
		std::cout << "ERROR" << endl;
	else
		std::cout << "TRUE" << endl;
	return 0;
}