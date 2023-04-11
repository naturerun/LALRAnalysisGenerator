#include "pch.h"
#include <fstream>
#include "LALRAutomata.h"
#include "assistfunction.h"
#include "Priority_Queue.h"
void LALRAutomata::initgrammar(ifstream& input)  //���ݴ�input������ķ���Ϣ��ʼ��Nonterminal��terminnal��StartSymbol��AugGraSS��productionSet�Լ�TerToPro
{
	enum T { TERMINAL, NONTERMINAL } flag;
	string current;
	string outerflag;
	bool TF = true;
	map<long, tuple<string, vector<ProductionBodySymbol>, set<string>>> ::iterator itp;
	map<string, set<long>>::iterator itT;
	while (input >> current)
	{
		if (current == "#1b" || current == "#2b" || current == "#3b" || current == "#4b")
		{
			outerflag = current;
			if (current == "#3b")
			{
				TF = true;
			}
		}
		else if (current == "#1e" || current == "#2e" || current == "#3e" || current == "#4e")
		{
			continue;
		}
		else if (current == "#b" || current == "$1")
		{
			if (current == "#b")
				TF = true;
			flag = NONTERMINAL;
		}
		else if (current == "#e")
		{
			continue;
		}
		else if (current == "$2")
		{
			flag = TERMINAL;
		}
		else
		{
			if (outerflag == "#4b")
			{
				if (flag == NONTERMINAL)
				{
					if (TF == true)
					{
						TF = false;
						if (productionSet.empty())
						{
							itp = productionSet.insert(make_pair(1, tuple<string, vector<ProductionBodySymbol>, set<string>>(current, vector<ProductionBodySymbol>(), set<string>()))).first;
							TerToPro.insert(make_pair(current, set<long>())).first->second.insert(1);
						}
						else
						{
							itp = productionSet.insert(make_pair(itp->first + 1, tuple<string, vector<ProductionBodySymbol>, set<string>>(current, vector<ProductionBodySymbol>(), set<string>()))).first;
							TerToPro.insert(make_pair(current, set<long>())).first->second.insert(itp->first);
						}
					}
					else
					{
						get<1>(itp->second).push_back(ProductionBodySymbol(current, false));
						get<2>(itp->second).insert(current);
					}
				}
				else
				{
					get<1>(itp->second).push_back(ProductionBodySymbol(current, true));
				}
			}
			else if (outerflag == "#3b")
			{
				if (TF)
				{
					TF = false;
					AugGraSS = current;
				}
				else
				{
					StartSymbol = current;
				}
			}
			else if (outerflag == "#2b")
			{
				terminnal.insert(current);
			}
			else
			{
				Nonterminal.insert(current);
			}
		}
	}
}

LALRAutomata::LALRAutomata(ifstream& input, size_t k)
{
	cout << "��ʼ����LALR(1)�﷨������" << endl;;
	clock_t start = clock();
	initgrammar(input);
	first = calculateFirst();  //����first��
	follow = calculateFollow();   //����follow��
	firstK = calculateFirstK(k);
	followK = calculateFollowK(k);
	constructLRKernel();
	LALRTable = constructLALR();   //����LALR(1)�﷨������
	clock_t end = clock();
	cout << "�﷨�������������,����ʱ" << end - start << "����" << endl;
}

pair<shared_ptr<map<string, int>>, shared_ptr<vector<vector<LALRTableItem>>>> LALRAutomata::constructLALR()
{
	shared_ptr<map<string, int>> symbolToIndex = make_shared<map<string, int>>();
	{
		int count = 0;
		setToMap(terminnal, *symbolToIndex, count);    //�����ķ����ŵ��﷨������ڶ�ά��ӳ��
		setToMap(Nonterminal, *symbolToIndex, count);
	}
	shared_ptr<vector<vector<LALRTableItem>>>LALRTablePtr = make_shared<vector<vector<LALRTableItem>>>(SetOfVertex.size(), vector<LALRTableItem>((*symbolToIndex).size()));
	for (vector<Graph<LALRState, string>::GraphVertexNode*>::size_type i = 0; i < SetOfVertex.size(); ++i)    //����LR(0)������﷨���������������붯��
	{
		if (SetOfVertex[i]->seilring != nullptr)
		{
			map<string, int>::iterator temp = symbolToIndex->find(*(SetOfVertex[i]->Edgeseilring));
			(*LALRTablePtr)[i][temp->second].ActionType = LALRTableItem::action::MOVE;
			(*LALRTablePtr)[i][temp->second].LALRStateNumber = i;
		}
		for (Graph<LALRState, string>::GraphEdgeNode* p = SetOfVertex[i]->firsttailptr; p != nullptr; p = p->sametailptr)
		{
			map<string, int>::iterator temp = symbolToIndex->find(*(p->Edgedatafield));
			(*LALRTablePtr)[i][temp->second].ActionType = LALRTableItem::action::MOVE;
			(*LALRTablePtr)[i][temp->second].LALRStateNumber = p->head;
		}
	}
	vector<map<long, map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>>> FLKSymbolTran(SetOfVertex.size(), map<long, map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>>());
	for (vector<Graph<LALRState, string>::GraphVertexNode*>::size_type i = 0; i < SetOfVertex.size(); ++i)   //�����LR(0)�ں����Է����ɵ���ǰ�����Ų�ȷ��LR(0)�ں��֮����ǰ�����ŵĴ�����ϵ
	{
		for (map<long, map<int, set<string>>>::iterator p = SetOfVertex[i]->Vertexdatafield->kernel.begin(); p != SetOfVertex[i]->Vertexdatafield->kernel.end(); ++p)
		{
			map<long, map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>>::iterator x1 = FLKSymbolTran[i].insert(make_pair(p->first, map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>())).first;
			for (map<int, set<string>>::iterator q = p->second.begin(); q != p->second.end(); ++q)
			{
				if (q->first != get<1>(productionSet[p->first]).size())
				{
					map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>::iterator x2 = x1->second.insert(make_pair(q->first, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>())).first;
					vector<Graph<LALRState, string>::GraphVertexNode*>::size_type index = (*LALRTablePtr)[i][(*symbolToIndex)[get<1>(productionSet[p->first])[q->first].symbol]].LALRStateNumber;
					if (get<1>(productionSet[p->first])[q->first].TerminalOrNot == false)
					{
						map<long, map<int, set<string>>> kernelitem;
						kernelitem.insert(make_pair(p->first, map<int, set<string>>())).first->second.insert(make_pair(q->first, set<string>())).first->second.insert("#");
						map<long, LALRState::attribute> nonkernelitem;
						calculateClosureLALR(kernelitem, nonkernelitem);
						for (map<long, LALRState::attribute>::iterator m = nonkernelitem.begin(); m != nonkernelitem.end(); ++m)
						{
							if (m->second.dotposition != get<1>(productionSet[m->first]).size())
							{
								vector<Graph<LALRState, string>::GraphVertexNode*>::size_type index = (*LALRTablePtr)[i][(*symbolToIndex)[get<1>(productionSet[m->first])[m->second.dotposition].symbol]].LALRStateNumber;
								set<string>::const_iterator temp = m->second.ForwardLookingSign.find("#");
								if (temp != m->second.ForwardLookingSign.cend())
								{
									((SetOfVertex[index]->Vertexdatafield->kernel)[m->first])[m->second.dotposition + 1].insert(m->second.ForwardLookingSign.cbegin(), temp);
									((SetOfVertex[index]->Vertexdatafield->kernel)[m->first])[m->second.dotposition + 1].insert(++temp, m->second.ForwardLookingSign.cend());
									x2->second.insert(make_pair(index, map<long, set<int>>())).first->second.insert(make_pair(m->first, set<int>())).first->second.insert(m->second.dotposition + 1);
								}
								else
								{
									((SetOfVertex[index]->Vertexdatafield->kernel)[m->first])[m->second.dotposition + 1].insert(m->second.ForwardLookingSign.cbegin(), m->second.ForwardLookingSign.cend());
								}
							}
						}
						x2->second.insert(make_pair(index, map<long, set<int>>())).first->second.insert(make_pair(p->first, set<int>())).first->second.insert(q->first + 1);
					}
					else
					{
						((x2->second)[index] = map<long, set<int>>()).insert(make_pair(p->first, set<int>())).first->second.insert(q->first + 1);
					}
				}
			}
			if (x1->second.empty())
			{
				FLKSymbolTran[i].erase(x1);
			}
		}
	}

	(SetOfVertex[start]->Vertexdatafield->kernel[*(TerToPro[AugGraSS].begin())])[0].insert("$");  //Ϊ�����ķ������Ĳ���ʽ��д��ǰ���������������$,�����Է����ɵ�
	while (true)    //����ɨ������LR(0)�������ǰ�����ţ�ֱ��ĳһ����Ҳû����ǰ�����ű�����Ϊֹ
	{
		bool continueTran = false; //
		for (vector<Graph<LALRState, string>::GraphVertexNode*>::size_type i = 0; i < SetOfVertex.size(); ++i)
		{
			if (FLKSymbolTran[i].empty() == false)
			{
				map<long, map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>>::iterator p1 = FLKSymbolTran[i].begin();
				map<long, map<int, set<string>>>::iterator p2 = SetOfVertex[i]->Vertexdatafield->kernel.begin();
				while (p1 != FLKSymbolTran[i].end() && p2 != SetOfVertex[i]->Vertexdatafield->kernel.end())
				{
					if (p1->first == p2->first)
					{
						map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>::iterator q1 = p1->second.begin();
						map<int, set<string>>::iterator q2 = p2->second.begin();
						while (q1 != p1->second.end() && q2 != p2->second.end())
						{
							if (q1->first == q2->first)
							{
								if (q2->second.empty() == false)
								{
									for (map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>::iterator w1 = q1->second.begin(); w1 != q1->second.end(); ++w1)
									{
										map<long, set<int>>::iterator m1 = w1->second.begin();
										map<long, map<int, set<string>>>::iterator m2 = SetOfVertex[w1->first]->Vertexdatafield->kernel.begin();
										while (m1 != w1->second.end() && m2 != SetOfVertex[w1->first]->Vertexdatafield->kernel.end())
										{
											if (m1->first == m2->first)
											{
												set<int>::const_iterator v1 = m1->second.cbegin();
												map<int, set<string>>::iterator v2 = m2->second.begin();
												while (v1 != m1->second.cend() && v2 != m2->second.end())
												{
													if (*v1 == v2->first)
													{
														set<string>::size_type temp = v2->second.size();
														v2->second.insert(q2->second.cbegin(), q2->second.cend());
														if (temp != v2->second.size())
														{
															continueTran = true;
														}
														++v1;
														++v2;
													}
													else if (*v1 < v2->first)
													{
														++v1;
													}
													else
													{
														++v2;
													}
												}
												++m1;
												++m2;
											}
											else if (m1->first < m2->first)
											{
												++m1;
											}
											else
											{
												++m2;
											}
										}

									}
								}
								++q1;
								++q2;
							}
							else if (q1->first < q2->first)
							{
								++q1;
							}
							else
							{
								++q2;
							}
						}
						++p1;
						++p2;
					}
					else if (p1->first < p2->first)
					{
						++p1;
					}
					else
					{
						++p2;
					}
				}
			}
		}
		if (continueTran == false)
			break;
	}

	for (vector<Graph<LALRState, string>::GraphVertexNode*>::size_type i = 0; i < SetOfVertex.size(); ++i)
	{
		calculateClosureLALR(SetOfVertex[i]->Vertexdatafield->kernel, SetOfVertex[i]->Vertexdatafield->nonkernel);  //Ϊÿ��LALR(1)�ں������հ��õ�LALR(1)�
		for (map<long, map<int, set<string>>>::iterator p = SetOfVertex[i]->Vertexdatafield->kernel.begin(); p != SetOfVertex[i]->Vertexdatafield->kernel.end(); ++p)   //ΪLALR(1)�﷨�����������Լ����
		{
			for (map<int, set<string>>::iterator q = p->second.begin(); q != p->second.end(); ++q)
			{
				if (q->first == get<1>(productionSet[p->first]).size())
				{
					for (set<string>::const_iterator m = q->second.cbegin(); m != q->second.cend(); ++m)
					{
						map<string, int>::iterator temp = symbolToIndex->find(*m);
						if ((*LALRTablePtr)[i][temp->second].ActionType == LALRTableItem::action::MOVE)
						{
							cout << "ERROR:�����Լ��ͻ" << endl;
							cout << "״̬" << i << "Ҫ�����ķ�����" << temp->first << "������״̬" << (*LALRTablePtr)[i][temp->second].LALRStateNumber;
							cout << "ͬʱ״̬" << i << "Ҫ�����ķ�����" << temp->first << "���ò���ʽ" << p->first << "��Լ" << endl;
							continue;
						}
						else if ((*LALRTablePtr)[i][temp->second].ActionType == LALRTableItem::action::REDUCTION)
						{
							cout << "ERROR:��Լ��Լ��ͻ" << endl;
							cout << "״̬" << i << "Ҫ�����ķ�����" << temp->first << "���ò���ʽ" << (*LALRTablePtr)[i][temp->second].production << "��Լ" << endl;
							cout << "ͬʱ״̬" << i << "Ҫ�����ķ�����" << temp->first << "���ò���ʽ" << p->first << "��Լ" << endl;
							continue;
						}
						(*LALRTablePtr)[i][temp->second].ActionType = LALRTableItem::action::REDUCTION;   //�����Լ,��Լ��Լ��ͻ����
						(*LALRTablePtr)[i][temp->second].production = p->first;
					}
				}
			}
		}

		for (map<long, LALRState::attribute>::iterator temp = SetOfVertex[i]->Vertexdatafield->nonkernel.begin(); temp != SetOfVertex[i]->Vertexdatafield->nonkernel.end(); ++temp)
		{
			if (temp->second.dotposition == 0 && get<1>(productionSet[temp->first]).empty() == true)
			{
				for (set<string>::const_iterator m = temp->second.ForwardLookingSign.cbegin(); m != temp->second.ForwardLookingSign.cend(); ++m)
				{
					map<string, int>::iterator temp1 = symbolToIndex->find(*m);
					if ((*LALRTablePtr)[i][temp1->second].ActionType == LALRTableItem::action::MOVE)
					{
						cout << "ERROR:�����Լ��ͻ" << endl;
						cout << "״̬" << i << "Ҫ�����ķ�����" << temp1->first << "������״̬" << (*LALRTablePtr)[i][temp1->second].LALRStateNumber;
						cout << "ͬʱ״̬" << i << "Ҫ�����ķ�����" << temp1->first << "���ò���ʽ" << temp->first << "��Լ" << endl;
						continue;
					}
					else if ((*LALRTablePtr)[i][temp1->second].ActionType == LALRTableItem::action::REDUCTION)
					{
						cout << "ERROR:��Լ��Լ��ͻ" << endl;
						cout << "״̬" << i << "Ҫ�����ķ�����" << temp1->first << "���ò���ʽ" << (*LALRTablePtr)[i][temp1->second].production << "��Լ" << endl;
						cout << "ͬʱ״̬" << i << "Ҫ�����ķ�����" << temp1->first << "���ò���ʽ" << temp->first << "��Լ" << endl;
						continue;
					}
					(*LALRTablePtr)[i][temp1->second].ActionType = LALRTableItem::action::REDUCTION;    //�����Լ,��Լ��Լ��ͻ����
					(*LALRTablePtr)[i][temp1->second].production = temp->first;
				}
			}
		}
	}
	(*LALRTablePtr)[((*LALRTablePtr)[start][(*symbolToIndex)[StartSymbol]].LALRStateNumber)][(*symbolToIndex)["$"]].ActionType = LALRTableItem::action::ACCEPT;  //���﷨�����������������
	accept = (*LALRTablePtr)[start][(*symbolToIndex)[StartSymbol]].LALRStateNumber;
	(*LALRTablePtr)[((*LALRTablePtr)[start][(*symbolToIndex)[StartSymbol]].LALRStateNumber)][(*symbolToIndex)["$"]].NULLLable = "";
	return { symbolToIndex, LALRTablePtr };  //����LALR(1)�﷨������
}

