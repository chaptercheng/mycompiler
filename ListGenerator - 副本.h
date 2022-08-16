#ifndef GENSLRLIST_H
#define GENSLRLIST_H
#include "Grammer.h"

struct Item {
	int genPos;//产生式在genList中的位置
	int curPos;//产生式当前位置
	Item() {}
	Item(int gen, int cur) :genPos(gen), curPos(cur) {}
	bool operator<(const Item& right) const
	{
		if (genPos != right.genPos)
			return genPos < right.genPos;
		else return (curPos < right.curPos);
	}
	bool operator==(const Item& right) const
	{
		return (genPos == right.genPos && curPos == right.curPos);
	}
};
struct ItemSet
{
	set<Item> itemset;
	int id;
	ItemSet() { id = 0; }
	ItemSet(set<Item> IS, int ID) :itemset(IS), id(ID) {}
	bool operator<(const ItemSet& right) const
	{//重构小于
		return itemset < right.itemset;
	}
	bool operator==(const ItemSet& right) const
	{//重构等于
		return itemset == right.itemset;
		return true;
	}
};
struct SLRNode//分析表的节点
{
	int state;//下一个状态
	bool isReduce;//是否为规约状态，若为规约则state指向规约式
	SLRNode() { state = -1; isReduce = false; }
	SLRNode(int ST, bool ISR) :state(ST), isReduce(ISR) {}
	bool operator==(const SLRNode& right) const
	{
		return (state == right.state && isReduce == right.isReduce);
	}
};
class ListGenerator
{
public:
	Grammer *gram;
	vector< vector<SLRNode> > chart;//SLR分析表
	set<ItemSet> cluster;//项目集簇
	map<string, set<string> > firstSet, followSet;//开始集合与结束集合
	ListGenerator(Grammer *g) : gram(g) {}
	void addFirstToFirst(string src, string des)//将一个firstSet添加到另一个firstSet
	{
		set<string> ::iterator iterJ = firstSet[src].begin();
		for (; iterJ != firstSet[src].end(); iterJ++)//将其加入到first集合
		{
			firstSet[des].insert(*iterJ);
		}
	}
	void addFirstToFollow(string src, string des)//将一个firstSet添加到另一个firstSet
	{
		set<string> ::iterator iterJ = firstSet[src].begin();
		for (; iterJ != firstSet[src].end(); iterJ++)//将其加入到first集合
		{
			if (*iterJ != "$")//follow集合不含空串
				followSet[des].insert(*iterJ);
		}
	}
	void addFollowToFollow(string src, string des)//将一个firstSet添加到另一个firstSet
	{
		set<string> ::iterator iterJ = followSet[src].begin();
		for (; iterJ != followSet[src].end(); iterJ++)//将其加入到first集合
		{
			followSet[des].insert(*iterJ);
		}
	}
	bool first(string str)//求一个非终结符str的first集合，返回该非终结符是否可空
	{
		bool flag = false;
		if (!firstSet[str].empty())//已经存在,不重复求
		{
			set<string>::iterator iter = firstSet[str].find("$");//查找是否可空
			return (iter != firstSet[str].end());//是否有可空元素，以便确定follow集合
		}
		for (int i = 0; i < (int)gram->P.size(); i++)
		{
			if (str == gram->P[i].genL)//找到第i组产生式左部
			{
				for (int j = 0; j < (int)gram->P[i].genR.size(); j++)//第i组产生式的第j个
				{
					if (gram->P[i].genR[j][0] == "&")//空串
					{
						firstSet[str].insert(gram->P[i].genR[j][0]);
						flag = true;
					}
					else if (gram->isTermin(gram->P[i].genR[j][0]))//是终结符
					{
						firstSet[str].insert(gram->P[i].genR[j][0]);
					}
					else if (gram->isNon(gram->P[i].genR[j][0]) && gram->P[i].genR[j][0] != str)//是非终结符且不重复
					{
						bool isEmpty = first(gram->P[i].genR[j][0]);//判断产生式右部第一个符号是否可空
						addFirstToFirst(gram->P[i].genR[j][0], str);
						for (int k = 1; isEmpty &&k < (int)gram->P[i].genR[j].size(); k++)
						{//递归求解，若该非终结符可空则继续求下一个非终结符的first
							if (gram->isTermin(gram->P[i].genR[j][k]))//如果是终结符
							{
								firstSet[str].insert(gram->P[i].genR[j][k]);
								isEmpty = false;
								break;
							}
							else//如果是非终结符
							{
								isEmpty = first(gram->P[i].genR[j][k]);
								addFirstToFirst(gram->P[i].genR[j][k], str);
							}
						}
						if (!isEmpty) firstSet[str].erase("$");//非空去掉$
					}
				}
				break;
			}
		}
		return flag;
	}
	int cursor = 0;
	void follow(string str)//求一个变元的follow集合
	{
		for (int i = 0; i < (int)gram->P.size(); i++)//第i组文法产生式
		{
			for (int j = 0; j < (int)gram->P[i].genR.size(); j++)//第j个产生式
			{
				for (int k = 0; k < (int)gram->P[i].genR[j].size(); k++)//第k个符号
					if (gram->P[i].genR[j][k] == str)
					{
						if ((k + 1) == (int)gram->P[i].genR[j].size())//是最后一个符号且左部不重复
						{
							if (gram->P[i].genL != str)
							{
								follow(gram->P[i].genL);//求左部的follow集合
								addFollowToFollow(gram->P[i].genL, str);//将左部的follow集合加入到当前集合
							}
							else continue;//进入最后一个符号且左部重复，跳出内层循环
							
						}
						else if (gram->isTermin(gram->P[i].genR[j][k + 1]))//后一个符号是终结符
						{
							followSet[str].insert(gram->P[i].genR[j][k + 1]);
						}
						else if (gram->isNon(gram->P[i].genR[j][k + 1]))//后一个符号是非终结符,需要求解后面一串符号的first集合
						{
							bool isEmpty = false, isFlwNull = true;//first为空，follow为空标志
							for (int t = k + 1; t < (int)gram->P[i].genR[j].size(); t++)
							{
								if (gram->isTermin(gram->P[i].genR[j][t]))//有终结符
								{
									followSet[str].insert(gram->P[i].genR[j][t]);
									isFlwNull = false;
									break;
								}
								else if(gram->isNon(gram->P[i].genR[j][t]))//非终结符
								{
									isEmpty = first(gram->P[i].genR[j][t]);//求非终结符的first集合
									addFirstToFollow(gram->P[i].genR[j][t], str);
								}
								else { printf("not nonTermin nor termin in follow func"); exit(0); }
								if (!isEmpty)//非空
								{
									isFlwNull = false;
									break;
								}
							}
							if (isFlwNull && str!= gram->P[i].genL)
							{
								follow(gram->P[i].genL);//求左部的follow集合
								addFollowToFollow(gram->P[i].genL, str);//将左部的follow集合加入到当前集合
							}
						}
						else { printf("%s not nonTermin nor termin in follow func",
							gram->P[i].genR[j][k + 1].c_str()); exit(0); }
					}//if
			}//for j
		}// for i
		if (str == gram->start) followSet[str].insert("#");
	}

