#include "pch.h"
#include "TrieTree.h"

Compare_Result TrieTree::strCompare(const vector<string>& left, const vector<string>& right, vector<string>::size_type& i)
{
    for (; ; ++i)
    {
        if (i == left.size() && i == right.size())
            return Compare_Result::EQUAL;
        else if (i == left.size() || i == right.size())
        {
            if (i == left.size())
                return Compare_Result::LEFT_IS_PREFIX;
            else
                return Compare_Result::RIGHT_IS_PREFIX;
        }
        else if (left[i] != right[i])
            return Compare_Result::NOT_EQUAL;
    }
}

void TrieTree::insertNewNodeIntoLeafLink(TrieTreeNode* p)
{
    if (leaf_link_list_last_node == nullptr)
    {
        leaf_link_list_first_node = leaf_link_list_last_node = p;
    }
    else
    {
        leaf_link_list_last_node->key_in_trie.next = p;
        leaf_link_list_last_node = p;
    }
}

bool TrieTree::insert(const vector<string>& be_inserted)
{
    TrieTreeNode* run = root;
    vector<string>::size_type i = 0;
    pair<map<string, TrieTreeNode*>::iterator, bool> result;
    while (run->type_flag != TrieTreeNode::NodeType::DATANODE)
    {
        if (i < be_inserted.size())
        {
            result = run->sub_ptr.insert(make_pair(be_inserted[i], new TrieTreeNode(be_inserted)));
            if (result.second)
            {
                break;
            }
            run = result.first->second;
            ++i;
        }
        else
        {
            if (run->sub_ptr.empty() || run->sub_ptr.begin()->first != "")
            {
                result = run->sub_ptr.insert(make_pair("", new TrieTreeNode(be_inserted)));
                break;

            }
            return false;
        }
    }

    if (run->type_flag != TrieTreeNode::NodeType::DATANODE)
    {
        insertNewNodeIntoLeafLink(result.first->second);
        ++size_of_trie_tree;
        return true;
    }

    Compare_Result compare_result;
    {
        string::size_type start_index = i;
        compare_result = strCompare(be_inserted, run->key_in_trie.key, i);
        if (compare_result == Compare_Result::EQUAL)
            return false;

        result.first->second = new TrieTreeNode();
        for (; start_index < i; ++start_index)
            result.first = result.first->second->sub_ptr.insert(make_pair(be_inserted[start_index], new TrieTreeNode())).first;
    }

    map<string, TrieTreeNode*>::iterator it;
    if (compare_result == Compare_Result::LEFT_IS_PREFIX)
    {
        it = result.first->second->sub_ptr.insert(make_pair("", new TrieTreeNode(be_inserted))).first;
        result.first->second->sub_ptr.insert(make_pair(run->key_in_trie.key[i], run));
    }
    else if (compare_result == Compare_Result::RIGHT_IS_PREFIX)
    {
        result.first->second->sub_ptr.insert(make_pair("", run));
        it = result.first->second->sub_ptr.insert(make_pair(be_inserted[i], new TrieTreeNode(be_inserted))).first;
    }
    else
    {
        result.first->second->sub_ptr.insert(make_pair(run->key_in_trie.key[i], run));
        it = result.first->second->sub_ptr.insert(make_pair(be_inserted[i], new TrieTreeNode(be_inserted))).first;
    }

    insertNewNodeIntoLeafLink(it->second);
    ++size_of_trie_tree;
    return true;
}