void LALRAutomata::constructLRKernel()   //�������LR(0)��屣���ڼ̳ж���������ͼ��
{
	struct Vertex
	{
		LALRState* state = new LALRState();
		vector<Graph<LALRState, string>::GraphVertexNode*>::size_type index = 0;   //LALR״̬�����Ӧ���
		Vertex(LALRState* s, vector<Graph<LALRState, string>::GraphVertexNode*>::size_type i) :state(s), index(i) {}
		Vertex(const Vertex& copy)
		{
			state = new LALRState(*(copy.state));
			index = copy.index;
		}
		Vertex() = default;
		~Vertex() { delete state; }
	};
	deque<Vertex> workqueue;
	workqueue.push_back(Vertex());
	workqueue.back().state->kernel.insert(make_pair(*(TerToPro[AugGraSS].begin()), map<int, set<string>>())).first->second.insert(make_pair(0, set<string>()));
	start = addVertex(new LALRState(*(workqueue.back().state)));

	while (workqueue.empty() == false)
	{
		calculateClosureLR(workqueue.front().state->kernel, workqueue.front().state->nonkernel);
		map<string, LALRState*> temp;
		for (map<long, map<int, set<string>>>::iterator p = workqueue.front().state->kernel.begin(); p != workqueue.front().state->kernel.end(); ++p)
		{
			for (map<int, set<string>>::iterator p2 = p->second.begin(); p2 != p->second.end(); ++p2)
			{
				if (static_cast<vector<ProductionBodySymbol>::size_type>(p2->first) < get<1>(productionSet[p->first]).size())
				{
					map<string, LALRState*>::iterator q = temp.insert(make_pair(get<1>(productionSet[p->first])[p2->first].symbol, new LALRState())).first;
					q->second->kernel.insert(make_pair(p->first, map<int, set<string>>())).first->second.insert(make_pair(p2->first + 1, set<string>()));
				}
			}
		}

		for (map<long, LALRState::attribute>::iterator p = workqueue.front().state->nonkernel.begin(); p != workqueue.front().state->nonkernel.end(); ++p)
		{
			if (static_cast<vector<ProductionBodySymbol>::size_type>(p->second.dotposition) < get<1>(productionSet[p->first]).size())
			{
				map<string, LALRState*>::iterator q = temp.insert(make_pair(get<1>(productionSet[p->first])[p->second.dotposition].symbol, new LALRState())).first;
				q->second->kernel.insert(make_pair(p->first, map<int, set<string>>())).first->second.insert(make_pair(p->second.dotposition + 1, set<string>()));
			}
		}

		vector<bool> must_be_no_equal(SetOfVertex.size(), false);
		for (map<string, LALRState*>::iterator p = temp.begin(); p != temp.end(); )
		{
			vector<Graph<LALRState, string>::GraphVertexNode*>::size_type i = 0;
			for (; i < SetOfVertex.size(); ++i)
			{
				if (must_be_no_equal[i] == true)
					continue;
				map<long, map<int, set<string>>>::iterator left = p->second->kernel.begin();
				map<long, map<int, set<string>>>::iterator right = SetOfVertex[i]->Vertexdatafield->kernel.begin();
				while (left != p->second->kernel.end() && right != SetOfVertex[i]->Vertexdatafield->kernel.end())
				{
					if (left->first == right->first)
					{
						map<int, set<string>>::iterator inleft = left->second.begin();
						map<int, set<string>>::iterator inright = right->second.begin();
						while (inleft != left->second.end() && inright != right->second.end())
						{
							if (inleft->first == inright->first)
							{
								++inleft;
								++inright;
							}
							else
							{
								break;
							}
						}
						if (inleft != left->second.end() || inright != right->second.end())
							break;
						++left;
						++right;
					}
					else
					{
						break;
					}
				}
				if (left == p->second->kernel.end() && right == SetOfVertex[i]->Vertexdatafield->kernel.end())
				{
					must_be_no_equal[i] = true;
					addEdge(workqueue.front().index, i, new string(p->first));
					break;
				}
			}
			if (i == SetOfVertex.size())
			{
				vector<Graph<LALRState, string>::GraphVertexNode*>::size_type s = addVertex(new LALRState(*(p->second)));
				must_be_no_equal.push_back(true);
				addEdge(workqueue.front().index, s, new string(p->first));
				workqueue.push_back(Vertex(p->second, s));
			}
			p = temp.erase(p);
		}
		workqueue.pop_front();
	}
}

