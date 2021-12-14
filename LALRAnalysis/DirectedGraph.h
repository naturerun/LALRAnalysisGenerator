#pragma once
#include <iostream>
#include <vector>
using std::vector;
template <typename V, typename E>  //V,E���붨�忽�����캯���͸��Ƹ�ֵ������Լ���������
class Graph   //����ͼ��,�洢�ṹΪʮ������
{
	friend class RELALRParsing;
public:
	class GraphVertexNode;
	struct GraphEdgeNode    //�߽ڵ���
	{
		typename vector<GraphVertexNode*>::size_type head = 0;  //��ͷ��Ӧ����
		typename vector<GraphVertexNode*>::size_type tail = 0;  //��β��Ӧ����
		E* Edgedatafield = nullptr;   //ָ��߽ڵ��������ָ��
		GraphEdgeNode* sameheadptr = nullptr;   //ָ��ͷ��ͬ����һ���߽ڵ�
		GraphEdgeNode* sametailptr = nullptr;   //ָ��β��ͬ����һ���߽ڵ�

		GraphEdgeNode() = default;
		GraphEdgeNode(typename vector<GraphVertexNode*>::size_type sh, typename vector<GraphVertexNode*>::size_type st, E* Edge) :head(sh), tail(st), Edgedatafield(Edge) {}
		~GraphEdgeNode() { delete Edgedatafield; }
	};

	struct GraphVertexNode
	{
		typename vector<GraphVertexNode*>::size_type number = 0;   //�������
		V* Vertexdatafield = nullptr;   //ָ�򶥵��������ָ��
		GraphEdgeNode* firstheadptr = nullptr;   //ָ���Զ���Ϊ��ͷ�ĵ�һ���߽ڵ�
		GraphEdgeNode* firsttailptr = nullptr;   //ָ���Զ���Ϊ��β�ĵ�һ���߽ڵ�
		GraphVertexNode* seilring = nullptr;     //������������Ի�����ָ��ָ�򶥵㱾��,����Ϊ��
		E* Edgeseilring = nullptr;     //ָ�����ڶ����ϵ��Ի����������ָ��

		GraphVertexNode() = default;
		GraphVertexNode(typename vector<GraphVertexNode*>::size_type num, V* Ver) :Vertexdatafield(Ver), number(num) {}
		~GraphVertexNode() { delete Vertexdatafield; delete Edgeseilring; }
	};

	Graph() = default;
	Graph(typename vector<GraphVertexNode*>::size_type n) :SetOfVertex(n, nullptr) {}   //��SetOfVertex��ʼ��Ϊ��СΪn�Ķ�������,����Ԫ����ָ�򶥵��ָ��Ϊ��
	virtual ~Graph();
	Graph<V, E>* Copy();   //������ΪĿ�Ķ��������ͼ���󣬷���ָ�򿽱��õ�������ͼ��ָ��
	typename vector<typename Graph<V, E>::GraphVertexNode*>::size_type addVertex(V* vertex);   //������ͼ������һ�����㣬�ö���������Ϊvertexָ������,���������Ķ������
	bool addEdge(typename vector<GraphVertexNode*>::size_type vertex1, typename vector<GraphVertexNode*>::size_type vertex2, E* edge);   //������ͼ�������vertex1Ϊ��β,vertex2Ϊ��ͷ�ı�,vertex1,vertex2������ڣ�������ȣ���ӳɹ�����true,���򷵻�false
	Graph<V, E>* merge(Graph<V, E>& Bemerged, bool copyOrNot);   //����ΪĿ�Ķ��������ͼ��Bemerged�ϲ�,copyOrNot=true�򿽱�Ŀ�Ķ��󲢽�Bemerged������Ŀ�Ķ��󸱱��ϲ������غϲ�����ͼָ�룬����Bemerged����ֱ�Ӻϲ���Ŀ�Ķ��󣬷���nullptr
	typename vector<GraphVertexNode*>::size_type getVertexNum() { return SetOfVertex.size(); }   //����ͼ�ж������
	void ReversalGraph();   //����ΪĿ�Ķ��������ͼ��ÿ���߷�ת���򣬼�������ͷ�ͻ�β

protected:
	vector<GraphVertexNode*> SetOfVertex;    //����ͼ���㹹�ɵļ���
};

