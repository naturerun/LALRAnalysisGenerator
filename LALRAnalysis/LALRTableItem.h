#pragma once
#include "DirectedGraph.h"
#include "LALRTableItem.h"
#include "LALRState.h"
struct LALRTableItem   //LALR语法分析表项
{
	enum action { MOVE, REDUCTION, ACCEPT, ERROR } ActionType;   //该项的语法分析动作
	union
	{
		vector<Graph<LALRState, string>::GraphVertexNode*>::size_type LALRStateNumber;  //该项为移入时应移入的LALR状态
		long production;   //该项为归约时应选则的产生式编号
		string NULLLable;  //该项为报错时表示错误消息的字符串，语法分析程序中为预留，没有填写具体内容
	};
	LALRTableItem() :ActionType(action::ERROR), NULLLable("") {}
	LALRTableItem(action A, long p) :ActionType(A), production(p) {}
	LALRTableItem(action A, vector<Graph<LALRState, string>::GraphVertexNode*>::size_type L) :ActionType(A), LALRStateNumber(L) {}
	LALRTableItem(action A) :NULLLable("") {}
	LALRTableItem(const LALRTableItem& copy);
	~LALRTableItem()
	{
		if (NULLLable != "")
			NULLLable.~string();
	}
};