void LALRAutomata::calculateClosureLR(map<long, map<int, set<string>>>& kernelset, map<long, LALRState::attribute>& nonkernelset)  //kernelset��arrtribute���Ե�ForwardLookingSignΪ��,nonkernelsetͬ��,���ں�������ıհ������nonkernelset��
{
	nonkernelset.clear();
	deque<pair<long, LALRState::attribute>> workqueue;
	for (map<long, map<int, set<string>>>::iterator m = kernelset.begin(); m != kernelset.end(); ++m)
	{
		for (map<int, set<string>>::iterator m2 = m->second.begin(); m2 != m->second.end(); ++m2)
		{
			workqueue.push_back(make_pair(m->first, LALRState::attribute(m2->first)));
		}
	}

	while (workqueue.empty() == false)
	{
		if (static_cast<vector<ProductionBodySymbol>::size_type>(workqueue.front().second.dotposition) < get<1>(productionSet[workqueue.front().first]).size())
		{
			if (get<1>(productionSet[workqueue.front().first])[workqueue.front().second.dotposition].TerminalOrNot == false)
			{
				pair<long, LALRState::attribute> maxp = workqueue.front();
				workqueue.pop_front();
				for (set<long>::iterator m = TerToPro[get<1>(productionSet[maxp.first])[maxp.second.dotposition].symbol].begin(); m != TerToPro[get<1>(productionSet[maxp.first])[maxp.second.dotposition].symbol].end(); ++m)
				{
					auto temp2 = nonkernelset.insert(make_pair(*m, LALRState::attribute(0)));
					if (temp2.second)
						workqueue.push_back(*temp2.first);
				}
				continue;
			}
		}
		workqueue.pop_front();
	}
}

void LALRAutomata::calculateClosureLALR(map<long, map<int, set<string>>>& kernelset, map<long, LALRState::attribute>& nonkernelset)
{
	nonkernelset.clear();
	Priority_Queue<pair<long, LALRState::attribute>> workqueue(function<bool(const pair<long, LALRState::attribute>&, const pair<long, LALRState::attribute>&)>([](const pair<long, LALRState::attribute>& left, const pair<long, LALRState::attribute>& right)->bool {return left.first < right.first; }));  //ʹ��lambda���ʽ���ݲ���ʽ���ά�����ȼ�����
	for (map<long, map<int, set<string>>>::iterator m = kernelset.begin(); m != kernelset.end(); ++m)
	{
		for (map<int, set<string>>::iterator m2 = m->second.begin(); m2 != m->second.end(); ++m2)
		{
			workqueue.Insert(make_pair(m->first, LALRState::attribute(m2->first))).second->second.ForwardLookingSign.insert(m2->second.cbegin(), m2->second.cend());
		}
	}

	map<long, bool> first_set_of_right_side_of_dot_has_empty_symbol;
	while (workqueue.isEmpty() == false)
	{
		if (static_cast<vector<ProductionBodySymbol>::size_type>(workqueue.begin()->second.dotposition) < get<1>(productionSet[workqueue.begin()->first]).size())
		{
			if (get<1>(productionSet[workqueue.begin()->first])[workqueue.begin()->second.dotposition].TerminalOrNot == false)
			{
				shared_ptr<set<string>> temp = nullptr;
				bool appear_second_and_has_empty_symbol = false;
				if (workqueue.begin()->second.dotposition == 0 && get<1>(productionSet[workqueue.begin()->first])[workqueue.begin()->second.dotposition].symbol != StartSymbol)
				{
					if (first_set_of_right_side_of_dot_has_empty_symbol.find(workqueue.begin()->first) != first_set_of_right_side_of_dot_has_empty_symbol.end())
					{
						if (first_set_of_right_side_of_dot_has_empty_symbol[workqueue.begin()->first] == true)
						{
							temp = make_shared<set<string>>(nonkernelset[workqueue.begin()->first].ForwardLookingSign);
							appear_second_and_has_empty_symbol = true;
						}
						else
						{
							workqueue.erase(workqueue.begin());
							continue;
						}
					}
					else
					{
						temp = calculateFirst(get<1>(productionSet[workqueue.begin()->first]), workqueue.begin()->second.dotposition + 1, get<1>(productionSet[workqueue.begin()->first]).size() - 1);
						if (temp->empty() == true || *(temp->begin()) == "")
						{
							first_set_of_right_side_of_dot_has_empty_symbol[workqueue.begin()->first] = true;
						}
						else
						{
							first_set_of_right_side_of_dot_has_empty_symbol[workqueue.begin()->first] = false;
						}
					}
				}
				else
				{
					temp = calculateFirst(get<1>(productionSet[workqueue.begin()->first]), workqueue.begin()->second.dotposition + 1, get<1>(productionSet[workqueue.begin()->first]).size() - 1);
				}

				if (appear_second_and_has_empty_symbol == false)
				{
					if (temp->empty() == true || *(temp->begin()) == "")
					{
						if (temp->empty() == false && *(temp->begin()) == "")
							temp->erase(temp->begin());
						temp->insert(workqueue.begin()->second.ForwardLookingSign.cbegin(), workqueue.begin()->second.ForwardLookingSign.cend());
					}
				}

				pair<long, LALRState::attribute> maxp = *(workqueue.begin());
				workqueue.erase(workqueue.begin());
				for (set<long>::iterator m = TerToPro[get<1>(productionSet[maxp.first])[maxp.second.dotposition].symbol].begin(); m != TerToPro[get<1>(productionSet[maxp.first])[maxp.second.dotposition].symbol].end(); ++m)
				{
					auto temp2 = nonkernelset.insert(make_pair(*m, LALRState::attribute(0)));
					if (temp2.second)
					{
						temp2.first->second.ForwardLookingSign.insert(temp->cbegin(), temp->cend());
						workqueue.Insert(*temp2.first);
					}
					else
					{
						set<string>::size_type q = temp2.first->second.ForwardLookingSign.size();
						temp2.first->second.ForwardLookingSign.insert(temp->cbegin(), temp->cend());
						if (temp2.first->second.ForwardLookingSign.size() != q)
						{
							Priority_Queue<pair<long, LALRState::attribute>>::iterator q2 = workqueue.Insert(*temp2.first).second;
							if (q2 != workqueue.begin())
							{
								--q2;
								if (q2->first == temp2.first->first)
								{
									if (q2->second.dotposition == 0 && q2->first != *(TerToPro[AugGraSS].begin()))
									{
										workqueue.erase(q2);
									}
								}
							}
						}
					}
				}
				continue;
			}
		}
		workqueue.erase(workqueue.begin());
	}
}

bool LALRAutomata::isSubSet(const map<string, set<string>>& left, const set<string>& right)
{
	map<string, set<string>>::const_iterator leftit = left.cbegin();
	set<string>::const_iterator rightit = right.cbegin();

	while (leftit != left.cend() && rightit != right.cend())
	{
		if (*rightit < leftit->first)
			return false;
		else if (*rightit > leftit->first)
		{
			++leftit;
		}
		else
		{
			++leftit;
			++rightit;
		}
	}

	if (rightit != right.cend())
		return false;
	return true;

}

#include "FollowKAndDependency.h"

