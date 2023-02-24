#include "pch.h"
#include <fstream>
#include "LALRAutomata.h"
#include "assistfunction.h"
#include "Priority_Queue.h"
void LALRAutomata::initgrammar(ifstream& input)  //根据从input读入的文法信息初始化Nonterminal、terminnal、StartSymbol、AugGraSS、productionSet以及TerToPro
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
	cout << "开始生成LALR(1)语法分析表" << endl;;
	clock_t start = clock();
	initgrammar(input);
	first = calculateFirst();  //计算first集
	follow = calculateFollow();   //计算follow集
	firstK = calculateFirstK(k);
	followK = calculateFollowK(k);
	constructLRKernel();
	LALRTable = constructLALR();   //构造LALR(1)语法分析表
	clock_t end = clock();
	cout << "语法分析表生成完成,共用时" << end - start << "毫秒" << endl;
}

pair<shared_ptr<map<string, int>>, shared_ptr<vector<vector<LALRTableItem>>>> LALRAutomata::constructLALR()
{
	shared_ptr<map<string, int>> symbolToIndex = make_shared<map<string, int>>();
	{
		int count = 0;
		setToMap(terminnal, *symbolToIndex, count);    //建立文法符号到语法分析表第二维的映射
		setToMap(Nonterminal, *symbolToIndex, count);
	}
	shared_ptr<vector<vector<LALRTableItem>>>LALRTablePtr = make_shared<vector<vector<LALRTableItem>>>(SetOfVertex.size(), vector<LALRTableItem>((*symbolToIndex).size()));
	for (vector<Graph<LALRState, string>::GraphVertexNode*>::size_type i = 0; i < SetOfVertex.size(); ++i)    //根据LR(0)项集族向语法分析表中填入移入动作
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
	vector<map<long, map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>>> FLKSymbolTran;
	for (vector<Graph<LALRState, string>::GraphVertexNode*>::size_type i = 0; i < SetOfVertex.size(); ++i)   //计算各LR(0)内核项自发生成的向前看符号并确定LR(0)内核项集之间向前看符号的传播关系
	{
		FLKSymbolTran.push_back(map<long, map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>>());
		for (map<long, map<int, set<string>>>::iterator p = SetOfVertex[i]->Vertexdatafield->kernel.begin(); p != SetOfVertex[i]->Vertexdatafield->kernel.end(); ++p)
		{
			map<long, map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>>::iterator x1 = FLKSymbolTran.back().insert(make_pair(p->first, map<int, map<vector<Graph<LALRState, string>::GraphVertexNode*>::size_type, map<long, set<int>>>>())).first;
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
				FLKSymbolTran.back().erase(x1);
			}
		}
	}

	(SetOfVertex[start]->Vertexdatafield->kernel[*(TerToPro[AugGraSS].begin())])[0].insert("$");  //为增广文法新增的产生式填写向前看符号输入结束符$,它是自发生成的
	while (true)    //不断扫描所有LR(0)项集传播向前看符号，直到某一轮再也没有向前看符号被传播为止
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
		calculateClosureLALR(SetOfVertex[i]->Vertexdatafield->kernel, SetOfVertex[i]->Vertexdatafield->nonkernel);  //为每个LALR(1)内核项集计算闭包得到LALR(1)项集
		for (map<long, map<int, set<string>>>::iterator p = SetOfVertex[i]->Vertexdatafield->kernel.begin(); p != SetOfVertex[i]->Vertexdatafield->kernel.end(); ++p)   //为LALR(1)语法分析表填入归约动作
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
							cout << "ERROR:移入归约冲突" << endl;
							cout << "状态" << i << "要求在文法符号" << temp->first << "下移入状态" << (*LALRTablePtr)[i][temp->second].LALRStateNumber;
							cout << "同时状态" << i << "要求在文法符号" << temp->first << "下用产生式" << p->first << "归约" << endl;
							continue;
						}
						else if ((*LALRTablePtr)[i][temp->second].ActionType == LALRTableItem::action::REDUCTION)
						{
							cout << "ERROR:归约归约冲突" << endl;
							cout << "状态" << i << "要求在文法符号" << temp->first << "下用产生式" << (*LALRTablePtr)[i][temp->second].production << "归约" << endl;
							cout << "同时状态" << i << "要求在文法符号" << temp->first << "下用产生式" << p->first << "归约" << endl;
							continue;
						}
						(*LALRTablePtr)[i][temp->second].ActionType = LALRTableItem::action::REDUCTION;   //移入归约,归约归约冲突处理
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
						cout << "ERROR:移入归约冲突" << endl;
						cout << "状态" << i << "要求在文法符号" << temp1->first << "下移入状态" << (*LALRTablePtr)[i][temp1->second].LALRStateNumber;
						cout << "同时状态" << i << "要求在文法符号" << temp1->first << "下用产生式" << temp->first << "归约" << endl;
						continue;
					}
					else if ((*LALRTablePtr)[i][temp1->second].ActionType == LALRTableItem::action::REDUCTION)
					{
						cout << "ERROR:归约归约冲突" << endl;
						cout << "状态" << i << "要求在文法符号" << temp1->first << "下用产生式" << (*LALRTablePtr)[i][temp1->second].production << "归约" << endl;
						cout << "同时状态" << i << "要求在文法符号" << temp1->first << "下用产生式" << temp->first << "归约" << endl;
						continue;
					}
					(*LALRTablePtr)[i][temp1->second].ActionType = LALRTableItem::action::REDUCTION;    //移入归约,归约归约冲突处理
					(*LALRTablePtr)[i][temp1->second].production = temp->first;
				}
			}
		}
	}
	(*LALRTablePtr)[((*LALRTablePtr)[start][(*symbolToIndex)[StartSymbol]].LALRStateNumber)][(*symbolToIndex)["$"]].ActionType = LALRTableItem::action::ACCEPT;  //向语法分析表中填入接受项
	accept = (*LALRTablePtr)[start][(*symbolToIndex)[StartSymbol]].LALRStateNumber;
	(*LALRTablePtr)[((*LALRTablePtr)[start][(*symbolToIndex)[StartSymbol]].LALRStateNumber)][(*symbolToIndex)["$"]].NULLLable = "";
	return { symbolToIndex, LALRTablePtr };  //返回LALR(1)语法分析表
}