template <typename V, typename E>
void Graph<V, E>::ReversalGraph()
{
	for (typename vector<GraphVertexNode*>::size_type scan = 0; scan < SetOfVertex.size(); ++scan)
	{
		for (GraphEdgeNode* q = SetOfVertex[scan]->firsttailptr; q != nullptr; )
		{
			swap(q->head, q->tail);
			GraphEdgeNode* temp = q->sametailptr;
			swap(q->sameheadptr, q->sametailptr);
			q = temp;
		}
		swap(SetOfVertex[scan]->firstheadptr, SetOfVertex[scan]->firsttailptr);
	}
}

template <typename V, typename E>
Graph<V, E>* Graph<V, E>::Copy()
{
	Graph<V, E>* temp = new Graph<V, E>(SetOfVertex.size());
	for (typename vector<GraphVertexNode*>::size_type scan = 0; scan < SetOfVertex.size(); ++scan)
	{
		temp->SetOfVertex[scan] = new GraphVertexNode(SetOfVertex[scan]->number, new V(*(SetOfVertex[scan]->Vertexdatafield)));
		if (SetOfVertex[scan]->seilring != nullptr)
		{
			temp->SetOfVertex[scan]->seilring = temp->SetOfVertex[scan];
			temp->SetOfVertex[scan]->Edgeseilring = new E(*(SetOfVertex[scan]->Edgeseilring));
		}
		GraphEdgeNode* p = nullptr;
		for (GraphEdgeNode* q = SetOfVertex[scan]->firsttailptr; q != nullptr; q = q->sametailptr)
		{
			if (p == nullptr)
			{
				p = temp->SetOfVertex[scan]->firsttailptr = new GraphEdgeNode(q->head, q->tail, new E(*(q->Edgedatafield)));
			}
			else
			{
				p = p->sametailptr = new GraphEdgeNode(q->head, q->tail, new E(*(q->Edgedatafield)));
			}
		}
	}

	for (typename vector<GraphVertexNode*>::size_type scan = 0; scan < SetOfVertex.size(); ++scan)
	{
		GraphEdgeNode* p = nullptr;
		for (GraphEdgeNode* q = SetOfVertex[scan]->firstheadptr; q != nullptr; q = q->sameheadptr)
		{
			GraphEdgeNode* m = temp->SetOfVertex[q->tail]->firsttailptr;
			for (; ; m = m->sametailptr)
			{
				if (m->head == scan)
					break;
			}

			if (p == nullptr)
			{
				p = temp->SetOfVertex[scan]->firstheadptr = m;
			}
			else
			{
				p = p->sameheadptr = m;
			}
		}
	}
	return temp;
}

template <typename V, typename E>
Graph<V, E>::~Graph()
{
	for (typename vector<GraphVertexNode*>::size_type scan = 0; scan < SetOfVertex.size(); ++scan)
	{
		GraphEdgeNode* ptr = nullptr;
		while (SetOfVertex[scan]->firsttailptr != nullptr)
		{
			ptr = SetOfVertex[scan]->firsttailptr;
			SetOfVertex[scan]->firsttailptr = ptr->sametailptr;
			SetOfVertex[ptr->head]->firstheadptr = ptr->sameheadptr;
			delete ptr;
		}
	}

	while (SetOfVertex.empty() == false)
	{
		delete SetOfVertex.back();
		SetOfVertex.pop_back();
	}
}

template <typename V, typename E>
typename vector<typename Graph<V, E>::GraphVertexNode*>::size_type Graph<V, E>::addVertex(V* vertex)
{
	SetOfVertex.push_back(new GraphVertexNode(SetOfVertex.size(), vertex));
	return SetOfVertex.size() - 1;
}