void jointStrAndInsert(TrieTreeNode* run_ptr, TrieTreeNode* temp_ptr, FollowKAndDependency& p, size_t K)
{
	vector<string> temp_list(run_ptr->key_in_trie.key);
	for (size_t v = 1; v <= K - run_ptr->key_in_trie.key.size(); ++v)
	{
		if (v > temp_ptr->key_in_trie.key.size())
			break;
		temp_list.push_back(temp_ptr->key_in_trie.key[v - 1]);
	}

	bool b = p.FollowK->insert(temp_list);
	if (b)
	{
		p.followK_new_add->insert(temp_list);
	}
}
shared_ptr<map<string, shared_ptr<TrieTree>>> LALRAutomata::calculateFollowK(size_t K)
{
	enum class GeneratingCycles { CURRENT_CYCLE, PREVIOUS_CYCLE };
	map<string, FollowKAndDependency> follow_k_set_and_dependency_for_unterminal_symbol;
	map<string, pair<GeneratingCycles, map<string, FollowKAndDependency>::iterator>> pre_and_cur_cycle_finish_followK_compute_info;  //���ս��,followKΪ�������ɻ�����һ������,ָ��follow_k_set_and_dependency_for_unterminal_symbol�ж�Ӧ�������
	{
		auto h = follow_k_set_and_dependency_for_unterminal_symbol.insert(make_pair(AugGraSS, FollowKAndDependency())).first;
		vector<string> temp = { "$" };
		h->second.FollowK->insert(temp);
		h->second.followK_new_add->insert(temp);
	}

	for (map<long, tuple<string, vector<ProductionBodySymbol>, set<string>>>::iterator p = productionSet.begin(); p != productionSet.end(); ++p)
	{
		for (vector<ProductionBodySymbol>::size_type i = 0; i < get<1>(p->second).size(); ++i)
		{
			if (get<1>(p->second)[i].TerminalOrNot == false)
			{
				map<string, FollowKAndDependency>::iterator it = follow_k_set_and_dependency_for_unterminal_symbol.insert(make_pair(get<1>(p->second)[i].symbol, FollowKAndDependency())).first;

				if (i == get<1>(p->second).size() - 1)    //���ս�����޷��ţ�����ʽͷ����ֱ��������
				{
					it->second.direct_dependency.insert(get<0>(p->second));
				}
				else
				{
					shared_ptr<TrieTree> q = calculateFirstK(get<1>(p->second), i + 1, get<1>(p->second).size() - 1, K);  //���ս�����з��Ŵ�,������firstK
					for (TrieTreeNode* run = q->get_first_leaf_list_node_ptr(); run != nullptr; run = run->key_in_trie.next)
					{
						if (run->key_in_trie.key.empty() == true)   //firstK���пմ�,����ʽͷ����ֱ��������
						{
							it->second.direct_dependency.insert(get<0>(p->second));
						}
						else if (run->key_in_trie.key.size() < K)  //firstK���г���С��K�Ĵ����ô��Ͳ���ʽͷ������������
						{
							map<string, shared_ptr<TrieTree>>::iterator tempit = it->second.undirect_dependency.insert(make_pair(get<0>(p->second), make_shared<TrieTree>())).first;
							tempit->second->insert(run->key_in_trie.key);
						}
						else    //firstK���г���ΪK�Ĵ�,�ô�ֱ�Ӽ�������
						{
							it->second.FollowK->insert(run->key_in_trie.key);
							it->second.followK_new_add->insert(run->key_in_trie.key);
						}
					}
				}
			}
		}
	}
	//ȥ��ֱ�������кͱ�����������ȵ����ֱ�������ͼ��������Ϊ��,��followK�����
	for (map<string, FollowKAndDependency>::iterator p = follow_k_set_and_dependency_for_unterminal_symbol.begin(); p != follow_k_set_and_dependency_for_unterminal_symbol.end(); ++p)
	{
		if (!(p->second.undirect_dependency.empty() == true && p->second.direct_dependency.empty() == true))
		{
			p->second.direct_dependency.erase(p->first);
		}

		if (p->second.undirect_dependency.empty() == true && p->second.direct_dependency.empty() == true)
		{
			p->second.followK_have_got = true;
			pre_and_cur_cycle_finish_followK_compute_info.insert(make_pair(p->first, make_pair(GeneratingCycles::PREVIOUS_CYCLE, p)));
		}
	}

	bool first_set_has_changed = false;
	bool result_has_changed = false;
	bool result_has_changed_previous_run = true;
	bool is_first_cycle = true;
	while (true)
	{
		map<string, pair<GeneratingCycles, map<string, FollowKAndDependency>::iterator>>::iterator n = pre_and_cur_cycle_finish_followK_compute_info.begin();
		for (map<string, FollowKAndDependency>::iterator p = follow_k_set_and_dependency_for_unterminal_symbol.begin(); p != follow_k_set_and_dependency_for_unterminal_symbol.end(); ++p)
		{
			if (p->second.followK_have_got == true)
			{
				if (pre_and_cur_cycle_finish_followK_compute_info.find(p->first) == pre_and_cur_cycle_finish_followK_compute_info.end())
					p->second.followK_new_add->clear();
				continue;
			}

			if (is_first_cycle == false)
			{
				p->second.followK_new_add->clear();
			}

			size_t size = p->second.FollowK->getSize();
			if (result_has_changed_previous_run)
			{
				map<string, pair<GeneratingCycles, map<string, FollowKAndDependency>::iterator>>::iterator itleft = pre_and_cur_cycle_finish_followK_compute_info.begin();
				map<string, shared_ptr<TrieTree>>::iterator itright = p->second.undirect_dependency.begin();
				while (itleft != pre_and_cur_cycle_finish_followK_compute_info.end() && itright != p->second.undirect_dependency.end())
				{
					if (itleft->first == itright->first)
					{
						for (TrieTreeNode* temp_ptr = itleft->second.second->second.followK_new_add->get_first_leaf_list_node_ptr(); temp_ptr != nullptr; temp_ptr = temp_ptr->key_in_trie.next)
						{
							for (TrieTreeNode* run_ptr = itright->second->get_first_leaf_list_node_ptr(); run_ptr != nullptr; run_ptr = run_ptr->key_in_trie.next)
							{
								jointStrAndInsert(run_ptr, temp_ptr, p->second, K);
							}
						}
						itright = p->second.undirect_dependency.erase(itright);
						++itleft;
					}
					else if (itleft->first < itright->first)
					{
						++itleft;
					}
					else
					{
						++itright;
					}
				}

				itleft = pre_and_cur_cycle_finish_followK_compute_info.begin();
				set<string>::iterator g = p->second.direct_dependency.begin();
				while (itleft != pre_and_cur_cycle_finish_followK_compute_info.end() && g != p->second.direct_dependency.end())
				{
					if (itleft->first == *g)
					{
						for (TrieTreeNode* temp_ptr = itleft->second.second->second.followK_new_add->get_first_leaf_list_node_ptr(); temp_ptr != nullptr; temp_ptr = temp_ptr->key_in_trie.next)
						{
							bool b = p->second.FollowK->insert(temp_ptr->key_in_trie.key);
							if (b)
							{
								p->second.followK_new_add->insert(temp_ptr->key_in_trie.key);
							}
						}
						g = p->second.direct_dependency.erase(g);
						++itleft;
					}
					else if (itleft->first < *g)
					{
						++itleft;
					}
					else
					{
						++g;
					}
				}

				if (p->second.direct_dependency.empty() == true && p->second.undirect_dependency.empty() == true)
				{
					pre_and_cur_cycle_finish_followK_compute_info.insert(make_pair(p->first, make_pair(GeneratingCycles::CURRENT_CYCLE, p)));
					p->second.followK_have_got = true;
					result_has_changed = true;
					continue;
				}
			}

			map<string, shared_ptr<TrieTree>>::iterator itself = p->second.undirect_dependency.end();
			for (map<string, shared_ptr<TrieTree>>::iterator itright = p->second.undirect_dependency.begin(); itright != p->second.undirect_dependency.end(); ++itright)
			{
				if (itright->first == p->first)
				{
					itself = itright;
					continue;
				}
				for (TrieTreeNode* temp_ptr = follow_k_set_and_dependency_for_unterminal_symbol[itright->first].followK_new_add->get_first_leaf_list_node_ptr(); temp_ptr != nullptr; temp_ptr = temp_ptr->key_in_trie.next)
				{
					for (TrieTreeNode* run_ptr = itright->second->get_first_leaf_list_node_ptr(); run_ptr != nullptr; run_ptr = run_ptr->key_in_trie.next)
					{
						jointStrAndInsert(run_ptr, temp_ptr, p->second, K);
					}
				}
			}

			if (itself != p->second.undirect_dependency.end())
			{
				for (TrieTreeNode* temp_ptr = p->second.followK_new_add->get_first_leaf_list_node_ptr(); temp_ptr != nullptr; temp_ptr = temp_ptr->key_in_trie.next)
				{
					for (TrieTreeNode* run_ptr = itself->second->get_first_leaf_list_node_ptr(); run_ptr != nullptr; run_ptr = run_ptr->key_in_trie.next)
					{
						jointStrAndInsert(run_ptr, temp_ptr, p->second, K);
					}
				}
			}

			for (set<string>::iterator g = p->second.direct_dependency.begin(); g != p->second.direct_dependency.end(); ++g)
			{
				for (TrieTreeNode* temp_ptr = follow_k_set_and_dependency_for_unterminal_symbol[*g].followK_new_add->get_first_leaf_list_node_ptr(); temp_ptr != nullptr; temp_ptr = temp_ptr->key_in_trie.next)
				{
					bool b = p->second.FollowK->insert(temp_ptr->key_in_trie.key);
					if (b)
					{
						p->second.followK_new_add->insert(temp_ptr->key_in_trie.key);
					}
				}
			}

			if (p->second.FollowK->getSize() != size)
			{
				first_set_has_changed = true;
			}

			if (n != pre_and_cur_cycle_finish_followK_compute_info.end())  //ɾ�����ᱻ�õ���pre_and_cur_cycle_finish_followK_compute_info�е���
			{
				if (p->first == n->first)
				{
					if (n->second.first == GeneratingCycles::CURRENT_CYCLE)
					{
						++n;
					}
					else
					{
						n = pre_and_cur_cycle_finish_followK_compute_info.erase(n);
					}
				}
			}
		}

		if (!first_set_has_changed && !result_has_changed)
		{
			break;
		}
		else
		{
			result_has_changed_previous_run = result_has_changed;
			first_set_has_changed = false;
			result_has_changed = false;
		}

		if (is_first_cycle)
			is_first_cycle = false;

		for (map<string, pair<GeneratingCycles, map<string, FollowKAndDependency>::iterator>>::iterator temp = pre_and_cur_cycle_finish_followK_compute_info.begin(); temp != pre_and_cur_cycle_finish_followK_compute_info.end(); ++temp)
		{
			temp->second.first = GeneratingCycles::CURRENT_CYCLE;
		}
	}
	shared_ptr<map<string, shared_ptr<TrieTree>>> result = make_shared<map<string, shared_ptr<TrieTree>>>();
	for (map<string, FollowKAndDependency>::iterator p = follow_k_set_and_dependency_for_unterminal_symbol.begin(); p != follow_k_set_and_dependency_for_unterminal_symbol.end(); ++p)
	{
		result->insert(make_pair(p->first, shared_ptr<TrieTree>(p->second.FollowK)));
	}
	return result;
}