void TrieTree::clear()
{
    TrieTreeNode* run = root;
    stack<pair<TrieTreeNode*, map<string, TrieTreeNode*>::iterator>> work_stack;

    bool trace_back_flag = true;
    while (true)
    {
        if (trace_back_flag == true)
        {
            if (run == root)
            {
                if (run->sub_ptr.begin() == run->sub_ptr.end())
                {
                    break;
                }
            }
            else
            {
                if (run->type_flag == TrieTreeNode::DATANODE)
                {
                    delete run;
                    run = work_stack.top().first;
                    ++work_stack.top().second;
                    //work_stack.top().second = run->sub_ptr.erase(work_stack.top().second);
                    trace_back_flag = false;
                    continue;
                }
            }

            work_stack.push(make_pair(run, run->sub_ptr.begin()));
            run = run->sub_ptr.begin()->second;
        }
        else
        {
            if (run == root || work_stack.top().second != run->sub_ptr.end())
            {
                if (run == root)
                {
                    if (work_stack.top().second == root->sub_ptr.end())
                    {
                        root->sub_ptr.clear();
                        break;
                    }
                }
                run = work_stack.top().second->second;
                trace_back_flag = true;
            }
            else
            {
                delete run;
                work_stack.pop();
                run = work_stack.top().first;
                ++work_stack.top().second;
                // work_stack.top().second = run->sub_ptr.erase(work_stack.top().second);
            }
        }
    }
    leaf_link_list_first_node = nullptr;
    leaf_link_list_last_node = nullptr;
    size_of_trie_tree = 0;
}

tuple<TrieTreeNode*, TrieTreeNode*, TrieTreeNode*> TrieTree::copy() const
{
    TrieTreeNode* be_copied = root;
    stack<pair<TrieTreeNode*, map<string, TrieTreeNode*>::iterator>> work_stack;
    stack<pair<TrieTreeNode*, map<string, TrieTreeNode*>::iterator>> copy_trace_stack;
    TrieTreeNode* root_of_copy = nullptr;
    TrieTreeNode* leaf_first_ptr_of_copy = nullptr;
    TrieTreeNode* leaf_last_ptr_of_copy = nullptr;

    bool trace_back_flag = true;
    while (true)
    {
        if (trace_back_flag == true)
        {
            if (be_copied == root)
            {
                root_of_copy = new TrieTreeNode(*be_copied);
                if (be_copied->sub_ptr.begin() == be_copied->sub_ptr.end())
                    break;
                copy_trace_stack.push(make_pair(root_of_copy, root_of_copy->sub_ptr.begin()));
            }
            else
            {
                if (work_stack.top().second != work_stack.top().first->sub_ptr.begin())
                    ++copy_trace_stack.top().second;
                copy_trace_stack.top().second->second = new TrieTreeNode(*be_copied);

                if (be_copied->type_flag != TrieTreeNode::DATANODE)
                    copy_trace_stack.push(make_pair(copy_trace_stack.top().second->second, copy_trace_stack.top().second->second->sub_ptr.begin()));
                else
                {
                    if (leaf_last_ptr_of_copy == nullptr)
                    {
                        leaf_first_ptr_of_copy = leaf_last_ptr_of_copy = copy_trace_stack.top().second->second;
                    }
                    else
                    {
                        leaf_last_ptr_of_copy->key_in_trie.next = copy_trace_stack.top().second->second;
                        leaf_last_ptr_of_copy = copy_trace_stack.top().second->second;
                    }
                    be_copied = work_stack.top().first;
                    trace_back_flag = false;
                    continue;
                }
            }

            work_stack.push(make_pair(be_copied, be_copied->sub_ptr.begin()));
            be_copied = be_copied->sub_ptr.begin()->second;
        }
        else
        {
            if (work_stack.top().second->second->type_flag != TrieTreeNode::DATANODE)
                copy_trace_stack.pop();

            if (be_copied == root || ++(work_stack.top().second) != be_copied->sub_ptr.end())
            {
                if (be_copied == root)
                {
                    if (++(work_stack.top().second) == root->sub_ptr.end())
                        break;
                }
                be_copied = work_stack.top().second->second;
                trace_back_flag = true;
            }
            else
            {
                work_stack.pop();
                be_copied = work_stack.top().first;
            }
        }
    }
    return { root_of_copy,  leaf_first_ptr_of_copy, leaf_last_ptr_of_copy };
}