#pragma once
#include <iostream>
#include <list>
#include <functional>
using namespace std;

template <typename T>
class Priority_Queue    //�豸�ȴ�������(���ȼ�����),��������Ԫ��Ϊt
{
public:
	typedef typename list<T>::iterator iterator;
	Priority_Queue() = default;
	Priority_Queue(const function<bool(const T&, const T&)> &com) :comparator(com) {}
	~Priority_Queue() = default;
	pair<bool, typename Priority_Queue<T>::iterator> Insert(const T &x);  //�������,���ص�pair��firstָʾ�����Ƿ�ɹ�,secondΪָ�����Ԫ�صĵ�����
	bool RemoveTop(T &x);     //ɾ��������ȼ�Ԫ�ز���x���䷵��
	bool GetTop(T &x) const;  //��ȡ������ȼ�Ԫ�ز���x���䷵�� 
	void MakeEmpty() { Queue.clear(); }    //��ն���
	bool isEmpty() const { return Queue.empty(); }   //�ж϶����Ƿ�Ϊ��
	bool isFull() const { return Queue.size() == Queue.max_size(); }   //�ж϶����Ƿ�����
	typename Priority_Queue<T>::iterator erase(const typename Priority_Queue<T>::iterator &p) { return Queue.erase(p); }  //ɾ��������p��ָԪ�ط��ر�ɾԪ�ص���һԪ��
	typename Priority_Queue<T>::iterator insert(const typename Priority_Queue<T>::iterator &p, const T &c) { return Queue.insert(p, c); }  //��c������p��ָλ��,����ָ�����Ԫ�صĵ�����
	typename list<T>::size_type  GetSize() const { return Queue.size(); }   //��ȡ����ʵ�ʴ�С
	iterator begin() { return Queue.begin(); }   //��ȡָ�����������ȼ�Ԫ�صĵ�����
	iterator end() { return Queue.end(); }   //��ȡ����β�������

private:
	function<bool(const T&, const T&)> comparator;  //�Ƚ�T���͵Ŀɵ��ö����������С���Ҳ���������true
	typename list<T>::iterator adjust();   //��Ԫ�ؼ�����к����Ԫ��λ��,ʹ�����и�Ԫ�ر������ȼ���ϵ
	list<T> Queue;
};

template <typename T>
typename list<T>::iterator Priority_Queue<T>::adjust()
{
	T temp = Queue.back();
	auto p = Queue.end();
	--p;
	p = Queue.erase(p);

	if (Queue.begin() != p)
		--p;
	else
	{
		return Queue.insert(p, temp);
	}

	while (true)
	{
		if (comparator(temp, (*p)))
		{
			if (p != Queue.begin())
			{
				--p;
				if (p == Queue.begin())
					continue;
			}
		}
		else
		{
			++p;
			return Queue.insert(p, temp);
		}

		if (p == Queue.begin())
			break;
	}
	return Queue.insert(p, temp);
}

template <typename T>
pair<bool, typename Priority_Queue<T>::iterator> Priority_Queue<T>::Insert(const T &x)
{
	if (isFull())
		return { false, end() };
	else
	{
		Queue.push_back(x);
		return { true, adjust() };
	}
}

template <typename T>
bool Priority_Queue<T>::RemoveTop(T &x)
{
	if (isEmpty())
		return false;
	else
	{
		x = Queue.front();
		Queue.pop_front();
		return true;
	}
}

template <typename T>
bool Priority_Queue<T>::GetTop(T &x) const
{
	if (isEmpty())
		return false;
	else
	{
		x = Queue.front();
		return true;
	}
}