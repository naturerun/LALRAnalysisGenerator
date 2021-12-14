#pragma once
#include "DirectedGraph.h"
#include "LALRTableItem.h"
#include "LALRState.h"
struct LALRTableItem   //LALR�﷨��������
{
	enum action { MOVE, REDUCTION, ACCEPT, ERROR } ActionType;   //������﷨��������
	union
	{
		vector<Graph<LALRState, string>::GraphVertexNode*>::size_type LALRStateNumber;  //����Ϊ����ʱӦ�����LALR״̬
		long production;   //����Ϊ��ԼʱӦѡ��Ĳ���ʽ���
		string NULLLable;  //����Ϊ����ʱ��ʾ������Ϣ���ַ������﷨����������ΪԤ����û����д��������
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