void LALRAutomata::constructLRKernel()   //计算出的LR(0)项集族保存在继承而来的有向图中
{
	struct Vertex
	{
		LALRState* state = new LALRState();
		vector<Graph<LALRState, string>::GraphVertexNode*>::size_type index = 0;   //LALR状态及其对应序号
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

void LALRAutomata::calculateClosureLR(map<long, map<int, set<string>>>& kernelset, map<long, LALRState::attribute>& nonkernelset)  //kernelset的arrtribute属性的ForwardLookingSign为空,nonkernelset同样,由内核项算出的闭包项保存在nonkernelset中
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
	Priority_Queue<pair<long, LALRState::attribute>> workqueue(function<bool(const pair<long, LALRState::attribute>&, const pair<long, LALRState::attribute>&)>([](const pair<long, LALRState::attribute>& left, const pair<long, LALRState::attribute>& right)->bool {return left.first < right.first; }));  //使用lambda表达式根据产生式编号维护优先级队列
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
	map<string, pair<GeneratingCycles, map<string, FollowKAndDependency>::iterator>> pre_and_cur_cycle_finish_followK_compute_info;  //非终结符,followK为本轮生成还是上一轮生成,指向follow_k_set_and_dependency_for_unterminal_symbol中对应项迭代器
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

				if (i == get<1>(p->second).size() - 1)    //非终结符后无符号，产生式头加入直接依赖集
				{
					it->second.direct_dependency.insert(get<0>(p->second));
				}
				else
				{
					shared_ptr<TrieTree> q = calculateFirstK(get<1>(p->second), i + 1, get<1>(p->second).size() - 1, K);  //非终结符后有符号串,计算其firstK
					for (TrieTreeNode* run = q->get_first_leaf_list_node_ptr(); run != nullptr; run = run->key_in_trie.next)
					{
						if (run->key_in_trie.key.empty() == true)   //firstK中有空串,产生式头加入直接依赖集
						{
							it->second.direct_dependency.insert(get<0>(p->second));
						}
						else if (run->key_in_trie.key.size() < K)  //firstK中有长度小于K的串，该串和产生式头加入间接依赖集
						{
							map<string, shared_ptr<TrieTree>>::iterator tempit = it->second.undirect_dependency.insert(make_pair(get<0>(p->second), make_shared<TrieTree>())).first;
							tempit->second->insert(run->key_in_trie.key);
						}
						else    //firstK中有长度为K的串,该串直接加入结果集
						{
							it->second.FollowK->insert(run->key_in_trie.key);
							it->second.followK_new_add->insert(run->key_in_trie.key);
						}
					}
				}
			}
		}
	}
	//去除直接依赖中和被依赖符号相等的项，若直接依赖和间接依赖均为空,则followK已求出
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

			if (n != pre_and_cur_cycle_finish_followK_compute_info.end())  //删除不会被用到的pre_and_cur_cycle_finish_followK_compute_info中的项
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
	map<string, tuple<set<string>, set<string>, set<string>, bool>> temp;  //非终结符,当前follow，新增follow,依赖符号,是否计算完成
	map<string, pair<GeneratingCycles, map<string, tuple<set<string>, set<string>, set<string>, bool>>::iterator>> pre_and_cur_cycle_finish_follow_compute_info;  //非终结符 产生轮次 temp中对应项迭代器
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
		bool back_flag = true;// 回溯标志
		size_t part_first_str_length = 0;//部分firstK长度

		TrieTreeNode* run_ptr = nullptr;
		m = 0;
		size_t visit_pos_every_layer = 0;
		bool first_into_nonter = true;
		for (; true; )   //这里计算符号串firstK是对计算非终结符firstK算法的简化
		{
			if (m < get<0>(production_info).size())
			{
				if (back_flag == true)  //前进至本层
				{
					if (get<0>(production_info)[m].TerminalOrNot == false)  //当前为非终结符
					{
						pair<TrieTreeNode*, size_t>& p2 = ptr_point_to_last_visit[get<0>(production_info)[m].symbol];
						TrieTree& temp2 = (*(*firstK)[get<0>(production_info)[m].symbol]);

						if (visit_pos_every_layer == 0 && first_into_nonter)
						{
							run_ptr = temp2.get_first_leaf_list_node_ptr();
						}
						size_t _size = run_ptr->key_in_trie.key.size();

						if (_size + part_first_str_length < K)  //本层firstK并入后未达到长度K
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
	map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>> temp;  //产生式头的非终结符号,产生式编号,产生式体符号及相关信息,产生式体中所有非终结符号集合,和产生式头相同的体中非终结符号下标集合
	shared_ptr<map<string, shared_ptr<TrieTree>>> result = make_shared<map<string, shared_ptr<TrieTree>>>();//保存firstK集的数据结构
	map<string, pair<TrieTreeNode*, size_t>> ptr_point_to_last_visit;  //非终结符号,pair<指向非终结符最后一个非新增firstK节点的指针,该非终结符新增firstK节点个数>

	for (map<long, tuple<string, vector<ProductionBodySymbol>, set<string>>>::iterator run = productionSet.begin(); run != productionSet.end(); ++run)
	{
		if (get<0>(run->second) != AugGraSS)
			ptr_point_to_last_visit.insert(make_pair(get<0>(run->second), make_pair(nullptr, 0)));
		if (get<1>(run->second).empty() == true || get<2>(run->second).empty() == true)  //非终结符推导终结符号串则前K个终结符加入FirstK集,不足K全部加入，推导空串则加入空集
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
		else   //前k个符号均为终结符则直接将其加入firstK,否则将当前产生式相关信息加入temp
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

	//对于firstK集不为空的非终结符更新temp中产生式体中对应于该非终结符的firstK集
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

	//对firstK集不为空的非终结符，更新ptr_point_to_last_visit中对应项
	for (map<string, shared_ptr<TrieTree>>::iterator p = result->begin(); p != result->end(); ++p)
	{
		auto p2 = ptr_point_to_last_visit.find(p->first);
		if (p2 != ptr_point_to_last_visit.end())
			p2->second.second = p->second->getSize();
	}

	while (true)
	{
		bool stopOrNot = true;   //标记是否有某个非终结符firstK发生改变
		for (map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>>::iterator p = temp.begin(); p != temp.end(); ++p)
		{
			auto o2 = result->find(p->first);
			size_t size_before_run;
			if (o2 != result->end())   //记录迭代前当前非终结符firstK集大小
			{
				size_before_run = o2->second->getSize();
			}
			else
			{
				size_before_run = 0;
			}

			for (map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>::iterator q = p->second.begin(); q != p->second.end(); ++q)  //记录当前产生式体中各非终结符firstK集尾节点指针和新增节点数
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

				vector<SymbolInfo>::size_type m = 0;      //当前正在处理下标
				vector<TraceBackInfo> trace_back_stack;//回溯工作栈
				vector<SymbolInfo>& symbol_in_production_body = get<0>(q->second);
				size_t new_add_first_before_num = 0;//栈中指针指向的各节点中为新增节点的节点数
				bool back_flag = true;// 回溯标志
				long current_advance_point_in_pro_head_symbol_indexs = -1;// 当前下标相对于产生式体中产生式头符号位置的位置
				size_t new_add_count = 0;//当前处理的下标之下有新增节点的层数
				size_t part_first_str_length = 0;//目前已形成的部分firstK串长度
				for (; m < symbol_in_production_body.size(); ++m)  //根据各层有无新增节点初始化new_add_count
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

				TrieTreeNode* run_ptr = nullptr;  //当前正在处理的节点指针
				long pro_head_symbol_array_index = 0;  //产生式体中头符号下标组成的数组中的下标，数组在该下标位置上对应下标一定是产生式体中所有产生式头符号的下标中大于当前下标的最小下标
				m = 0;
				size_t visit_pos_every_layer = 0;  //每一层的访问位置
				bool first_into_nonter = true;  //是否为进入非终结符所在层的第一轮循环
				bool is_new_add;   //表示当前节点是否为新增节点
				bool move_updown = true;   //表示是否为从上一层前进至某个非终结符对应的新的一层
				for (; true; )
				{
					if (m < get<0>(q->second).size())
					{
						if (back_flag == true)  //前进至本层
						{
							if (symbol_in_production_body[m].symbol_info.TerminalOrNot == false)  //当前为非终结符
							{
								if (symbol_in_production_body[m].visit_flag.empty() == false)  //非终结符对应Trie不为空
								{
									pair<TrieTreeNode*, size_t>& p2 = ptr_point_to_last_visit[symbol_in_production_body[m].symbol_info.symbol];
									if (p2.first != nullptr && p2.first->key_in_trie.next == nullptr) //本层不是第一次访问 本层无新增节点
									{
										if (m == 0 && new_add_count == 0)  //所有非终结符均无新增节点，直接退出
										{
											break;
										}
										++new_add_count;
									}

									if (new_add_first_before_num == 0 && new_add_count - 1 == 0)   //辨别需要在当前层跳转至第一个新增节点以剪枝的情形
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
									if (new_add_first_before_num == 0/*栈之上不包含新增节点*/ ? new_add_count - 1 > 0 /*当前层以下各层有新增节点*/ || is_new_add : true)   //没有必要查表，将来可做优化
									{
										if (visit_pos_every_layer == 0 && first_into_nonter)  //首次进入非终结符所在层则应初始化run_ptr为该层第一个节点
										{
											run_ptr = temp2.get_first_leaf_list_node_ptr();
										}
										size_t _size = run_ptr->key_in_trie.key.size();   //记录当前节点对应firstK长度

										if (pro_head_symbol_array_index != get<2>(q->second).size())
										{
											if (get<2>(q->second)[pro_head_symbol_array_index] == m)  //本层的产生式体中非终结符为产生式头
											{
												current_advance_point_in_pro_head_symbol_indexs = pro_head_symbol_array_index;
												++pro_head_symbol_array_index;
											}
										}
										--new_add_count;

										if (p2.first == nullptr || p2.first->key_in_trie.next != nullptr)  //本层有新增节点
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

										if (_size + part_first_str_length < K)  //本层firstK并入部分firstK后未达到长度K
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
										{    //此时应将当前节点firstK和栈内以上各层(终结符或非终结符firstK)符号串拼接为firstK
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
										if (p2.first != nullptr)  //剪枝,跳转至当前层第一个新增节点
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
							{       //前进至终结符所在层
								if (part_first_str_length + 1 == K)  //加上本层终结符后部分firstK长度恰好为K,形成firstK并回溯
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
								else    //继续前进
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
						if (trace_back_stack.back().current_pos_in_stacked_flaged == false)  //处理完整个当前产生式，拼接firstK,回溯
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
	//产生式头的非终结符,产生式编号,产生式体中第一个终结符前的所有非终结符及新增first,第一个终结符,循环过程中最新推进点, 产生体中第一个终结符前全部非终结符是否为result关键字集合子集,产生式体中第一个终结符前的所有非终结符集合
	shared_ptr<map<string, set<string>>> result = make_shared<map<string, set<string>>>();  //非终结符,first集

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
					for (map<string, map<long, tuple<vector<NonTerminalSymbolInfo>, string, vector<NonTerminalSymbolInfo>::size_type, bool, set<string>>>>::iterator m = temp.begin(); m != p; ++m)
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