shared_ptr<map<string, set<string>>> LALRAutomata::calculateFollow()
{
	enum class GeneratingCycles { CURRENT_CYCLE, PREVIOUS_CYCLE };
	map<string, tuple<set<string>, set<string>, set<string>, bool>> temp;  //���ս��,��ǰfollow������follow,��������,�Ƿ�������
	map<string, pair<GeneratingCycles, map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator>> pre_and_cur_cycle_finish_follow_compute_info;  //���ս�� �����ִ� temp�ж�Ӧ�������
	{
		auto h = temp.insert(make_pair(AugGraSS, tuple<set<string>, set<string>, set<string>, bool>())).first;
		get<0>(h->second).insert("$");
		get<1>(h->second).insert("$");
	}

	for (map<long, tuple<string, vector<ProductionBodySymbol>, set<string>>>::iterator p = productionSet.begin(); p != productionSet.end(); ++p)
	{
		for (vector<ProductionBodySymbol>::size_type i = 0; i < get<1>(p->second).size(); ++i)
		{
			if (get<1>(p->second)[i].TerminalOrNot == false)
			{
				if (i == get<1>(p->second).size() - 1)
				{
					map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator it = temp.insert(make_pair(get<1>(p->second)[i].symbol, tuple<set<string>, set<string>, set<string>, bool>())).first;
					get<2>(it->second).insert(get<0>(p->second));
				}
				else
				{
					shared_ptr<set<string>> q = calculateFirst(get<1>(p->second), i + 1, get<1>(p->second).size() - 1);
					map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator it = temp.insert(make_pair(get<1>(p->second)[i].symbol, tuple<set<string>, set<string>, set<string>, bool>())).first;
					if (*(q->begin()) == "")
					{
						set<string>::iterator w = q->begin();
						get<0>(it->second).insert(++w, q->end());
						get<1>(it->second).insert(w, q->end());
						get<2>(it->second).insert(get<0>(p->second));
					}
					else
					{
						get<0>(it->second).insert(q->begin(), q->end());
						get<1>(it->second).insert(q->begin(), q->end());
					}
				}
			}
		}
	}

	for (map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator p = temp.begin(); p != temp.end(); ++p)
	{
		if (get<2>(p->second).empty() == true)
		{
			get<3>(p->second) = true;
			pre_and_cur_cycle_finish_follow_compute_info.insert(make_pair(p->first, make_pair(GeneratingCycles::PREVIOUS_CYCLE, p)));
		}
		else
		{
			set<string>::iterator tempit;
			if ((tempit = get<2>(p->second).find(p->first)) != get<2>(p->second).end())
			{
				get<2>(p->second).erase(tempit);
				if (get<2>(p->second).empty() == true)
				{
					get<3>(p->second) = true;
					pre_and_cur_cycle_finish_follow_compute_info.insert(make_pair(p->first, make_pair(GeneratingCycles::PREVIOUS_CYCLE, p)));
				}
				else
				{
					get<3>(p->second) = false;
				}
			}
			else
			{
				get<3>(p->second) = false;
			}
		}
	}

	bool first_set_has_changed = false;
	bool result_has_changed = false;
	bool result_has_changed_previous_run = true;
	bool is_first_cycle = true;
	while (true)
	{
		map<string, pair<GeneratingCycles, map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator>>::iterator n = pre_and_cur_cycle_finish_follow_compute_info.begin();
		for (map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator p = temp.begin(); p != temp.end(); ++p)
		{
			if (get<3>(p->second) == true)
			{
				if (pre_and_cur_cycle_finish_follow_compute_info.find(p->first) == pre_and_cur_cycle_finish_follow_compute_info.end())
					get<1>(p->second).clear();
				continue;
			}

			if (is_first_cycle == false)
			{
				get<1>(p->second).clear();
			}

			set<string>::size_type size = get<0>(p->second).size();
			if (result_has_changed_previous_run)
			{
				map<string, pair<GeneratingCycles, map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator>>::iterator itleft = pre_and_cur_cycle_finish_follow_compute_info.begin();
				set<string>::iterator itright = get<2>(p->second).begin();
				while (itleft != pre_and_cur_cycle_finish_follow_compute_info.end() && itright != get<2>(p->second).end())
				{
					if (itleft->first == *itright)
					{
						computeDifferenceSet(get<1>(itleft->second.second->second), get<0>(p->second), get<1>(p->second), false);
						itright = get<2>(p->second).erase(itright);
						++itleft;
					}
					else if (itleft->first < *itright)
					{
						++itleft;
					}
					else
					{
						++itright;
					}
				}
				if (get<2>(p->second).empty())
				{
					pre_and_cur_cycle_finish_follow_compute_info.insert(make_pair(p->first, make_pair(GeneratingCycles::CURRENT_CYCLE, p)));
					get<3>(p->second) = true;
					result_has_changed = true;
					continue;
				}
			}

			for (set<string>::iterator m = get<2>(p->second).begin(); m != get<2>(p->second).end(); ++m)
			{
				computeDifferenceSet(get<1>(temp[*m]), get<0>(p->second), get<1>(p->second), false);
			}

			if (get<0>(p->second).size() != size)
			{
				first_set_has_changed = true;
			}

			if (n != pre_and_cur_cycle_finish_follow_compute_info.end())
			{
				if (p->first == n->first)
				{
					if (n->second.first == GeneratingCycles::CURRENT_CYCLE)
					{
						++n;
					}
					else
					{
						n = pre_and_cur_cycle_finish_follow_compute_info.erase(n);
					}
				}
			}
		}

		if (!first_set_has_changed && !result_has_changed)
		{
			break;
		}
		else
		{
			result_has_changed_previous_run = result_has_changed;
			first_set_has_changed = false;
			result_has_changed = false;
		}

		if (is_first_cycle)
			is_first_cycle = false;

		for (map<string, pair<GeneratingCycles, map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator>>::iterator temp = pre_and_cur_cycle_finish_follow_compute_info.begin(); temp != pre_and_cur_cycle_finish_follow_compute_info.end(); ++temp)
		{
			temp->second.first = GeneratingCycles::CURRENT_CYCLE;
		}
	}

	shared_ptr<map<string, set<string>>> result = make_shared<map<string, set<string>>>();
	for (map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator p = temp.begin(); p != temp.end(); ++p)
	{
		result->insert(make_pair(p->first, set<string>())).first->second.insert(get<0>(p->second).begin(), get<0>(p->second).end());
	}
	return result;
}

void LALRAutomata::computeDifferenceSet(set<string>& left_operand, set<string>& right_operand, set<string>& result, bool clear_or_not)
{
	set<string>::iterator left_it = left_operand.begin();
	if (left_it != left_operand.end() && *left_it == "")
		++left_it;
	set<string>::iterator right_it = right_operand.begin();
	if (right_it != right_operand.end() && *right_it == "")
		++right_it;

	while (left_it != left_operand.end() && right_it != right_operand.end())
	{
		if (*left_it < *right_it)
		{
			right_operand.insert(*left_it);
			result.insert(*left_it);
			++left_it;
		}
		else if (*left_it > *right_it)
		{
			++right_it;
		}
		else
		{
			++left_it;
			++right_it;
		}
	}

	if (left_it != left_operand.end())
	{
		while (left_it != left_operand.end())
		{
			right_operand.insert(*left_it);
			result.insert(*left_it);
			++left_it;
		}
	}

	if (clear_or_not)
		left_operand.clear();
}

shared_ptr<TrieTree> LALRAutomata::calculateFirstK(const vector<ProductionBodySymbol>& Pro, vector<ProductionBodySymbol>::size_type left, vector<ProductionBodySymbol>::size_type right, size_t K)
{
	shared_ptr<TrieTree> result = make_shared<TrieTree>();
	if (left <= right)
	{
		tuple<vector<ProductionBodySymbol>, set<string>, vector<bool>> production_info;
		map<string, pair<TrieTreeNode*, size_t>> ptr_point_to_last_visit;
		for (vector<ProductionBodySymbol>::size_type i = left; i <= right; ++i)
		{
			get<0>(production_info).push_back(ProductionBodySymbol(Pro[i]));
			if (Pro[i].TerminalOrNot == false)
			{
				get<1>(production_info).insert(Pro[i].symbol);
				get<2>(production_info).push_back(false);
				ptr_point_to_last_visit.insert(make_pair(Pro[i].symbol, make_pair(nullptr, (*firstK)[Pro[i].symbol]->getSize())));
			}
		}

		vector<SymbolInfo>::size_type m = 0;
		vector<TraceStackNode> trace_back_stack;
		bool back_flag = true;// ���ݱ�־
		size_t part_first_str_length = 0;//����firstK����

		TrieTreeNode* run_ptr = nullptr;
		m = 0;
		size_t visit_pos_every_layer = 0;
		bool first_into_nonter = true;
		for (; true; )   //���������Ŵ�firstK�ǶԼ�����ս��firstK�㷨�ļ�
		{
			if (m < get<0>(production_info).size())
			{
				if (back_flag == true)  //ǰ��������
				{
					if (get<0>(production_info)[m].TerminalOrNot == false)  //��ǰΪ���ս��
					{
						pair<TrieTreeNode*, size_t>& p2 = ptr_point_to_last_visit[get<0>(production_info)[m].symbol];
						TrieTree& temp2 = (*(*firstK)[get<0>(production_info)[m].symbol]);

						if (visit_pos_every_layer == 0 && first_into_nonter)
						{
							run_ptr = temp2.get_first_leaf_list_node_ptr();
						}
						size_t _size = run_ptr->key_in_trie.key.size();

						if (_size + part_first_str_length < K)  //����firstK�����δ�ﵽ����K
						{
							trace_back_stack.push_back(TraceStackNode(visit_pos_every_layer, run_ptr, m));
							part_first_str_length += _size;
							visit_pos_every_layer = 0;
							if (m + 1 < get<0>(production_info).size())
							{
								if (get<0>(production_info)[m + 1].TerminalOrNot == false)
								{
									run_ptr = (*firstK)[get<0>(production_info)[m + 1].symbol]->get_first_leaf_list_node_ptr();
								}
							}
							++m;
						}
						else
						{
							vector<string> first_k;
							splicing_firstK(first_k, trace_back_stack, get<0>(production_info), m);

							for (size_t t = 0; t < K - part_first_str_length; ++t)
							{
								first_k.push_back(run_ptr->key_in_trie.key[t]);
							}
							result->insert(first_k);
							back_flag = false;
						}
						if (first_into_nonter)
							first_into_nonter = false;
					}
					else
					{
						if (part_first_str_length + 1 == K)
						{
							vector<string> first_k;
							splicing_firstK(first_k, trace_back_stack, get<0>(production_info), m);
							first_k.push_back(get<0>(production_info)[m].symbol);
							result->insert(first_k);
							--m;
							back_flag = false;
						}
						else
						{
							if (m + 1 < get<0>(production_info).size())
							{
								if (get<0>(production_info)[m + 1].TerminalOrNot == false)
								{
									run_ptr = (*firstK)[get<0>(production_info)[m + 1].symbol]->get_first_leaf_list_node_ptr();
								}
							}
							++part_first_str_length;
							++m;
						}
					}
				}
				else
				{
					if (trace_back_stack.empty() == false && trace_back_stack.back().symbol_array_index == m || get<0>(production_info)[m].TerminalOrNot == true)
					{
						if (trace_back_stack.empty() == false && trace_back_stack.back().symbol_array_index == m)
						{
							visit_pos_every_layer = trace_back_stack.back().have_visited_index;
							run_ptr = trace_back_stack.back().firstK_for_symbol;
							pair<TrieTreeNode*, size_t>& p2 = ptr_point_to_last_visit[get<0>(production_info)[m].symbol];
							part_first_str_length -= trace_back_stack.back().firstK_for_symbol->key_in_trie.key.size();
							trace_back_stack.pop_back();
						}
						else
						{
							if (m == 0)
								break;
							else
							{
								--part_first_str_length;
								--m;
							}
							continue;
						}
					}

					if (run_ptr->key_in_trie.next == nullptr)
					{
						if (m == 0)
						{
							break;
						}
						else
						{
							--m;
						}
					}
					else
					{
						++visit_pos_every_layer;
						run_ptr = run_ptr->key_in_trie.next;
						back_flag = true;
					}
				}
			}
			else
			{
				vector<string> first_k;
				splicing_firstK(first_k, trace_back_stack, get<0>(production_info), m);
				result->insert(first_k);
				back_flag = false;
				--m;
			}
		}
	}
	return result;
}
void LALRAutomata::splicing_firstK(vector<string>& first_k, vector<TraceStackNode>& trace_back_stack, vector<ProductionBodySymbol>& p, size_t& m)
{
	size_t a = 0;
	for (size_t t = 0; t < m; ++t)
	{
		if (trace_back_stack.empty() == false && t == trace_back_stack[a].symbol_array_index)
		{
			for (vector<string>::iterator t2 = trace_back_stack[a].firstK_for_symbol->key_in_trie.key.begin(); t2 != trace_back_stack[a].firstK_for_symbol->key_in_trie.key.end(); ++t2)
				first_k.push_back(*t2);
			if (a + 1 != trace_back_stack.size())
				++a;
		}
		else
		{
			first_k.push_back(p[t].symbol);
		}
	}
}

