#pragma once
#include "DirectedGraph.h"
#include "LALRState.h"
#include "TrieTree.h"
#include "LALRTableItem.h"
class LALRAutomata : public  Graph<LALRState, string>  //LALR(1)自动机类，自动机用有向图表示，该有向图来自Graph类
{
	friend class RELALRParsing;
	friend void output(LALRAutomata& test, ofstream& output);
public:
	struct ProductionBodySymbol   //产生式体中的文法符号
	{
		string symbol;   //终结符或非终结符
		bool TerminalOrNot;  //true终结符,false非终结符
		ProductionBodySymbol() = default;
		ProductionBodySymbol(string s, bool T) :symbol(s), TerminalOrNot(T) {}
	};

	struct SymbolInfo
	{
		ProductionBodySymbol symbol_info;
		vector<bool> visit_flag;//每一层非终结符firstK节点访问标记
		SymbolInfo(const ProductionBodySymbol& P) :symbol_info(P) {}
		SymbolInfo() = default;
	};

	struct TraceBackInfo
	{
		bool is_new_add;
		bool current_pos_in_stacked_flaged;
		vector<bool>::size_type have_visited_index;
		TrieTreeNode* firstK_for_symbol;
		size_t symbol_array_index;
		TraceBackInfo(bool i, bool c, vector<bool>::size_type h, TrieTreeNode* f, size_t s) :is_new_add(i), current_pos_in_stacked_flaged(c), have_visited_index(h), firstK_for_symbol(f), symbol_array_index(s) {}
	};

	struct TraceStackNode
	{
		vector<bool>::size_type have_visited_index;
		TrieTreeNode* firstK_for_symbol;
		size_t symbol_array_index;
		TraceStackNode(vector<bool>::size_type h, TrieTreeNode* f, size_t s) :have_visited_index(h), firstK_for_symbol(f), symbol_array_index(s) {}
	};
	LALRAutomata(ifstream& input, size_t k);
	static bool isSubSet(const map<string, set<string>>& left, const set<string>& right);  //判断right是否为left键集的子集,返回true是返回false不是
private:
	shared_ptr<map<string, shared_ptr<TrieTree>>> calculateFirstK(size_t K);
	shared_ptr<map<string, set<string>>> calculateFirst(); //构造各非终结符及其first集的映射表
	shared_ptr<set<string>> calculateFirst(const vector<ProductionBodySymbol>& Pro, vector<ProductionBodySymbol>::size_type left, vector<ProductionBodySymbol>::size_type right); //返回给定文法符号串的first集
	shared_ptr<map<string, shared_ptr<TrieTree>>> calculateFollowK(size_t K);
	shared_ptr<map<string, set<string>>> calculateFollow(); //构造各非终结符及其follow集的映射表
	void calculateClosureLALR(map<long, map<int, set<string>>>& kernelset, map<long, LALRState::attribute>& nonkernelset); //第一个参数内核项集，由内核项集生成非内核项构成的闭包存放至第二个参数,函数功能为计算LALR内核项集闭包，计算开始前nonkernelset会被清空
	void calculateClosureLR(map<long, map<int, set<string>>>& kernelset, map<long, LALRState::attribute>& nonkernelset);  //计算LR(0)内核项集(存放于kernelset)闭包(只包括非内核项)，计算结果存放至nonkernelset，计算开始前nonkernelset会被清空
	void constructLRKernel(); //构造LR(0)项集族，每个项集只包含LR(0)内核项
	pair<shared_ptr<map<string, int>>, shared_ptr<vector<vector<LALRTableItem>>>> constructLALR(); //构造LALR(1)项集族,返回的pair第一分量为文法符号和LALR语法分析表列表映射关系的智能指针，第二分量为指向LALR语法分析表的智能指针
	void initgrammar(ifstream& input); //初始化文法各组成要素
	static void computeDifferenceSet(set<string>& left_operand, set<string>& right_operand, set<string>& result, bool clear_or_not);
	void adjustAfterInsert(vector<SymbolInfo>& q, size_t& visit_pos_every_layer, vector<string>& first_k,
		shared_ptr<map<string, shared_ptr<TrieTree>>>& result, map<string, pair<TrieTreeNode*, size_t>>& ptr_point_to_last_visit, vector<size_t>& p, size_t& new_add_count, map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>>& temp, const string& head, size_t& m, long& current_advance_point_in_pro_head_symbol_indexs);
	void insertFirstK(vector<TraceBackInfo>& trace_back_stack, vector<SymbolInfo>& q, vector<string>& first_k, size_t& m);
	shared_ptr<TrieTree> calculateFirstK(const vector<ProductionBodySymbol>& Pro, vector<ProductionBodySymbol>::size_type left, vector<ProductionBodySymbol>::size_type right, size_t K);
	static void splicing_firstK(vector<string>& first_k, vector<TraceStackNode>& trace_back_stack, vector<ProductionBodySymbol>& p, size_t& m);
	set<string> Nonterminal; //非终结符号集合(增广文法)
	set<string> terminnal;  //终结符号集合
	string StartSymbol; //原文法开始符号
	string AugGraSS;  //增广文法开始符号
	map<long, tuple<string, vector<ProductionBodySymbol>, set<string>>> productionSet; //产生式集合(增广文法)，键为产生式编号，值tuple第一分量为非终结符号，第二分量为产生式代表的终结符号和非终结符号的序列,增广文法新增唯一产生式编号必须为1，第三分量为产生式体中非终结符号的集合
	map<string, set<long>> TerToPro; //键为非终结符，值为对应产生式编号集
	vector<Graph<LALRState, string>::GraphVertexNode>::size_type start; //LALR(1)自动机开始状态
	vector<Graph<LALRState, string>::GraphVertexNode>::size_type accept; //读入向前看符号$后结束语法分析的接受态
	shared_ptr<map<string, set<string>>> first; //指向各非终结符及其first集的映射表
	shared_ptr<map<string, shared_ptr<TrieTree>>> firstK;
	shared_ptr<map<string, set<string>>> follow;  //指向各非终结符及其follow集的映射表
	shared_ptr<map<string, shared_ptr<TrieTree>>> followK;
	pair<shared_ptr<map<string, int>>, shared_ptr<vector<vector<LALRTableItem>>>> LALRTable; //LALR(1)语法分析动作表
};