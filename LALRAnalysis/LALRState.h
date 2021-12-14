#pragma once
#include "pch.h"
#include <set>
#include <string>
#include <map>
using namespace std;
struct LALRState   //LALR状态
{
	struct attribute
	{
		int dotposition;  //每一个产生式点号位置
		set<string> ForwardLookingSign;  //产生式向前看符号集合
		attribute() = default;
		attribute(int dot) : dotposition(dot) {}
	};

	map<long, map<int, set<string>>> kernel;  //内核项集合，键为产生式编号,值键点号位置，值向前看符号集合
	map<long, attribute> nonkernel;  //非内核项集合, 键为产生式编号
};