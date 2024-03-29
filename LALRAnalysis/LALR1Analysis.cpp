#include "pch.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <memory>
#include <functional>
#include <deque>
#include <fstream>
#include <stack>
#include <algorithm>
#include <time.h>
using namespace std;
#include "Priority_Queue.h"
#include "TrieTree.h"
#include "DirectedGraph.h"
#include "LALRState.h"  
#include "LALRTableItem.h"
#include "LALRAutomata.h"

void output(LALRAutomata &test, ofstream &output)  //将语法分析表，LALR自动机状态和其他文法信息输出至文件
{
	cout << "非终结符：";
	output << "非终结符:,";
	for (set<string>::iterator temp = test.Nonterminal.begin(); temp != test.Nonterminal.end(); ++temp)
	{
		cout << *temp << " ";
		output << *temp << ",";
	}
	cout << endl;
	output << endl;
	cout << "终结符：";
	output << "终结符:,";
	for (set<string>::iterator temp = test.terminnal.begin(); temp != test.terminnal.end(); ++temp)
	{
		cout << *temp << " ";
		output << *temp << ",";
	}
	cout << endl;
	output << endl;
	cout << "原文法开始符号:" << test.StartSymbol << endl;
	output << "原文法开始符号:," << test.StartSymbol << endl;
	cout << "增广文法开始符号:" << test.AugGraSS << endl;
	output << "增广文法开始符号:," << test.AugGraSS << endl;
	cout << endl;
	output << endl;
	cout << "产生式:" << endl;
	output << "产生式:" << endl;
	for (map<long, tuple<string, vector<LALRAutomata::ProductionBodySymbol>, set<string>>>::iterator temp = test.productionSet.begin(); temp != test.productionSet.end(); ++temp)
	{
		cout << "产生式" << temp->first << ":";
		output << "产生式" << temp->first << ":,";
		cout << get<0>(temp->second) << "->";
		output << get<0>(temp->second) << ",->,";
		for (auto p = get<1>(temp->second).begin(); p != get<1>(temp->second).end(); ++p)
		{
			cout << p->symbol;
			output << p->symbol << ",";
		}
		cout << "非终结符号集合:";
		output << ",非终结符号集合:,";
		if (get<2>(temp->second).begin() == get<2>(temp->second).end())
		{
			cout << "无";
			output << "无,";
		}
		for (set<string>::iterator p = get<2>(temp->second).begin(); p != get<2>(temp->second).end(); ++p)
		{
			cout << *p << " ";
			output << *p << ",";
		}
		cout << endl;
		output << endl;
	}

	cout << endl;
	output << endl;
	cout << "非终结符号对应的产生式编号:" << endl;
	output << "非终结符号对应的产生式编号:" << endl;
	for (map<string, set<long>>::iterator temp = test.TerToPro.begin(); temp != test.TerToPro.end(); ++temp)
	{
		cout << "非终结符号:" << temp->first << " ";
		output << "非终结符号:" << temp->first << ",";
		for (set<long>::iterator p = temp->second.begin(); p != temp->second.end(); ++p)
		{
			cout << *p << " ";
			output << *p << ",";
		}
		cout << endl;
		output << endl;
	}

	cout << endl;
	output << endl;
	cout << "各非终结符first集:" << endl;
	output << "各非终结符first集:" << endl;
	for (map<string, set<string>>::iterator temp = test.first->begin(); temp != test.first->end(); ++temp)
	{
		cout << "非终结符" << temp->first << ":";
		output << "非终结符" << temp->first << ":,";
		for (set<string>::iterator p = temp->second.begin(); p != temp->second.end(); ++p)
		{
			cout << *p << " ";
			output << *p << ",";
		}
		cout << endl;
		output << endl;
	}

	cout << endl;
	output << endl;
	/*cout << "各非终结符firstK集:" << endl;
	output << "各非终结符firstK集:" << endl;
	for (map<string, shared_ptr<TrieTree>>::iterator temp = test.firstK->begin(); temp != test.firstK->end(); ++temp)
	{
		cout << "非终结符" << temp->first << ":";
		output << "非终结符" << temp->first << ":,";
		for (TrieTreeNode *p = temp->second->get_first_leaf_list_node_ptr(); p != nullptr; p = p->key_in_trie.next)
		{
			cout << endl;
			output << endl;
			cout << "[";
			output << "[,";
			for (vector<string>::iterator q = p->key_in_trie.key.begin(); q != p->key_in_trie.key.end(); )
			{
				cout << *q;
				output << *q;
				++q;
				if (q != p->key_in_trie.key.end())
				{
					cout << ",";
					output << ",";
				}
			}
			cout << "]";
			output << ",]";
		}
		cout << endl;
		output << endl;
	}*/
	
	cout << endl;
	output << endl;
	cout << "各非终结符follow集:" << endl;
	output << "各非终结符follow集:" << endl;
	for (map<string, set<string>>::iterator temp = test.follow->begin(); temp != test.follow->end(); ++temp)
	{
		cout << "非终结符" << temp->first << ":";
		output << "非终结符" << temp->first << ":,";
		for (set<string>::iterator p = temp->second.begin(); p != temp->second.end(); ++p)
		{
			cout << *p << " ";
			output << *p << ",";
		}
		cout << endl;
		output << endl;
	}

	cout << endl;
	output << endl;
	/*cout << "各非终结符followK集:" << endl;
	output << "各非终结符followK集:" << endl;
	for (map<string, shared_ptr<TrieTree>>::iterator temp = test.followK->begin(); temp != test.followK->end(); ++temp)
	{
		cout << "非终结符" << temp->first << ":";
		output << "非终结符" << temp->first << ":,";
		for (TrieTreeNode *p = temp->second->get_first_leaf_list_node_ptr(); p != nullptr; p = p->key_in_trie.next)
		{
			cout << endl;
			output << endl;
			cout << "[";
			output << "[,";
			for (vector<string>::iterator q = p->key_in_trie.key.begin(); q != p->key_in_trie.key.end(); )
			{
				cout << *q;
				output << *q;
				++q;
				if (q != p->key_in_trie.key.end())
				{
					cout << ",";
					output << ",";
				}
			}
			cout << "]";
			output << ",]";
		}
		cout << endl;
		output << endl;
	}*/

	cout << endl;
	output << endl;
	cout << "LALR自动机状态:" << endl;
	output << "LALR自动机状态:" << endl;
	for (vector<Graph<LALRState, string>::GraphVertexNode *>::size_type i = 0; i < test.SetOfVertex.size(); ++i)
	{
		cout << "状态" << i << ":" << endl;
		output << "状态" << i << ":" << endl;
		for (auto p = test.SetOfVertex[i]->Vertexdatafield->kernel.begin(); p != test.SetOfVertex[i]->Vertexdatafield->kernel.end(); ++p)
		{
			cout << "产生式" << p->first << ":" << get<0>(test.productionSet[p->first]) << "->";
			output << "产生式" << p->first << ":," << get<0>(test.productionSet[p->first]) << ",->,";
			for (auto m = get<1>(test.productionSet[p->first]).begin(); m != get<1>(test.productionSet[p->first]).end(); ++m)
			{
				cout << m->symbol;
				output << m->symbol << ",";
			}
			cout << endl;
			output << endl;
			for (auto m = p->second.begin(); m != p->second.end(); ++m)
			{
				cout << "点号位置" << m->first << " ";
				output << "点号位置" << m->first << ",";
				cout << "向前看符号集合 ";
				output << "向前看符号集合,";
				for (auto v = m->second.begin(); v != m->second.end(); ++v)
				{
					cout << *v << " ";
					output << *v << ",";
				}
				cout << endl;
				output << endl;
			}
		}

		for (auto p = test.SetOfVertex[i]->Vertexdatafield->nonkernel.begin(); p != test.SetOfVertex[i]->Vertexdatafield->nonkernel.end(); ++p)
		{
			cout << "产生式" << p->first << ":" << get<0>(test.productionSet[p->first]) << "->";
			output << "产生式" << p->first << ":," << get<0>(test.productionSet[p->first]) << ",->,";
			for (auto m = get<1>(test.productionSet[p->first]).begin(); m != get<1>(test.productionSet[p->first]).end(); ++m)
			{
				cout << m->symbol;
				output << m->symbol << ",";
			}
			cout << endl;
			output << endl;
			cout << "点号位置" << p->second.dotposition << " ";
			output << "点号位置" << p->second.dotposition << ",";
			cout << "向前看符号集合 ";
			output << "向前看符号集合,";
			for (auto v = p->second.ForwardLookingSign.begin(); v != p->second.ForwardLookingSign.end(); ++v)
			{
				cout << *v << " ";
				output << *v << ",";
			}
			cout << endl;
			output << endl;
		}
	}

	cout << endl;
	output << endl;
	cout << "LALR语法分析表:" << endl;
	output << "LALR语法分析表:" << endl;
	vector<string> temp(test.LALRTable.first->size());
	for (auto p = test.LALRTable.first->begin(); p != test.LALRTable.first->end(); ++p)
	{
		temp[p->second] = p->first;
	}
	output << " ,";
	for (const auto &m : temp)
	{
		cout << m << " ";
		output << m << ",";
	}
	cout << endl;
	output << endl;
	for (vector<vector<LALRTableItem>>::size_type i = 0; i < test.LALRTable.second->size(); ++i)
	{
		cout << "状态" << i << " ";
		output << "状态" << i << ",";
		for (const auto &m : (*(test.LALRTable.second))[i])
		{
			if (m.ActionType == m.MOVE)
			{
				cout << "m" << m.LALRStateNumber << " ";
				output << "m" << m.LALRStateNumber << ",";
			}
			else if (m.ActionType == m.REDUCTION)
			{
				cout << "r" << m.production << " ";
				output << "r" << m.production << ",";
			}
			else if (m.ActionType == m.ACCEPT)
			{
				cout << "A ";
				output << "A,";
			}
			else
			{
				cout << "ERR ";
				output << "ERR,";
			}
		}
		cout << endl;
		output << endl;
	}
}
int main()
{
	size_t k = 2;
	ifstream input("inputexample1.txt");   //文法描述
	ofstream dataoutput("output.txt");   //输出结果
	LALRAutomata test(input, k);  //根据和input绑定的文法描述生成LALR(1)语法分析信息
	output(test, dataoutput);  //将语法分析信息输出至和dataoutput绑定的文件
	system("pause");
	return 0;
}

