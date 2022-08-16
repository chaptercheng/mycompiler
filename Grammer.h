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
struct PData//�ķ�����ʽ���ݽڵ�
{
	string genL;//�ķ���
	vector<vector<string>> genR;//�ķ��Ҳ�
	PData(string GENL) :genL(GENL) {}
};
struct ListData//����ʽ�б����ݽڵ�
{
	string genL;
	vector<string> genR;
	ListData(string GL, vector<string> GR) :genL(GL), genR(GR) {}
};
class Grammer
{
public:
	string start;//��ʼ����,�涨��ʼ����Ϊ��һ������ʽ��
	int nonNum, terminNum;//�ս������ս����Ŀ
	vector<string> termin;//�ս������
	vector<string> nonTermin;//���ս��
	map<string, int> firstPos, //��¼���ս����һ����genList�г��ֵ�λ��
		symPos;//�ͷ�����SLR���е�λ��
	vector<PData> P;//����ʽ����
	vector<ListData> genList;//����ʽ�б�
	/****���ļ��ж�ȡ�﷨��Ϣ****/
	bool readGrammer(string filename = "MyGram.txt")
	{
		bool flag = false;
		ifstream in;
		in.open(filename);
		if (in.is_open())//���ķ��ļ�
		{
			string tstr;
			int symP = 0, firstP = 0;
			///��ʽ�����ķ�
			string tempI;
			while (in >> tempI && tempI != "##")
			{//һ��ѭ������һ�����ʽ
				firstPos[tempI] = firstP;//���ս��
				PData pd(tempI);
				symPos[tempI] = symP++;
				nonTermin.push_back(tempI);
				vector<string> strs;
				string tempJ;
				while (in >> tempJ && tempJ != "#")
				{//����һ������ʽ
					firstP++;
					int left = 0, right = tempJ.find("`", 0);
					while (right)//rightΪ0ʱ��������
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
			while (in >> tstr && tstr != "#")//�����ս��
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

	/**********��ʾ����ʽ�ķ��ķ�*************/
	void showGenList()
	{
		printf("(genList:%d,nonNum:%d,terminNum:%d)\n", genList.size(), nonNum, terminNum);
		for (int i = 0, k = 0, t = 0; i<int(genList.size()); i++)//����ķ�
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
	/**********�ս������ս�����ж�**/
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
	{///���ս���ж�
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