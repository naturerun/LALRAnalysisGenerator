#pragma once
#include<set>
#include<map>
#include<string>
void setToMap(const set<string> &source, map<string, int> &goal, int &count)
{
	for (set<string>::iterator p = source.cbegin(); p != source.cend(); ++p)
	{
		goal.insert(make_pair(*p, count++));
	}
}