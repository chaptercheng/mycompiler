#ifndef GRAMMER_H
#define GRAMMER_H
#include<iostream>
#include<stdlib.h>
#include <vector>
#include <iterator>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <fstream>
#include "Scanner.h"

using namespace std;
struct PData//文法产生式数据节点
{
	string genL;//文法左部
	vector<vector<string>> genR;//文法右部
	PData(string GENL) :genL(GENL) {}
};
struct ListData//产生式列表数据节点
{
	string genL;
	vector<string> genR;
	ListData(string GL, vector<string> GR) :genL(GL), genR(GR) {}
};
class Grammer
{
public:
	string start;//开始符号,规定开始符号为第一个产生式左部
	int nonNum, terminNum;//终结符与非终结符数目
	vector<string> termin;//终结符集合
	vector<string> nonTermin;//非终结符
	map<string, int> firstPos, //记录非终结符第一次在genList中出现的位置
		symPos;//和符号在SLR表中的位置
	vector<PData> P;//产生式集合
	vector<ListData> genList;//产生式列表
	/****从文件中读取语法信息****/
	bool readGrammer(string filename = "MyGram.txt")
	{
		bool flag = false;
		ifstream in;
		in.open(filename);
		if (in.is_open())//打开文法文件
		{
			string tstr;
			int symP = 0, firstP = 0;
			///正式输入文法
			string tempI;
			while (in >> tempI && tempI != "##")
			{//一次循环处理一组产生式
				firstPos[tempI] = firstP;//非终结符
				PData pd(tempI);
				symPos[tempI] = symP++;
				nonTermin.push_back(tempI);
				vector<string> strs;
				string tempJ;
				while (in >> tempJ && tempJ != "#")
				{//处理一个产生式
					firstP++;
					int left = 0, right = tempJ.find("`", 0);
					while (right)//right为0时，即结束
					{
						strs.push_back(tempJ.substr(left, right - left));
						left = ++right;
						if (right != 0)
							right = tempJ.find("`", left);
					}
					pd.genR.push_back(strs);
					genList.push_back(ListData(pd.genL, strs));
					strs.clear();
				}
				P.push_back(pd);
			}
			while (in >> tstr && tstr != "#")//输入终结符
			{
				termin.push_back(tstr);
				symPos[tstr] = symP++;
			}
			nonNum = nonTermin.size();
			terminNum = termin.size();
			symPos["#"] = nonNum + terminNum;
			start = genList[0].genL;
			in.close();
			flag = true;
		}//if open
		return flag;
	}//getGrammer

	/**********显示产生式文法文法*************/
	void showGenList()
	{
		printf("(genList:%d,nonNum:%d,terminNum:%d)\n", genList.size(), nonNum, terminNum);
		for (int i = 0, k = 0, t = 0; i<int(genList.size()); i++)//输出文法
		{
			printf("|%3d :%s-> ", i, genList[i].genL.c_str());
			for (int j = 0; j < (int)genList[i].genR.size(); j++, k++)
				printf("%s ", genList[i].genR[j].c_str());
			printf("\n");
		}printf("|\n");
	}
	void showPList()
	{
		for (int i = 0; i <(int) P.size(); i++)
		{
			printf("%s-> ", P.at(i).genL.c_str());
			for (int j = 0; j < (int)P.at(i).genR.size(); j++)
			{
				for (int k = 0; k < (int)P.at(i).genR.at(j).size(); k++)
				{
					printf("%s", P.at(i).genR.at(j).at(k).c_str());
				}
				printf("|");
			}printf("\n");
		}
	}
	/**********终结符与非终结符的判断**/
	bool isTermin(const string &str)
	{
		bool flag = false;
		for (int i = 0; i < terminNum; i++)
		{
			if (str == termin.at(i))
			{
				flag = true;
				break;
			}
		}
		return flag;
	}

	bool isNon(const string &str)
	{///非终结符判断
		bool flag = false;
		for (int i = 0; i < nonNum; i++)
		{
			if (str == nonTermin.at(i))
			{
				flag = true;
				break;
			}
		}
		return flag;
	}
};
#endif // GRAMMER_H