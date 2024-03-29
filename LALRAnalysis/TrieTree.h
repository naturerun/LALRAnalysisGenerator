#pragma once
#include <map>
#include <stack>
#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <tuple>
using namespace std;

enum Compare_Result { EQUAL, LEFT_IS_PREFIX, RIGHT_IS_PREFIX, NOT_EQUAL };
struct TrieTreeNode     //Trie树节点类型
{
    enum NodeType { DATANODE, BRANCHNODE } type_flag;  //节点类型标志，分支节点或存放关键字的叶节点
    struct LeafNode
    {
        vector<string> key;
        TrieTreeNode* next = nullptr;
        LeafNode(const vector<string>& k) :key(k) {}
        LeafNode() = default;
        LeafNode(const LeafNode& be_copied) :key(be_copied.key), next(nullptr) {}
    };

    union
    {
        LeafNode key_in_trie;   //叶节点关键字
        map<string, TrieTreeNode*> sub_ptr;  //分支节点的分支字符和对应的指向分支字符对应的子节点的指针之间的映射关系
    };

    TrieTreeNode(const vector<string>& k) :type_flag(NodeType::DATANODE), key_in_trie(k) {}
    TrieTreeNode() :type_flag(NodeType::BRANCHNODE) { new (&sub_ptr) map<string, TrieTreeNode*>(); }

    TrieTreeNode(const TrieTreeNode& be_copied)
    {
        switch (be_copied.type_flag)
        {
        case NodeType::DATANODE: new (&key_in_trie) LeafNode(be_copied.key_in_trie); break;
        case NodeType::BRANCHNODE:
        {
            new (&sub_ptr) map<string, TrieTreeNode*>();
            for (map<string, TrieTreeNode*>::const_iterator p = be_copied.sub_ptr.cbegin(); p != be_copied.sub_ptr.cend(); ++p)
                sub_ptr.insert(make_pair(p->first, nullptr));
        }
        break;
        }
        type_flag = be_copied.type_flag;
    }

    ~TrieTreeNode()
    {
        switch (type_flag)
        {
        case NodeType::DATANODE: key_in_trie.~LeafNode(); break;
        case NodeType::BRANCHNODE: sub_ptr.~map<string, TrieTreeNode*>(); break;
        }
    }
};

class TrieTree
{
public:
    bool insert(const vector<string>& be_inserted);    //Trie树中插入关键字,true成功false失败
    tuple<TrieTreeNode*, TrieTreeNode*, TrieTreeNode*> copy() const;   //拷贝Trie树，返回指向副本Trie树的指针
    void clear();
    size_t getSize() { return size_of_trie_tree; }
    TrieTreeNode* get_first_leaf_list_node_ptr() { return leaf_link_list_first_node; }
    TrieTreeNode* get_last_leaf_list_node_ptr() { return leaf_link_list_last_node; }
    TrieTree() { root = new TrieTreeNode(); }
    TrieTree(const TrieTree& be_copied) { auto temp = be_copied.copy(); root = get<0>(temp);  leaf_link_list_first_node = get<1>(temp); leaf_link_list_last_node = get<2>(temp); size_of_trie_tree = be_copied.size_of_trie_tree; }
    ~TrieTree() { clear(); delete root; }
private:
    Compare_Result static strCompare(const vector<string>& left, const vector<string>& right, vector<string>::size_type& i);
    void insertNewNodeIntoLeafLink(TrieTreeNode* p);
    TrieTreeNode* root = nullptr;   //Trie树根节点
    TrieTreeNode* leaf_link_list_first_node = nullptr;
    TrieTreeNode* leaf_link_list_last_node = nullptr;
    size_t size_of_trie_tree = 0;
};