void updateProductHeadRelevant(long& pro_head_symbol_array_index, long& current_advance_point_in_pro_head_symbol_indexs, vector<size_t>& v, size_t m)
{
	if (pro_head_symbol_array_index - 1 < 0 || v[pro_head_symbol_array_index - 1] > m)
	{
		if (pro_head_symbol_array_index - 1 >= 0)
		{
			--pro_head_symbol_array_index;
			if (pro_head_symbol_array_index == 0)
				current_advance_point_in_pro_head_symbol_indexs = -1;
			else
			{
				current_advance_point_in_pro_head_symbol_indexs = pro_head_symbol_array_index - 1;
			}
		}
	}
}

shared_ptr<map<string, shared_ptr<TrieTree>>> LALRAutomata::calculateFirstK(size_t K)
{
	map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>> temp;  //����ʽͷ�ķ��ս����,����ʽ���,����ʽ����ż������Ϣ,����ʽ�������з��ս���ż���,�Ͳ���ʽͷ��ͬ�����з��ս�����±꼯��
	shared_ptr<map<string, shared_ptr<TrieTree>>> result = make_shared<map<string, shared_ptr<TrieTree>>>();//����firstK�������ݽṹ
	map<string, pair<TrieTreeNode*, size_t>> ptr_point_to_last_visit;  //���ս����,pair<ָ����ս�����һ��������firstK�ڵ��ָ��,�÷��ս������firstK�ڵ����>

	for (map<long, tuple<string, vector<ProductionBodySymbol>, set<string>>>::iterator run = productionSet.begin(); run != productionSet.end(); ++run)
	{
		if (get<0>(run->second) != AugGraSS)
			ptr_point_to_last_visit.insert(make_pair(get<0>(run->second), make_pair(nullptr, 0)));
		if (get<1>(run->second).empty() == true || get<2>(run->second).empty() == true)  //���ս���Ƶ��ս���Ŵ���ǰK���ս������FirstK��,����Kȫ�����룬�Ƶ��մ������ռ�
		{
			map<string, shared_ptr<TrieTree>>::iterator q = result->find(get<0>(run->second));
			if (q == result->end())
			{
				q = result->insert(make_pair(get<0>(run->second), make_shared<TrieTree>())).first;
			}

			if (get<1>(run->second).empty())
			{
				q->second->insert(vector<string>());
			}
			else
			{
				size_t _size;
				if (get<1>(run->second).size() >= K)
				{
					_size = K - 1;
				}
				else
				{
					_size = get<1>(run->second).size() - 1;
				}

				vector<string> temp;
				for (vector<ProductionBodySymbol>::size_type i = 0; i <= _size; ++i)
				{
					temp.push_back(get<1>(run->second)[i].symbol);
				}
				q->second->insert(temp);
			}
		}
		else   //ǰk�����ž�Ϊ�ս����ֱ�ӽ������firstK,���򽫵�ǰ����ʽ�����Ϣ����temp
		{
			{
				vector<ProductionBodySymbol>::size_type p = 0;
				for (; p < get<1>(run->second).size(); ++p)
				{
					if (get<1>(run->second)[p].TerminalOrNot == false)
						break;
					if (p + 1 == K)
					{
						++p;
						break;
					}
				}
				if (p == K)
				{
					map<string, shared_ptr<TrieTree>>::iterator q = result->find(get<0>(run->second));
					if (q == result->end())
					{
						q = result->insert(make_pair(get<0>(run->second), make_shared<TrieTree>())).first;
					}

					vector<string> temp;
					for (vector<ProductionBodySymbol>::size_type i = 0; i < K; ++i)
					{
						temp.push_back(get<1>(run->second)[i].symbol);
					}
					q->second->insert(temp);
					continue;
				}
			}
			map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>>::iterator tempit1 = temp.insert(make_pair(get<0>(run->second), map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>())).first;
			map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>::iterator tempit2 = tempit1->second.insert(make_pair(run->first, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>())).first;

			vector<ProductionBodySymbol>::size_type p = 0;
			for (; p != get<1>(run->second).size(); ++p)
			{
				if (get<1>(run->second)[p].TerminalOrNot == false)
				{
					if (get<1>(run->second)[p].symbol == get<0>(run->second))
					{
						get<2>(tempit2->second).push_back(p);
					}
					get<1>(tempit2->second).insert(get<1>(run->second)[p].symbol);
				}
				get<0>(tempit2->second).push_back(SymbolInfo(get<1>(run->second)[p]));
			}
		}
	}

	//����firstK����Ϊ�յķ��ս������temp�в���ʽ���ж�Ӧ�ڸ÷��ս����firstK��
	for (map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>>::iterator p = temp.begin(); p != temp.end(); ++p)
	{
		for (map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>::iterator q = p->second.begin(); q != p->second.end(); ++q)
		{
			for (vector<SymbolInfo>::iterator run = get<0>(q->second).begin(); run != get<0>(q->second).end(); ++run)
			{
				if (run->symbol_info.TerminalOrNot == false)
				{
					map<string, shared_ptr<TrieTree>>::iterator m = result->find(run->symbol_info.symbol);
					if (m != result->end())
					{
						for (size_t count = 1; count <= m->second->getSize(); ++count)
						{
							run->visit_flag.push_back(false);
						}
					}
				}
			}
		}
	}

	//��firstK����Ϊ�յķ��ս��������ptr_point_to_last_visit�ж�Ӧ��
	for (map<string, shared_ptr<TrieTree>>::iterator p = result->begin(); p != result->end(); ++p)
	{
		auto p2 = ptr_point_to_last_visit.find(p->first);
		if (p2 != ptr_point_to_last_visit.end())
			p2->second.second = p->second->getSize();
	}

	while (true)
	{
		bool stopOrNot = true;   //����Ƿ���ĳ�����ս��firstK�����ı�
		for (map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>>::iterator p = temp.begin(); p != temp.end(); ++p)
		{
			auto o2 = result->find(p->first);
			size_t size_before_run;
			if (o2 != result->end())   //��¼����ǰ��ǰ���ս��firstK����С
			{
				size_before_run = o2->second->getSize();
			}
			else
			{
				size_before_run = 0;
			}

			for (map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>::iterator q = p->second.begin(); q != p->second.end(); ++q)  //��¼��ǰ����ʽ���и����ս��firstK��β�ڵ�ָ��������ڵ���
			{
				map<string, pair<TrieTreeNode*, size_t>> temp_list;
				for (set<string>::iterator e = get<1>(q->second).begin(); e != get<1>(q->second).end(); ++e)
				{
					auto e2 = result->find(*e);
					if (e2 != result->end())
					{
						temp_list.insert(make_pair(*e, make_pair(e2->second->get_last_leaf_list_node_ptr(), ptr_point_to_last_visit[*e].second)));
					}
				}

				vector<SymbolInfo>::size_type m = 0;      //��ǰ���ڴ����±�
				vector<TraceBackInfo> trace_back_stack;//���ݹ���ջ
				vector<SymbolInfo>& symbol_in_production_body = get<0>(q->second);
				size_t new_add_first_before_num = 0;//ջ��ָ��ָ��ĸ��ڵ���Ϊ�����ڵ�Ľڵ���
				bool back_flag = true;// ���ݱ�־
				long current_advance_point_in_pro_head_symbol_indexs = -1;// ��ǰ�±�����ڲ���ʽ���в���ʽͷ����λ�õ�λ��
				size_t new_add_count = 0;//��ǰ������±�֮���������ڵ�Ĳ���
				size_t part_first_str_length = 0;//Ŀǰ���γɵĲ���firstK������
				for (; m < symbol_in_production_body.size(); ++m)  //���ݸ������������ڵ��ʼ��new_add_count
				{
					if (symbol_in_production_body[m].symbol_info.TerminalOrNot == false)
					{
						if (symbol_in_production_body[m].visit_flag.empty() == false)
						{
							TrieTreeNode* v = nullptr;
							if ((v = ptr_point_to_last_visit[symbol_in_production_body[m].symbol_info.symbol].first) == nullptr || v->key_in_trie.next != nullptr)
							{
								++new_add_count;
							}
						}
					}
				}

				TrieTreeNode* run_ptr = nullptr;  //��ǰ���ڴ���Ľڵ�ָ��
				long pro_head_symbol_array_index = 0;  //����ʽ����ͷ�����±���ɵ������е��±꣬�����ڸ��±�λ���϶�Ӧ�±�һ���ǲ���ʽ�������в���ʽͷ���ŵ��±��д��ڵ�ǰ�±����С�±�
				m = 0;
				size_t visit_pos_every_layer = 0;  //ÿһ��ķ���λ��
				bool first_into_nonter = true;  //�Ƿ�Ϊ������ս�����ڲ�ĵ�һ��ѭ��
				bool is_new_add;   //��ʾ��ǰ�ڵ��Ƿ�Ϊ�����ڵ�
				bool move_updown = true;   //��ʾ�Ƿ�Ϊ����һ��ǰ����ĳ�����ս����Ӧ���µ�һ��
				for (; true; )
				{
					if (m < get<0>(q->second).size())
					{
						if (back_flag == true)  //ǰ��������
						{
							if (symbol_in_production_body[m].symbol_info.TerminalOrNot == false)  //��ǰΪ���ս��
							{
								if (symbol_in_production_body[m].visit_flag.empty() == false)  //���ս����ӦTrie��Ϊ��
								{
									pair<TrieTreeNode*, size_t>& p2 = ptr_point_to_last_visit[symbol_in_production_body[m].symbol_info.symbol];
									if (p2.first != nullptr && p2.first->key_in_trie.next == nullptr) //���㲻�ǵ�һ�η��� �����������ڵ�
									{
										if (m == 0 && new_add_count == 0)  //���з��ս�����������ڵ㣬ֱ���˳�
										{
											break;
										}
										++new_add_count;
									}

									if (new_add_first_before_num == 0 && new_add_count - 1 == 0)   //�����Ҫ�ڵ�ǰ����ת����һ�������ڵ��Լ�֦������
									{
										if (visit_pos_every_layer == 0 && move_updown)
										{
											if (p2.first == nullptr || p2.first->key_in_trie.next != nullptr)
											{
												is_new_add = false;
											}
										}
									}

									TrieTree& temp2 = *((*result)[symbol_in_production_body[m].symbol_info.symbol]);
									if (new_add_first_before_num == 0/*ջ֮�ϲ����������ڵ�*/ ? new_add_count - 1 > 0 /*��ǰ�����¸����������ڵ�*/ || is_new_add : true)   //û�б�Ҫ������������Ż�
									{
										if (visit_pos_every_layer == 0 && first_into_nonter)  //�״ν�����ս�����ڲ���Ӧ��ʼ��run_ptrΪ�ò��һ���ڵ�
										{
											run_ptr = temp2.get_first_leaf_list_node_ptr();
										}
										size_t _size = run_ptr->key_in_trie.key.size();   //��¼��ǰ�ڵ��ӦfirstK����

										if (pro_head_symbol_array_index != get<2>(q->second).size())
										{
											if (get<2>(q->second)[pro_head_symbol_array_index] == m)  //����Ĳ���ʽ���з��ս��Ϊ����ʽͷ
											{
												current_advance_point_in_pro_head_symbol_indexs = pro_head_symbol_array_index;
												++pro_head_symbol_array_index;
											}
										}
										--new_add_count;

										if (p2.first == nullptr || p2.first->key_in_trie.next != nullptr)  //�����������ڵ�
										{
											if (visit_pos_every_layer == 0)
											{
												if (p2.first != nullptr && p2.first->key_in_trie.next != run_ptr)
												{
													is_new_add = false;
												}
												else
												{
													is_new_add = true;
												}
											}
											else
											{
												if (is_new_add == false)
												{
													if (p2.first->key_in_trie.next == run_ptr)
														is_new_add = true;
												}
											}
										}
										else
										{
											is_new_add = false;
										}

										if (_size + part_first_str_length < K)  //����firstK���벿��firstK��δ�ﵽ����K
										{
											if (get<0>(q->second)[m].visit_flag[visit_pos_every_layer] == false)
											{
												trace_back_stack.push_back(TraceBackInfo(is_new_add, false, visit_pos_every_layer, run_ptr, m));
											}
											else
											{
												if (trace_back_stack.empty() == false && trace_back_stack.back().current_pos_in_stacked_flaged == false)
												{
													trace_back_stack.push_back(TraceBackInfo(is_new_add, false, visit_pos_every_layer, run_ptr, m));
												}
												else
												{
													trace_back_stack.push_back(TraceBackInfo(is_new_add, true, visit_pos_every_layer, run_ptr, m));
												}
											}
											if (is_new_add)
											{
												++new_add_first_before_num;
											}

											part_first_str_length += _size;
											visit_pos_every_layer = 0;
											if (m + 1 < get<0>(q->second).size())
											{
												if (get<0>(q->second)[m + 1].symbol_info.TerminalOrNot == false)
												{
													auto temporary = result->find(get<0>(q->second)[m + 1].symbol_info.symbol);
													if (temporary != result->end())
														run_ptr = temporary->second->get_first_leaf_list_node_ptr();
												}
												move_updown = true;
											}
											++m;
										}
										else
										{    //��ʱӦ����ǰ�ڵ�firstK��ջ�����ϸ���(�ս������ս��firstK)���Ŵ�ƴ��ΪfirstK
											if (trace_back_stack.empty() == false && trace_back_stack.back().current_pos_in_stacked_flaged == false || get<0>(q->second)[m].visit_flag[visit_pos_every_layer] == false)
											{
												vector<string> first_k;
												insertFirstK(trace_back_stack, get<0>(q->second), first_k, m);
												get<0>(q->second)[m].visit_flag[visit_pos_every_layer] = true;

												for (size_t t = 0; t < K - part_first_str_length; ++t)
												{
													first_k.push_back(run_ptr->key_in_trie.key[t]);
												}
												adjustAfterInsert(get<0>(q->second), visit_pos_every_layer, first_k,
													result, ptr_point_to_last_visit, get<2>(q->second), new_add_count, temp, p->first, m, current_advance_point_in_pro_head_symbol_indexs);
											}
											back_flag = false;
										}
									}
									else
									{
										if (p2.first != nullptr)  //��֦,��ת����ǰ���һ�������ڵ�
										{
											run_ptr = p2.first->key_in_trie.next;
										}
										else
										{
											run_ptr = temp2.get_first_leaf_list_node_ptr();
										}
										visit_pos_every_layer = (*result)[get<0>(q->second)[m].symbol_info.symbol]->getSize() - p2.second;
										is_new_add = true;
										move_updown = false;
									}
								}
								else
								{
									if (first_into_nonter)
									{
										break;
									}
									else
									{
										back_flag = false;
										--m;
									}
								}
								if (first_into_nonter)
									first_into_nonter = false;
							}
							else
							{       //ǰ�����ս�����ڲ�
								if (part_first_str_length + 1 == K)  //���ϱ����ս���󲿷�firstK����ǡ��ΪK,�γ�firstK������
								{
									if (trace_back_stack.back().current_pos_in_stacked_flaged == false)
									{
										vector<string> first_k;
										insertFirstK(trace_back_stack, get<0>(q->second), first_k, m);
										first_k.push_back(get<0>(q->second)[m].symbol_info.symbol);
										adjustAfterInsert(get<0>(q->second), visit_pos_every_layer, first_k,
											result, ptr_point_to_last_visit, get<2>(q->second), new_add_count, temp, p->first, m, current_advance_point_in_pro_head_symbol_indexs);
									}
									--m;
									back_flag = false;
								}
								else    //����ǰ��
								{
									if (pro_head_symbol_array_index != get<2>(q->second).size())
									{
										if (get<2>(q->second)[pro_head_symbol_array_index] == m)
										{
											current_advance_point_in_pro_head_symbol_indexs = pro_head_symbol_array_index;
											++pro_head_symbol_array_index;
										}
									}
									if (m + 1 < get<0>(q->second).size())
									{
										if (get<0>(q->second)[m + 1].symbol_info.TerminalOrNot == false)
										{
											auto temporary = result->find(get<0>(q->second)[m + 1].symbol_info.symbol);
											if (temporary != result->end())
												run_ptr = temporary->second->get_first_leaf_list_node_ptr();
										}
									}
									++part_first_str_length;
									++m;
								}
							}
						}
						else
						{
							if (trace_back_stack.empty() == false && trace_back_stack.back().symbol_array_index == m || get<0>(q->second)[m].symbol_info.TerminalOrNot == true)
							{
								if (trace_back_stack.empty() == false && trace_back_stack.back().symbol_array_index == m)
								{
									is_new_add = trace_back_stack.back().is_new_add;
									visit_pos_every_layer = trace_back_stack.back().have_visited_index;
									run_ptr = trace_back_stack.back().firstK_for_symbol;
									if (is_new_add) { --new_add_first_before_num; }
									updateProductHeadRelevant(pro_head_symbol_array_index, current_advance_point_in_pro_head_symbol_indexs, get<2>(q->second), m);
									pair<TrieTreeNode*, size_t>& p2 = ptr_point_to_last_visit[symbol_in_production_body[m].symbol_info.symbol];
									if (p2.second != 0)
										++new_add_count;
									part_first_str_length -= trace_back_stack.back().firstK_for_symbol->key_in_trie.key.size();
									trace_back_stack.pop_back();
								}
								else
								{
									if (m == 0)
										break;
									else
									{
										updateProductHeadRelevant(pro_head_symbol_array_index, current_advance_point_in_pro_head_symbol_indexs, get<2>(q->second), m);
										--part_first_str_length;
										--m;
									}
									continue;
								}
							}

							if (run_ptr->key_in_trie.next == nullptr)
							{
								if (m == 0)
								{
									break;
								}
								else
								{
									--m;
								}
							}
							else
							{
								++visit_pos_every_layer;
								run_ptr = run_ptr->key_in_trie.next;
								back_flag = true;
							}
						}
					}
					else
					{
						if (trace_back_stack.back().current_pos_in_stacked_flaged == false)  //������������ǰ����ʽ��ƴ��firstK,����
						{
							vector<string> first_k;
							insertFirstK(trace_back_stack, get<0>(q->second), first_k, m);
							adjustAfterInsert(get<0>(q->second), visit_pos_every_layer, first_k,
								result, ptr_point_to_last_visit, get<2>(q->second), new_add_count, temp, p->first, m, current_advance_point_in_pro_head_symbol_indexs);
						}
						back_flag = false;
						--m;
					}
				}

				for (map<string, pair<TrieTreeNode*, size_t>>::iterator x = temp_list.begin(); x != temp_list.end(); ++x)
				{
					auto x2 = ptr_point_to_last_visit[x->first];
					x2.first = x->second.first;
					x2.second = x2.second - x->second.second;
				}
			}

			if (o2 != result->end())
			{
				if (size_before_run != o2->second->getSize())
					stopOrNot = false;
			}
			else
			{
				o2 = result->find(p->first);
				if (o2 != result->end())
					stopOrNot = false;
			}
		}
		if (stopOrNot == true)
			break;
	}
	return result;
}