	void addToItemSet(string ch, ItemSet *itstPtr)
	{//求以变元ch开始的项目集并加入到itemset中
		set<string> rec;//记录求过的非终结符,便于查重校验
		set<Item> itemSet;//项目集
		queue<string> que;//对于变量循环加入首部非终结符
		set<string>::iterator c_it;
		que.push(ch);
		while (!que.empty())
		{
			string tempCh = que.front();//取出队头
			que.pop();//队头出队
			int gPos = gram->firstPos[tempCh];
			for (int i = 0; gPos < (int)gram->genList.size() && gram->genList[gPos].genL == tempCh; gPos++)
			{//对于每个文法右部非终结符加入itemSet
				itstPtr->itemset.insert(Item(gPos, 0));
				if (gram->isNon(gram->genList[gPos].genR[0]))//查看文法产生式右部是否为非终结符
				{
					c_it = rec.find(gram->genList[gPos].genR[0]);
					if (c_it == rec.end())//未在记录中重复出现
					{
						que.push(gram->genList[gPos].genR[0]);
						rec.insert(gram->genList[gPos].genR[0]);
					}
				}
			}//for
		}//while
	}
	///填分析表
	bool fillChart(int src, string str, int des, bool isRDC)
	{//根据isRDC的状态来确定是否是规约填表还是移进填表
	 //若为规约填表ch为规约式左部，des为规约式位置
		cout << "next we fill chart with";
		cout << "(" << src << "," << str << "," << des << ")" << endl;
		bool flag = true;
		if (gram->symPos[str] == 0 && str != gram->start)//非法字符检查
		{
			gram->symPos.erase(str);
			printf("###########################################################################illigel%s\n", str.c_str());
			return false;
		}
		while ((int)chart.size() <= max(src, des))//先扩展表格
		{
			vector<SLRNode> slrList(gram->symPos.size());
			chart.push_back(slrList);
		}
		if (isRDC)//规约填表
		{
			set<string>::iterator fit = followSet[str].begin();//遍历follow集合填表
			//cout << "a reduce Item in fillchart I" << src << "(" << src << ",";
			for (; fit != followSet[str].end(); fit++)
			{
				SLRNode lrnR(des, true);
				if (chart.at(src).at(gram->symPos[*fit]).state == -1)//该表项为空
				{
					chart.at(src).at(gram->symPos[*fit]) = lrnR;
				}
				else if (!(chart.at(src).at(gram->symPos[*fit]) == lrnR))
				{//表项已经存在无需再填,若存在与将要填写的值不等则出现冲突
					flag = false;
				}
				//cout << *fit << " ";
			}//cout << "," << des << ")" << endl;
			if (!flag)
				cerr << "--------------------------------------reduce collision!" << endl;
		}
		else//移进填表
		{
			SLRNode lrnM(des, isRDC);
			if (chart.at(src).at(gram->symPos[str]).state == -1)//该表项为空
			{
				chart.at(src).at(gram->symPos[str]) = lrnM;
			}
			else if (!(chart.at(src).at(gram->symPos[str]) == lrnM))//已经存在
			{//产生规约规约或移进规约冲突
				flag = false;
			}
			if (!flag)
				cerr << "--------------------------------------move collision!" << endl;
			chart[src][gram->symPos[str]] = lrnM;
		}
		return flag;
	}