template <typename V, typename E>
bool Graph<V, E>::addEdge(typename vector<GraphVertexNode*>::size_type vertex1, typename vector<GraphVertexNode*>::size_type vertex2, E* edge)  //vertex1Ϊ��β,vertex2Ϊ��ͷ
{
	if (vertex1 == vertex2)
	{
		SetOfVertex[vertex1]->seilring = SetOfVertex[vertex1];
		SetOfVertex[vertex1]->Edgeseilring = new E(*edge);
		return true;
	}

	GraphEdgeNode* start = SetOfVertex[vertex1]->firsttailptr;
	GraphEdgeNode* pre = nullptr;
	if (start == nullptr)
	{
		SetOfVertex[vertex1]->firsttailptr = new GraphEdgeNode(vertex2, vertex1, edge);
		if (SetOfVertex[vertex2]->firstheadptr == nullptr)
		{
			SetOfVertex[vertex2]->firstheadptr = SetOfVertex[vertex1]->firsttailptr;
			return true;
		}
	}
	else
	{
		for (; start != nullptr; )
		{
			if (start->head == vertex2)
				return false;
			else if (start->head < vertex2)
			{
				pre = start;
				start = start->sametailptr;
			}
			else
			{
				if (pre == nullptr)
				{
					pre = SetOfVertex[vertex1]->firsttailptr;
					SetOfVertex[vertex1]->firsttailptr = new GraphEdgeNode(vertex2, vertex1, edge);
					SetOfVertex[vertex1]->firsttailptr->sametailptr = pre;
					if (SetOfVertex[vertex2]->firstheadptr == nullptr)
					{
						SetOfVertex[vertex2]->firstheadptr = SetOfVertex[vertex1]->firsttailptr;
						return true;
					}
					pre = SetOfVertex[vertex1]->firsttailptr;
				}
				else
				{
					pre->sametailptr = new GraphEdgeNode(vertex2, vertex1, edge);
					pre->sametailptr->sametailptr = start;
					if (SetOfVertex[vertex2]->firstheadptr == nullptr)
					{
						SetOfVertex[vertex2]->firstheadptr = pre->sametailptr;
						return true;
					}
					pre = pre->sametailptr;
				}
				break;
			}
		}
		if (start == nullptr)
		{
			if (pre->head == vertex2)
				return false;
			pre->sametailptr = new GraphEdgeNode(vertex2, vertex1, edge);
			if (SetOfVertex[vertex2]->firstheadptr == nullptr)
			{
				SetOfVertex[vertex2]->firstheadptr = pre->sametailptr;
				return true;
			}
			pre = pre->sametailptr;
		}
	}

	if (pre == nullptr)
	{
		pre = SetOfVertex[vertex1]->firsttailptr;
	}

	GraphEdgeNode* p = nullptr;
	for (GraphEdgeNode* start = SetOfVertex[vertex2]->firstheadptr; start != nullptr; )
	{
		if (start->tail < vertex1)
		{
			p = start;
			start = start->sameheadptr;
		}
		else
		{
			if (p == nullptr)
			{
				p = SetOfVertex[vertex2]->firstheadptr;
				SetOfVertex[vertex2]->firstheadptr = pre;
				pre->sameheadptr = p;
			}
			else
			{
				p->sameheadptr = pre;
				pre->sameheadptr = start;
			}
			return true;
		}
	}
	p->sameheadptr = pre;
	return true;
}

template <typename V, typename E>
Graph<V, E>* Graph<V, E>::merge(Graph<V, E>& Bemerged, bool copyOrNot/*true����Ŀ�Ķ���,fasle������,ֱ�Ӻϲ���Ŀ�Ķ���*/)
{
	Graph<V, E>* temp1 = nullptr;
	typename vector<GraphVertexNode*>::size_type Ca1 = 0;
	if (copyOrNot)
	{
		temp1 = Copy();
		Ca1 = temp1->SetOfVertex.size();
	}
	else
	{
		Ca1 = SetOfVertex.size();
	}

	{
		Graph<V, E>* temp2 = Bemerged.Copy();
		for (typename vector<GraphVertexNode*>::size_type p = 0; p < temp2->SetOfVertex.size(); ++p)
		{
			if (copyOrNot)
			{
				temp1->SetOfVertex.push_back(temp2->SetOfVertex[p]);
			}
			else
			{
				SetOfVertex.push_back(temp2->SetOfVertex[p]);
			}
		}
		while (temp2->SetOfVertex.empty() == false)
		{
			temp2->SetOfVertex.pop_back();
		}
		delete temp2;
	}

	for (typename vector<GraphVertexNode*>::size_type p = Ca1; ; ++p)
	{
		Graph<V, E>::GraphEdgeNode* q = nullptr;
		if (copyOrNot)
		{
			if (p >= temp1->SetOfVertex.size())
				break;
			temp1->SetOfVertex[p]->number = p;
			q = temp1->SetOfVertex[p]->firsttailptr;
		}
		else
		{
			if (p >= SetOfVertex.size())
				break;
			SetOfVertex[p]->number = p;
			q = SetOfVertex[p]->firsttailptr;
		}

		for (; q != nullptr; q = q->sametailptr)
		{
			q->head = q->head + Ca1;
			q->tail = q->tail + Ca1;
		}
	}

	if (copyOrNot)
		return temp1;
	else
		return nullptr;
}