void LALRAutomata::insertFirstK(vector<TraceBackInfo>& trace_back_stack, vector<SymbolInfo>& q, vector<string>& first_k, size_t& m)
{
	for (vector<TraceBackInfo>::iterator v2 = trace_back_stack.begin(); v2 != trace_back_stack.end(); ++v2)
	{
		q[v2->symbol_array_index].visit_flag[v2->have_visited_index] = true;
	}
	size_t a = 0;
	for (size_t t = 0; t < m; ++t)
	{
		if (trace_back_stack.empty() == false && t == trace_back_stack[a].symbol_array_index)
		{
			for (vector<string>::iterator t2 = trace_back_stack[a].firstK_for_symbol->key_in_trie.key.begin(); t2 != trace_back_stack[a].firstK_for_symbol->key_in_trie.key.end(); ++t2)
				first_k.push_back(*t2);
			if (a + 1 != trace_back_stack.size())
				++a;
		}
		else
		{
			first_k.push_back(q[t].symbol_info.symbol);
		}
	}
}
void LALRAutomata::adjustAfterInsert(vector<SymbolInfo>& q, size_t& visit_pos_every_layer, vector<string>& first_k,
	shared_ptr<map<string, shared_ptr<TrieTree>>>& result, map<string, pair<TrieTreeNode*, size_t>>& ptr_point_to_last_visit, vector<size_t>& p, size_t& new_add_count, map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>>& temp, const string& head, size_t& m, long& current_advance_point_in_pro_head_symbol_indexs)
{
	map<string, shared_ptr<TrieTree>>::iterator l = result->find(head);
	if (l == result->end())
	{
		l = result->insert(make_pair(head, make_shared<TrieTree>())).first;
	}

	bool insert_suc_or_not = l->second->insert(first_k);
	if (insert_suc_or_not)
	{
		if (p.empty() != true)
		{
			if (head != AugGraSS && ptr_point_to_last_visit[head].second == 0)
			{
				size_t f;
				if (current_advance_point_in_pro_head_symbol_indexs == -1)
					f = p.size();
				else
					f = p[current_advance_point_in_pro_head_symbol_indexs] == m ? p.size() - current_advance_point_in_pro_head_symbol_indexs : p.size() - current_advance_point_in_pro_head_symbol_indexs - 1;
				for (size_t h = 1; h <= f; ++h) { ++new_add_count; }
			}
		}

		for (map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>>::iterator p3 = temp.begin(); p3 != temp.end(); ++p3)
		{
			for (map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>::iterator q3 = p3->second.begin(); q3 != p3->second.end(); ++q3)
			{
				for (vector<SymbolInfo>::iterator v3 = get<0>(q3->second).begin(); v3 != get<0>(q3->second).end(); ++v3)
				{
					if (v3->symbol_info.symbol == head)
					{
						v3->visit_flag.push_back(false);
					}
				}
			}
		}
		if (head != AugGraSS)
			++ptr_point_to_last_visit[head].second;
	}
}

