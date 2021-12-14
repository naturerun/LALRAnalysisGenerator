#pragma once
struct FollowKAndDependency
{
	set<string> direct_dependency;
	map<string, shared_ptr<TrieTree>> undirect_dependency;
	TrieTree* followK_new_add = new TrieTree;  //新增的followK
	TrieTree* FollowK = new TrieTree;  //部分形成的followK
	bool followK_have_got = false;
	~FollowKAndDependency() { delete followK_new_add; }
	FollowKAndDependency(const FollowKAndDependency& be_copied) :FollowK(new TrieTree(*be_copied.FollowK)), followK_new_add(new TrieTree(*be_copied.followK_new_add)), followK_have_got(be_copied.followK_have_got)
	{
		new (&undirect_dependency) map<string, shared_ptr<TrieTree>>();
		for (map<string, shared_ptr<TrieTree>>::const_iterator p = be_copied.undirect_dependency.cbegin(); p != be_copied.undirect_dependency.cend(); ++p)
		{
			undirect_dependency.insert(make_pair(p->first, make_shared<TrieTree>(*(p->second))));
		}
	}
	FollowKAndDependency() = default;
};