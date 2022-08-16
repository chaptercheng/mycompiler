#ifndef NFATODFA_H
#define NFATODFA_H
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <queue>
#include <stdio.h>
#include <iomanip>//cout格式化输出
#include <fstream>
using namespace std;

/*****************查找ch是否在本DFA的字母表中***********************/
bool isInAlpha(int ch, map<char, int> &alpha)
{
	map<char, int>::iterator iter = alpha.begin();
	for (; iter != alpha.end(); iter++)
	{//用迭代器查找字母ch是否在DFA的字母表中
		if (iter->first == ch)
			return true;
		else if (ch >= '0' && ch <= '9')
			return true;
	}
	return false;
}

class DFA
{
private:
	int stateNum;//状态数
	int alphaNum;//字母表数目
	map<char, int> alpha;//字母表集合(对应的列)
	vector<int> chart;//状态转移表
public:
	int start;//开始状态
	vector<int> finl;//终止状态
	int transfer(int state, char ch)//状态转移函数
	{
		if (!isInAlpha(ch, alpha))
			return -1;
		return chart[alphaNum*state + alpha[ch]];
	}
	int transfer(int state, int t)//状态转移函数
	{
		if (t >= alphaNum)//状态越界检查
		{
			cout << "transfer overflow!" << endl;
			exit(0);
		}
		return chart[alphaNum*state + t];
	}
	DFA(int sNum, int aNum) :stateNum(sNum), alphaNum(aNum)
	{
		chart.resize(sNum * aNum);//状态转移表初始化
	}
	DFA() {}
	void showDFA()//显示DFA
	{
		cout << "------------transfer matrix--------------" << endl;
		map<int, char> falpha;
		map<char, int>::iterator iter = alpha.begin();
		for (; iter != alpha.end(); iter++)
		{
			falpha[iter->second] = iter->first;
		}
		map<int, char>::iterator fiter = falpha.begin();
		cout << "  ";
		for (; fiter != falpha.end(); fiter++)
		{
			cout << fiter->second << " ";
		}
		cout << endl;
		for (int i = 0; i < stateNum; i++)
		{
			cout << i << ":";
			for (int j = 0; j < alphaNum; j++)
			{
				cout << chart[alphaNum*i + j] << " ";
			}
			cout << endl;
		}
		cout << "start state>" << start << endl;
		cout << "final set>>>";
		for (int i = 0; i<int(finl.size()); i++)
			cout << finl[i] << " ";
		cout << endl;
		cout << "keys>>\n";
		iter = alpha.begin();
		for (; iter != alpha.end(); iter++)
			cout << iter->first << " " << iter->second << endl;
	}
	/***************保存当前DFA******************/
	bool saveDFA(string filename)
	{
		ofstream out;
		out.open(filename);//打开保存文件
		bool flag = false;
		if (out.is_open())
		{
			out << stateNum << " " << alphaNum << endl;//首两行为状态数和字母表数
			for (int i = 0; i < stateNum; i++)
			{
				for (int j = 0; j < alphaNum; j++)
				{
					out << chart[alphaNum*i + j];//保存状态转移表
					if (j < alphaNum - 1) out << " ";
				}
				out << endl;
			}
			out << start << endl;
			int n = finl.size();
			out << n << " ";//现保存结束状态的位数
			for (int i = 0; i<int(finl.size()); i++)
			{
				out << finl[i];
				if (i<int(finl.size() - 1)) out << " ";
				else out << endl;
			}
			map<char, int>::iterator iter = alpha.begin();
			iter = alpha.begin();
			for (; iter != alpha.end(); iter++)//最后保存DFA的字母表
				out << iter->first << " " << iter->second << endl;
			out << "###";//文件结束标记
			flag = true;
			out.close();
		}
		return flag;
	}
	/************从文件中读取已存在的DFA*************/
	bool readDFA(string filename)
	{
		ifstream in;
		bool flag = false;
		in.open(filename);//打开文件
		if (in.is_open())
		{
			flag = true;
			in >> stateNum >> alphaNum;//读取字母表大小
			chart.resize(stateNum*alphaNum);
			for (int i = 0; i < stateNum; i++)//读取字母表
			{
				for (int j = 0; j < alphaNum; j++)
				{
					in >> chart[alphaNum*i + j];
				}
			}
			in >> start;
			int n;
			in >> n;//结束状态个数
			finl.resize(n);
			for (int i = 0; i < n; i++)/**************final size*/
				in >> finl[i];
			char ch; int num;
			string str;
			getline(in, str);
			//getline(in,str);
			while (1)
			{//读取DFA字母表遇见###结束
				getline(in, str);
				if (str == "###") break;
				ch = str[0];
				num = str[2] - '0';
				if (int(str.size()) >= 4)
					num = num * 10 + str[3] - '0';
				alpha[ch] = num;
			}
			in.close();
		}
		return flag;
	}
};
/**
9 18 6 1 1
d . e ` + -
0 + 1
0 - 1
0 d 2
1 d 2
2 d 2
2 ` 8
2 . 3
2 e 5
3 d 4
4 d 4
4 ` 8
4 e 5
5 d 7
5 + 6
5 - 6
6 d 7
7 d 7
7 ` 8
0
8
4567;
2346;
2.4e+1;
3.1415;
***

*/
#endif // NFATODFA_H