shared_ptr<map<string, set<string>>> LALRAutomata::calculateFirst()
{
	struct NonTerminalSymbolInfo
	{
		string symbol;
		set<string> new_add_first;
		NonTerminalSymbolInfo(const string& s) :symbol(s), new_add_first() {}
	};
	map<string, map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>> temp;
	//����ʽͷ�ķ��ս��,����ʽ���,����ʽ���е�һ���ս��ǰ�����з��ս��������first,��һ���ս��,ѭ�������������ƽ���, �������е�һ���ս��ǰȫ�����ս���Ƿ�Ϊresult�ؼ��ּ����Ӽ�,����ʽ���е�һ���ս��ǰ�����з��ս������
	shared_ptr<map<string, set<string>>> result = make_shared<map<string, set<string>>>();  //���ս��,first��

	for (map<long, tuple<string, vector<ProductionBodySymbol>, set<string>>>::iterator run = productionSet.begin(); run != productionSet.end(); ++run)
	{
		if (get<1>(run->second).empty() == true || get<1>(run->second)[0].TerminalOrNot == true)
		{
			map<string, set<string>>::iterator q = result->insert(make_pair(get<0>(run->second), set<string>())).first;
			if (get<1>(run->second).empty())
			{
				q->second.insert("");
			}
			else
			{
				q->second.insert(get<1>(run->second)[0].symbol);
			}
		}
		else
		{
			map<string, map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>>::iterator tempit;
			map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>::iterator tempit2;
			tempit = temp.insert(make_pair(get<0>(run->second), map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>())).first;
			tempit2 = tempit->second.insert(make_pair(run->first, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>())).first;

			set<string> test_repeat;
			vector<ProductionBodySymbol>::iterator p;
			for (p = get<1>(run->second).begin(); p != get<1>(run->second).end(); ++p)
			{
				if (p->TerminalOrNot == false)
				{
					if (test_repeat.insert(p->symbol).second == true)
					{
						get<4>(tempit2->second).insert(p->symbol);
						get<0>(tempit2->second).push_back(NonTerminalSymbolInfo(p->symbol));
					}
				}
				else
				{
					get<1>(tempit2->second) = p->symbol;
					break;
				}
			}

			if (p == get<1>(run->second).end())
				get<1>(tempit2->second) = "";
			get<2>(tempit2->second) = 0;
			get<3>(tempit2->second) = false;
		}
	}

	for (map<string, map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>>::iterator run = temp.begin(); run != temp.end(); ++run)
	{
		for (map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>::iterator second_run = run->second.begin(); second_run != run->second.end(); ++second_run)
		{
			for (vector<NonTerminalSymbolInfo>::iterator last_run = get<0>(second_run->second).begin(); last_run != get<0>(second_run->second).end(); ++last_run)
			{
				if (result->find(last_run->symbol) != result->end())
				{
					if (last_run->symbol != run->first)
						last_run->new_add_first.insert((*result)[last_run->symbol].begin(), (*result)[last_run->symbol].end());
				}
			}
		}
	}

	while (true)
	{
		bool stopOrNot = true;
		for (map<string, map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>>::iterator p = temp.begin(); p != temp.end(); ++p)
		{
			map<string, set<string>>::iterator first_set = result->insert(make_pair(p->first, set<string>())).first;
			set<string> compute_difference_set_result;

			for (map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>::iterator s = p->second.begin(); s != p->second.end(); ++s)
			{
				if (get<3>(s->second) == false && isSubSet(*result, get<4>(s->second)) == true)
				{
					get<3>(s->second) = true;
				}
				else if (get<3>(s->second) == false)
				{
					stopOrNot = false;
					continue;
				}

				vector<string>::size_type q = 0;
				for (; q < get<0>(s->second).size(); ++q)
				{
					if (get<0>(s->second)[q].symbol != p->first)
					{
						computeDifferenceSet(get<0>(s->second)[q].new_add_first, first_set->second, compute_difference_set_result, true);  //
					}

					if (get<2>(s->second) == q || get<2>(s->second) == q + 1)
					{
						if (get<2>(s->second) == q)
							++(get<2>(s->second));

						if (get<0>(s->second)[q].symbol != p->first)
						{
							if (*((*result)[get<0>(s->second)[q].symbol].begin()) != "")
								break;
						}
						else
						{
							if (first_set->second.empty() || *(first_set->second.begin()) != "")
								break;
						}
					}
				}

				if (q == get<0>(s->second).size())
				{
					if (get<1>(s->second) != "")
					{
						if (get<2>(s->second) == get<0>(s->second).size())
						{
							++(get<2>(s->second));
							bool temp = first_set->second.insert(get<1>(s->second)).second;
							if (temp)
							{
								compute_difference_set_result.insert(get<1>(s->second));
							}
						}
					}
					else
					{
						if (get<2>(s->second) == get<0>(s->second).size())
						{
							++(get<2>(s->second));
							bool temp = first_set->second.insert("").second;
							if (temp)
							{
								compute_difference_set_result.insert("");
							}
						}
					}
				}
			}

			if (first_set->second.empty() == true)
			{
				result->erase(first_set);
			}
			else
			{
				if (compute_difference_set_result.empty() != true)
				{
					for (map<string, map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>>::iterator m = temp.begin(); m != temp.end(); ++m)
					{
						if (m != p)
						{
							for (map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>::iterator v = m->second.begin(); v != m->second.end(); ++v)
							{
								for (vector<NonTerminalSymbolInfo>::iterator k = get<0>(v->second).begin(); k != get<0>(v->second).end(); ++k)
								{
									if (k->symbol == p->first)
									{
										k->new_add_first.insert(compute_difference_set_result.begin(), compute_difference_set_result.end());   //
										break;
									}
								}
							}
						}
					}
					stopOrNot = false;
				}
			}
		}
		if (stopOrNot == true)
			break;
	}
	return result;
}

shared_ptr<set<string>> LALRAutomata::calculateFirst(const vector<ProductionBodySymbol>& Pro, vector<ProductionBodySymbol>::size_type left, vector<ProductionBodySymbol>::size_type right)
{
	shared_ptr<set<string>> result = make_shared<set<string>>();
	if (left <= right)
	{
		for (vector<ProductionBodySymbol>::size_type temp = left; temp <= right; ++temp)
		{
			if (Pro[temp].TerminalOrNot == true)
			{
				result->insert(Pro[temp].symbol);
				return result;
			}
			else
			{
				set<string>::const_iterator p;
				if ((p = (*first)[Pro[temp].symbol].find("")) == (*first)[Pro[temp].symbol].cend())
				{
					result->insert((*first)[Pro[temp].symbol].cbegin(), (*first)[Pro[temp].symbol].cend());
					return result;
				}
				else
				{
					result->insert(++p, (*first)[Pro[temp].symbol].cend());
				}
			}
		}
		result->insert("");
	}
	return result;
}