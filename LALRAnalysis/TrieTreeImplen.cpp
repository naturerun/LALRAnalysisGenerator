#include "pch.h"
#include "TrieTree.h"

void TrieTree::clear()
{
	TrieTreeNode *run = root;
	stack<pair<TrieTreeNode *, map<string, TrieTreeNode *>::iterator>> work_stack;

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
					work_stack.top().second = run->sub_ptr.erase(work_stack.top().second);
					trace_back_flag = false;
					continue;
				}
			}

			work_stack.push(make_pair(run, run->sub_ptr.begin()));
			run = run->sub_ptr.begin()->second;
		}
		else
		{
			if (run == root)
			{
				if (work_stack.top().second == root->sub_ptr.end())
				{
					break;
				}

				run = work_stack.top().second->second;
				trace_back_flag = true;
			}
			else
			{
				if (work_stack.top().second != run->sub_ptr.end())
				{
					run = work_stack.top().second->second;
					trace_back_flag = true;
				}
				else
				{
					delete run;
					work_stack.pop();
					run = work_stack.top().first;
					work_stack.top().second = run->sub_ptr.erase(work_stack.top().second);
				}
			}
		}
	}
	leaf_link_list_first_node = nullptr;
	leaf_link_list_last_node = nullptr;
	size_of_trie_tree = 0;
}

bool TrieTree::strCompare(const vector<string> &left, const vector<string> &right, const size_t &i)
{
	for (size_t j = i; ; ++j)
	{
		if (j >= left.size() && j >= right.size())
			return true;
		else if (j >= left.size() || j >= right.size())
			return false;
		else if (left[j] != right[j])
			return false;
	}
}

void TrieTree::adjustLeafLinkAfterDel(TrieTreeNode *run)
{
	if (leaf_link_list_first_node != run)
	{
		TrieTreeNode *ptr = leaf_link_list_first_node;
		for (; ptr->key_in_trie.next != run; ptr = ptr->key_in_trie.next) { ; }
		ptr->key_in_trie.next = ptr->key_in_trie.next->key_in_trie.next;
	}
	else
	{
		leaf_link_list_first_node = leaf_link_list_first_node->key_in_trie.next;
	}

	if (run == leaf_link_list_last_node) { leaf_link_list_last_node = leaf_link_list_last_node->key_in_trie.next; }
}

bool TrieTree::deleteElem(const vector<string> &be_deleted)
{
	TrieTreeNode *run = root;
	if (be_deleted.empty() == true)
	{
		map<string, TrieTreeNode *>::iterator tempit = run->sub_ptr.find("");
		if (tempit != run->sub_ptr.end())
		{
			adjustLeafLinkAfterDel(tempit->second);
			delete tempit->second;
			run->sub_ptr.erase(tempit);
			--size_of_trie_tree;
			return true;
		}
		else
		{
			return false;
		}
	}

	stack<pair<TrieTreeNode *, map<string, TrieTreeNode *>::iterator>> work_stack;
	string::size_type i = 0;
	while (run->type_flag != TrieTreeNode::NodeType::DATANODE)
	{
		if (i < be_deleted.size())
		{
			string temp = be_deleted[i];
			++i;
			map<string, TrieTreeNode *>::iterator it = run->sub_ptr.find(temp);
			if (it == run->sub_ptr.end())
			{
				return false;
			}
			else
			{
				work_stack.push(make_pair(run, it));
				run = it->second;
			}
		}
		else
		{
			map<string, TrieTreeNode *>::iterator it = run->sub_ptr.find("");
			if (it != run->sub_ptr.end())
			{
				work_stack.push(make_pair(run, it));
				run = it->second;
				break;
			}
			else
			{
				return false;
			}
		}
	}

	if (work_stack.top().second->first != "" && strCompare(be_deleted, run->key_in_trie.key, i) == false)
	{
		return false;
	}

	bool delete_or_not = true;
	while (work_stack.top().first != root)
	{
		if (delete_or_not == true)
		{
			if (work_stack.top().second->second->type_flag == TrieTreeNode::NodeType::DATANODE)  ///
			{
				adjustLeafLinkAfterDel(work_stack.top().second->second);
				--size_of_trie_tree;
				run = nullptr;
			}
			delete work_stack.top().second->second;
			work_stack.top().first->sub_ptr.erase(work_stack.top().second);

			if (work_stack.top().first->sub_ptr.size() >= 2)
			{
				return true;
			}
			else if (work_stack.top().first->sub_ptr.size() == 1)
			{
				if (work_stack.top().first->sub_ptr.begin()->second->type_flag != TrieTreeNode::NodeType::DATANODE)
				{
					return true;
				}
				else
				{
					run = work_stack.top().first->sub_ptr.begin()->second;
					delete work_stack.top().first;
					delete_or_not = false;
				}
			}
			work_stack.pop();
		}
		else
		{
			if (work_stack.top().first->sub_ptr.size() >= 2)
			{
				work_stack.top().second->second = run;
				return true;
			}
			else
			{
				delete work_stack.top().first;
				work_stack.pop();
			}
		}
	}

	if (delete_or_not == true)
	{
		delete work_stack.top().second->second;
		root->sub_ptr.erase(work_stack.top().second);
	}
	else
	{
		work_stack.top().second->second = run;
	}
	return true;
}

void TrieTree::insertNewNodeIntoLeafLink(TrieTreeNode *p)
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

