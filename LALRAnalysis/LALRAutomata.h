#pragma once
#include "DirectedGraph.h"
#include "LALRState.h"
#include "TrieTree.h"
#include "LALRTableItem.h"
class LALRAutomata : public  Graph<LALRState, string>  //LALR(1)�Զ����࣬�Զ���������ͼ��ʾ��������ͼ����Graph��
{
	friend class RELALRParsing;
	friend void output(LALRAutomata& test, ofstream& output);
public:
	struct ProductionBodySymbol   //����ʽ���е��ķ�����
	{
		string symbol;   //�ս������ս��
		bool TerminalOrNot;  //true�ս��,false���ս��
		ProductionBodySymbol() = default;
		ProductionBodySymbol(string s, bool T) :symbol(s), TerminalOrNot(T) {}
	};

	struct SymbolInfo
	{
		ProductionBodySymbol symbol_info;
		vector<bool> visit_flag;//ÿһ����ս��firstK�ڵ���ʱ��
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
	static bool isSubSet(const map<string, set<string>>& left, const set<string>& right);  //�ж�right�Ƿ�Ϊleft�������Ӽ�,����true�Ƿ���false����
private:
	shared_ptr<map<string, shared_ptr<TrieTree>>> calculateFirstK(size_t K);
	shared_ptr<map<string, set<string>>> calculateFirst(); //��������ս������first����ӳ���
	shared_ptr<set<string>> calculateFirst(const vector<ProductionBodySymbol>& Pro, vector<ProductionBodySymbol>::size_type left, vector<ProductionBodySymbol>::size_type right); //���ظ����ķ����Ŵ���first��
	shared_ptr<map<string, shared_ptr<TrieTree>>> calculateFollowK(size_t K);
	shared_ptr<map<string, set<string>>> calculateFollow(); //��������ս������follow����ӳ���
	void calculateClosureLALR(map<long, map<int, set<string>>>& kernelset, map<long, LALRState::attribute>& nonkernelset); //��һ�������ں�������ں�����ɷ��ں���ɵıհ�������ڶ�������,��������Ϊ����LALR�ں���հ������㿪ʼǰnonkernelset�ᱻ���
	void calculateClosureLR(map<long, map<int, set<string>>>& kernelset, map<long, LALRState::attribute>& nonkernelset);  //����LR(0)�ں��(�����kernelset)�հ�(ֻ�������ں���)�������������nonkernelset�����㿪ʼǰnonkernelset�ᱻ���
	void constructLRKernel(); //����LR(0)��壬ÿ���ֻ����LR(0)�ں���
	pair<shared_ptr<map<string, int>>, shared_ptr<vector<vector<LALRTableItem>>>> constructLALR(); //����LALR(1)���,���ص�pair��һ����Ϊ�ķ����ź�LALR�﷨�������б�ӳ���ϵ������ָ�룬�ڶ�����Ϊָ��LALR�﷨�����������ָ��
	void initgrammar(ifstream& input); //��ʼ���ķ������Ҫ��
	static void computeDifferenceSet(set<string>& left_operand, set<string>& right_operand, set<string>& result, bool clear_or_not);
	void adjustAfterInsert(vector<SymbolInfo>& q, size_t& visit_pos_every_layer, vector<string>& first_k,
		shared_ptr<map<string, shared_ptr<TrieTree>>>& result, map<string, pair<TrieTreeNode*, size_t>>& ptr_point_to_last_visit, vector<size_t>& p, size_t& new_add_count, map<string, map<long, tuple<vector<SymbolInfo>, set<string>, vector<size_t>>>>& temp, const string& head, size_t& m, long& current_advance_point_in_pro_head_symbol_indexs);
	void insertFirstK(vector<TraceBackInfo>& trace_back_stack, vector<SymbolInfo>& q, vector<string>& first_k, size_t& m);
	shared_ptr<TrieTree> calculateFirstK(const vector<ProductionBodySymbol>& Pro, vector<ProductionBodySymbol>::size_type left, vector<ProductionBodySymbol>::size_type right, size_t K);
	static void splicing_firstK(vector<string>& first_k, vector<TraceStackNode>& trace_back_stack, vector<ProductionBodySymbol>& p, size_t& m);
	set<string> Nonterminal; //���ս���ż���(�����ķ�)
	set<string> terminnal;  //�ս���ż���
	string StartSymbol; //ԭ�ķ���ʼ����
	string AugGraSS;  //�����ķ���ʼ����
	map<long, tuple<string, vector<ProductionBodySymbol>, set<string>>> productionSet; //����ʽ����(�����ķ�)����Ϊ����ʽ��ţ�ֵtuple��һ����Ϊ���ս���ţ��ڶ�����Ϊ����ʽ������ս���źͷ��ս���ŵ�����,�����ķ�����Ψһ����ʽ��ű���Ϊ1����������Ϊ����ʽ���з��ս���ŵļ���
	map<string, set<long>> TerToPro; //��Ϊ���ս����ֵΪ��Ӧ����ʽ��ż�
	vector<Graph<LALRState, string>::GraphVertexNode>::size_type start; //LALR(1)�Զ�����ʼ״̬
	vector<Graph<LALRState, string>::GraphVertexNode>::size_type accept; //������ǰ������$������﷨�����Ľ���̬
	shared_ptr<map<string, set<string>>> first; //ָ������ս������first����ӳ���
	shared_ptr<map<string, shared_ptr<TrieTree>>> firstK;
	shared_ptr<map<string, set<string>>> follow;  //ָ������ս������follow����ӳ���
	shared_ptr<map<string, shared_ptr<TrieTree>>> followK;
	pair<shared_ptr<map<string, int>>, shared_ptr<vector<vector<LALRTableItem>>>> LALRTable; //LALR(1)�﷨����������
};