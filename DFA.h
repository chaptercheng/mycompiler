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
#include <iomanip>//cout��ʽ�����
#include <fstream>
using namespace std;

/*****************����ch�Ƿ��ڱ�DFA����ĸ����***********************/
bool isInAlpha(int ch, map<char, int> &alpha)
{
	map<char, int>::iterator iter = alpha.begin();
	for (; iter != alpha.end(); iter++)
	{//�õ�����������ĸch�Ƿ���DFA����ĸ����
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
	int stateNum;//״̬��
	int alphaNum;//��ĸ����Ŀ
	map<char, int> alpha;//��ĸ����(��Ӧ����)
	vector<int> chart;//״̬ת�Ʊ�
public:
	int start;//��ʼ״̬
	vector<int> finl;//��ֹ״̬
	int transfer(int state, char ch)//״̬ת�ƺ���
	{
		if (!isInAlpha(ch, alpha))
			return -1;
		return chart[alphaNum*state + alpha[ch]];
	}
	int transfer(int state, int t)//״̬ת�ƺ���
	{
		if (t >= alphaNum)//״̬Խ����
		{
			cout << "transfer overflow!" << endl;
			exit(0);
		}
		return chart[alphaNum*state + t];
	}
	DFA(int sNum, int aNum) :stateNum(sNum), alphaNum(aNum)
	{
		chart.resize(sNum * aNum);//״̬ת�Ʊ��ʼ��
	}
	DFA() {}
	void showDFA()//��ʾDFA
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
	/***************���浱ǰDFA******************/
	bool saveDFA(string filename)
	{
		ofstream out;
		out.open(filename);//�򿪱����ļ�
		bool flag = false;
		if (out.is_open())
		{
			out << stateNum << " " << alphaNum << endl;//������Ϊ״̬������ĸ����
			for (int i = 0; i < stateNum; i++)
			{
				for (int j = 0; j < alphaNum; j++)
				{
					out << chart[alphaNum*i + j];//����״̬ת�Ʊ�
					if (j < alphaNum - 1) out << " ";
				}
				out << endl;
			}
			out << start << endl;
			int n = finl.size();
			out << n << " ";//�ֱ������״̬��λ��
			for (int i = 0; i<int(finl.size()); i++)
			{
				out << finl[i];
				if (i<int(finl.size() - 1)) out << " ";
				else out << endl;
			}
			map<char, int>::iterator iter = alpha.begin();
			iter = alpha.begin();
			for (; iter != alpha.end(); iter++)//��󱣴�DFA����ĸ��
				out << iter->first << " " << iter->second << endl;
			out << "###";//�ļ��������
			flag = true;
			out.close();
		}
		return flag;
	}
	/************���ļ��ж�ȡ�Ѵ��ڵ�DFA*************/
	bool readDFA(string filename)
	{
		ifstream in;
		bool flag = false;
		in.open(filename);//���ļ�
		if (in.is_open())
		{
			flag = true;
			in >> stateNum >> alphaNum;//��ȡ��ĸ���С
			chart.resize(stateNum*alphaNum);
			for (int i = 0; i < stateNum; i++)//��ȡ��ĸ��
			{
				for (int j = 0; j < alphaNum; j++)
				{
					in >> chart[alphaNum*i + j];
				}
			}
			in >> start;
			int n;
			in >> n;//����״̬����
			finl.resize(n);
			for (int i = 0; i < n; i++)/**************final size*/
				in >> finl[i];
			char ch; int num;
			string str;
			getline(in, str);
			//getline(in,str);
			while (1)
			{//��ȡDFA��ĸ������###����
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