	int makeGo(ItemSet *srcSet, string ch, ItemSet *desSet)//扫描Go函数,对项目集中的符号ch做Go
	{//Go函数扫描一个字符到达另一个，state=1表示项目集存在规约
		int state = 0;//求Go的状态
		set<Item>::iterator sit = srcSet->itemset.begin();
		for (; sit != srcSet->itemset.end(); sit++)//遍历源项目集的每一个项目
		{
			if ((int)gram->genList[sit->genPos].genR.size() == sit->curPos
				|| gram->genList[sit->genPos].genR[0] == "$")//到达末尾应当规约
			{
				///将follow集合填表并判断有无冲突,注意是当前项目集的follow
				if (followSet[gram->genList[sit->genPos].genL].empty())
					follow(gram->genList[sit->genPos].genL);//求待规约项目的follow集合
				fillChart(srcSet->id, gram->genList[sit->genPos].genL, sit->genPos, true);/**此处潜在存在问题*/
				state = 1;//存在规约项目
			}
			else if (gram->genList[sit->genPos].genR[sit->curPos] == ch)//当前字符匹配则移进入下一个
			{
				int nextPos = sit->curPos + 1;
				desSet->itemset.insert(Item(sit->genPos, nextPos));
				if (nextPos < (int)gram->genList[sit->genPos].genR.size() &&
					gram->isNon(gram->genList[sit->genPos].genR[nextPos]))//下一个符号为非终结符
				{
					addToItemSet(gram->genList[sit->genPos].genR[nextPos], desSet);
					state = 2;//存在扩展项目
				}
			}
		}
		return state;
	}

