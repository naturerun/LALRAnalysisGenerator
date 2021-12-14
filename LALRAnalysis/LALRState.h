#pragma once
#include "pch.h"
#include <set>
#include <string>
#include <map>
using namespace std;
struct LALRState   //LALR״̬
{
	struct attribute
	{
		int dotposition;  //ÿһ������ʽ���λ��
		set<string> ForwardLookingSign;  //����ʽ��ǰ�����ż���
		attribute() = default;
		attribute(int dot) : dotposition(dot) {}
	};

	map<long, map<int, set<string>>> kernel;  //�ں���ϣ���Ϊ����ʽ���,ֵ�����λ�ã�ֵ��ǰ�����ż���
	map<long, attribute> nonkernel;  //���ں����, ��Ϊ����ʽ���
};