bool TrieTree::insert(const vector<string> &be_inserted)
{
	TrieTreeNode *run = root;
	if (be_inserted.empty() == true)
	{
		auto temp = run->sub_ptr.insert(make_pair("", new TrieTreeNode(be_inserted)));
		if (temp.second)
		{
			insertNewNodeIntoLeafLink(temp.first->second);
			++size_of_trie_tree;
			return true;
		}
		else
			return false;
	}

	map<string, TrieTreeNode *>::iterator father;
	string::size_type i = 0;
	while (run->type_flag != TrieTreeNode::NodeType::DATANODE)
	{
		if (i < be_inserted.size())
		{
			string temp = be_inserted[i];
			++i;
			map<string, TrieTreeNode *>::iterator it = run->sub_ptr.find(temp);
			if (it == run->sub_ptr.end())
			{
				auto l = run->sub_ptr.insert(make_pair(temp, new TrieTreeNode(be_inserted))).first;
				insertNewNodeIntoLeafLink(l->second);
				++size_of_trie_tree;
				return true;
			}
			else
			{
				father = it;
				run = it->second;
			}
		}
		else
		{
			if (run->sub_ptr.find("") != run->sub_ptr.end())
			{
				return false;
			}
			else
			{
				auto l = run->sub_ptr.insert(make_pair("", new TrieTreeNode(be_inserted))).first;
				insertNewNodeIntoLeafLink(l->second);
				++size_of_trie_tree;
				return true;
			}
		}
	}

	if (strCompare(be_inserted, run->key_in_trie.key, i) == true)
	{
		return false;
	}
	else
	{
		while (true)
		{
			father->second = new TrieTreeNode();
			map<string, TrieTreeNode *>::iterator it;
			if (i >= be_inserted.size())
			{
				it = father->second->sub_ptr.insert(make_pair("", new TrieTreeNode(be_inserted))).first;
				father->second->sub_ptr.insert(make_pair(run->key_in_trie.key[i], run));
			}
			else if (i >= run->key_in_trie.key.size())
			{
				father->second->sub_ptr.insert(make_pair("", run));
				it = father->second->sub_ptr.insert(make_pair(be_inserted[i], new TrieTreeNode(be_inserted))).first;
			}
			else if (be_inserted[i] != run->key_in_trie.key[i])
			{
				father->second->sub_ptr.insert(make_pair(run->key_in_trie.key[i], run));
				it = father->second->sub_ptr.insert(make_pair(be_inserted[i], new TrieTreeNode(be_inserted))).first;
			}
			else
			{
				father = father->second->sub_ptr.insert(make_pair(be_inserted[i], new TrieTreeNode())).first;
				++i;
				continue;
			}

			insertNewNodeIntoLeafLink(it->second);
			++size_of_trie_tree;
			return true;
		}
	}
}

tuple<TrieTreeNode *, TrieTreeNode *, TrieTreeNode *> TrieTree::copy() const
{
	TrieTreeNode *be_copied = root;
	stack<pair<TrieTreeNode *, map<string, TrieTreeNode *>::iterator>> work_stack;
	stack<pair<TrieTreeNode *, map<string, TrieTreeNode *>::iterator>> copy_trace_stack;
	TrieTreeNode *root_of_copy = nullptr;
	TrieTreeNode *leaf_first_ptr_of_copy = nullptr;
	TrieTreeNode *leaf_last_ptr_of_copy = nullptr;
	map<TrieTreeNode *, TrieTreeNode *> be_copied_leaf_addr_to_copy_leaf_addr;

	bool trace_back_flag = true;
	while (true)
	{
		if (trace_back_flag == true)
		{
			if (be_copied == root)
			{
				root_of_copy = new TrieTreeNode(*be_copied);
				if (be_copied->sub_ptr.begin() == be_copied->sub_ptr.end())
				{
					break;
				}
				copy_trace_stack.push(make_pair(root_of_copy, root_of_copy->sub_ptr.begin()));
			}
			else
			{
				if (work_stack.top().second == work_stack.top().first->sub_ptr.begin())
				{
					copy_trace_stack.top().second->second = new TrieTreeNode(*be_copied);
				}
				else
				{
					++copy_trace_stack.top().second;
					copy_trace_stack.top().second->second = new TrieTreeNode(*be_copied);
				}

				if (copy_trace_stack.top().second->second->type_flag == TrieTreeNode::NodeType::DATANODE)
				{
					be_copied_leaf_addr_to_copy_leaf_addr.insert(make_pair(be_copied, copy_trace_stack.top().second->second));
				}

				if (be_copied->type_flag != TrieTreeNode::DATANODE)
					copy_trace_stack.push(make_pair(copy_trace_stack.top().second->second, copy_trace_stack.top().second->second->sub_ptr.begin()));
				else
				{
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
			map<string, TrieTreeNode *>::iterator tempit = work_stack.top().second;
			if (tempit->second->type_flag != TrieTreeNode::DATANODE)
			{
				copy_trace_stack.pop();
			}

			if (be_copied == root)
			{
				if (++(work_stack.top().second) == root->sub_ptr.end())
					break;

				be_copied = work_stack.top().second->second;
				trace_back_flag = true;
			}
			else
			{
				if (++(work_stack.top().second) != be_copied->sub_ptr.end())
				{
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
	}

	if (leaf_link_list_first_node != nullptr)
	{
		TrieTreeNode * p = leaf_first_ptr_of_copy = be_copied_leaf_addr_to_copy_leaf_addr[leaf_link_list_first_node];
		for (TrieTreeNode *leaf_run_of_becopied = leaf_link_list_first_node->key_in_trie.next; leaf_run_of_becopied != nullptr; leaf_run_of_becopied = leaf_run_of_becopied->key_in_trie.next)
		{
			p->key_in_trie.next = be_copied_leaf_addr_to_copy_leaf_addr[leaf_run_of_becopied];
			p = p->key_in_trie.next;
		}
		leaf_last_ptr_of_copy = p;
	}

	return { root_of_copy, leaf_first_ptr_of_copy, leaf_last_ptr_of_copy };
}