	///对项目集集簇的项目集进行遍历
	void traverse()
	{
		queue<ItemSet> itemsQ;//项目集指针队列
		ItemSet src;
		addToItemSet(gram->start, &src);//求开始符号项目集
		cluster.insert(ItemSet(src.itemset, cursor++));//加入到项目集簇
		itemsQ.push(src);
		while (!itemsQ.empty())
		{
			src = itemsQ.front();
			///先求出src里的字母有哪些，然后再遍历
			set<string> reap;//项目集中首字母集合
			set<Item>::iterator sit = src.itemset.begin();
			for (; sit != src.itemset.end(); sit++)
			{
				if (sit->curPos < (int)gram->genList[sit->genPos].genR.size())//当前位置不在产生式末尾
					reap.insert(gram->genList[sit->genPos].genR[sit->curPos]);//加入去重的集合
			}
			if (reap.empty()) reap.insert("$");//去重集合中只有规约的项目集
			itemsQ.pop();
			//对当前项目集，对字母表上每个字母遍历求Go
			//cout << "---------------------------------------Go a ItemSet------------------------------------------" << endl;
			set<string>::iterator pit = reap.begin();
			for (; pit != reap.end(); pit++)
			{
				ItemSet des;
				makeGo(&src, *pit, &des);//对项目集的首字符调用Go，目标集合加入到des
				set<ItemSet>::iterator cit = cluster.end();
				if (!des.itemset.empty() && (cit = cluster.find(des)) == cluster.end())//项目集簇中无项目集des
				{
					des.id = cursor;//目标项目集标号
					itemsQ.push(des);//des项目集入队
					cluster.insert(ItemSet(des.itemset, cursor++));
					fillChart(src.id, *pit, des.id, false);//移进填表
				}
				else if (cit != cluster.end())//找到项目集
				{
					fillChart(src.id, *pit, cit->id, false);//移进填表
				}
			}
		}
	}

	void showCluster()//显示项目集分析表
	{
		///显示项目集
		
		set<ItemSet>::iterator clusIt = cluster.begin();//指向每一个ItemSet
		for (; clusIt != cluster.end(); clusIt++)
		{//项目集簇
			printf("\nItemSet:%-3d:\n", clusIt->id);
			set<Item>::iterator itemIt = clusIt->itemset.begin();
			for (; itemIt != clusIt->itemset.end(); itemIt++)
			{//每个项目
				printf("(%s->", gram->genList[itemIt->genPos].genL.c_str());
				for (int j = 0; j < (int)gram->genList[itemIt->genPos].genR.size(); j++)
					printf(" %s", gram->genList[itemIt->genPos].genR[j].c_str());
				printf(",%d)\n", itemIt->curPos);
			}
		}
		gram->showGenList();///显示文法产生式
		///显示分析表
		map<int, string> order;
		map<string, int>::iterator mit = gram->symPos.begin();
		for (; mit != gram->symPos.end(); mit++)
			order[mit->second] = mit->first;//将键值调序
		cout << "      ";
		for (int i = 0; i < (int)gram->symPos.size(); i++)
		{//输出列表头
			cout << setw(9) << order[i];
		}cout << endl;
		int anotherLine = 0;/*
		for (int i = 0; i < (int)chart.size(); i++)
		{//输出分析表
			printf("%2d:\n\t", i);
			for (int j = 0; j < (int)chart[i].size(); j++)
			{
				anotherLine++;
				printf("(%3d,%d,%-8s) ", chart.at(i).at(j).state, chart.at(i).at(j).isReduce, order[j].c_str());
				if (anotherLine == 10) { printf("\n\t"); anotherLine = 0; }
			}cout << endl; anotherLine = 0;
		}*/
	}
};
#endif // !GENSLRLIST_H
