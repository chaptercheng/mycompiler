#ifndef GENSLRLIST_H
#define GENSLRLIST_H
#include "Grammer.h"

struct Item {
	int genPos;//����ʽ��genList�е�λ��
	int curPos;//����ʽ��ǰλ��
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
	{//�ع�С��
		return itemset < right.itemset;
	}
	bool operator==(const ItemSet& right) const
	{//�ع�����
		return itemset == right.itemset;
		return true;
	}
};
struct SLRNode//������Ľڵ�
{
	int state;//��һ��״̬
	bool isReduce;//�Ƿ�Ϊ��Լ״̬����Ϊ��Լ��stateָ���Լʽ
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
	vector< vector<SLRNode> > chart;//SLR������
	set<ItemSet> cluster;//��Ŀ����
	map<string, set<string> > firstSet, followSet;//��ʼ�������������
	ListGenerator(Grammer *g) : gram(g) {}
	void addFirstToFirst(string src, string des)//��һ��firstSet��ӵ���һ��firstSet
	{
		set<string> ::iterator iterJ = firstSet[src].begin();
		for (; iterJ != firstSet[src].end(); iterJ++)//������뵽first����
		{
			firstSet[des].insert(*iterJ);
		}
	}
	void addFirstToFollow(string src, string des)//��һ��firstSet��ӵ���һ��firstSet
	{
		set<string> ::iterator iterJ = firstSet[src].begin();
		for (; iterJ != firstSet[src].end(); iterJ++)//������뵽first����
		{
			if (*iterJ != "$")//follow���ϲ����մ�
				followSet[des].insert(*iterJ);
		}
	}
	void addFollowToFollow(string src, string des)//��һ��firstSet��ӵ���һ��firstSet
	{
		set<string> ::iterator iterJ = followSet[src].begin();
		for (; iterJ != followSet[src].end(); iterJ++)//������뵽first����
		{
			followSet[des].insert(*iterJ);
		}
	}
	bool first(string str)//��һ�����ս��str��first���ϣ����ظ÷��ս���Ƿ�ɿ�
	{
		bool flag = false;
		if (!firstSet[str].empty())//�Ѿ�����,���ظ���
		{
			set<string>::iterator iter = firstSet[str].find("$");//�����Ƿ�ɿ�
			return (iter != firstSet[str].end());//�Ƿ��пɿ�Ԫ�أ��Ա�ȷ��follow����
		}
		for (int i = 0; i < (int)gram->P.size(); i++)
		{
			if (str == gram->P[i].genL)//�ҵ���i�����ʽ��
			{
				for (int j = 0; j < (int)gram->P[i].genR.size(); j++)//��i�����ʽ�ĵ�j��
				{
					if (gram->P[i].genR[j][0] == "&")//�մ�
					{
						firstSet[str].insert(gram->P[i].genR[j][0]);
						flag = true;
					}
					else if (gram->isTermin(gram->P[i].genR[j][0]))//���ս��
					{
						firstSet[str].insert(gram->P[i].genR[j][0]);
					}
					else if (gram->isNon(gram->P[i].genR[j][0]) && gram->P[i].genR[j][0] != str)//�Ƿ��ս���Ҳ��ظ�
					{
						bool isEmpty = first(gram->P[i].genR[j][0]);//�жϲ���ʽ�Ҳ���һ�������Ƿ�ɿ�
						addFirstToFirst(gram->P[i].genR[j][0], str);
						for (int k = 1; isEmpty &&k < (int)gram->P[i].genR[j].size(); k++)
						{//�ݹ���⣬���÷��ս���ɿ����������һ�����ս����first
							if (gram->isTermin(gram->P[i].genR[j][k]))//������ս��
							{
								firstSet[str].insert(gram->P[i].genR[j][k]);
								isEmpty = false;
								break;
							}
							else//����Ƿ��ս��
							{
								isEmpty = first(gram->P[i].genR[j][k]);
								addFirstToFirst(gram->P[i].genR[j][k], str);
							}
						}
						if (!isEmpty) firstSet[str].erase("$");//�ǿ�ȥ��$
					}
				}
				break;
			}
		}
		return flag;
	}
	int cursor = 0;
	void follow(string str)//��һ����Ԫ��follow����
	{
		for (int i = 0; i < (int)gram->P.size(); i++)//��i���ķ�����ʽ
		{
			for (int j = 0; j < (int)gram->P[i].genR.size(); j++)//��j������ʽ
			{
				for (int k = 0; k < (int)gram->P[i].genR[j].size(); k++)//��k������
					if (gram->P[i].genR[j][k] == str)
					{
						if ((k + 1) == (int)gram->P[i].genR[j].size())//�����һ���������󲿲��ظ�
						{
							if (gram->P[i].genL != str)
							{
								follow(gram->P[i].genL);//���󲿵�follow����
								addFollowToFollow(gram->P[i].genL, str);//���󲿵�follow���ϼ��뵽��ǰ����
							}
							else continue;//�������һ�����������ظ��������ڲ�ѭ��
							
						}
						else if (gram->isTermin(gram->P[i].genR[j][k + 1]))//��һ���������ս��
						{
							followSet[str].insert(gram->P[i].genR[j][k + 1]);
						}
						else if (gram->isNon(gram->P[i].genR[j][k + 1]))//��һ�������Ƿ��ս��,��Ҫ������һ�����ŵ�first����
						{
							bool isEmpty = false, isFlwNull = true;//firstΪ�գ�followΪ�ձ�־
							for (int t = k + 1; t < (int)gram->P[i].genR[j].size(); t++)
							{
								if (gram->isTermin(gram->P[i].genR[j][t]))//���ս��
								{
									followSet[str].insert(gram->P[i].genR[j][t]);
									isFlwNull = false;
									break;
								}
								else if(gram->isNon(gram->P[i].genR[j][t]))//���ս��
								{
									isEmpty = first(gram->P[i].genR[j][t]);//����ս����first����
									addFirstToFollow(gram->P[i].genR[j][t], str);
								}
								else { printf("not nonTermin nor termin in follow func"); exit(0); }
								if (!isEmpty)//�ǿ�
								{
									isFlwNull = false;
									break;
								}
							}
							if (isFlwNull && str!= gram->P[i].genL)
							{
								follow(gram->P[i].genL);//���󲿵�follow����
								addFollowToFollow(gram->P[i].genL, str);//���󲿵�follow���ϼ��뵽��ǰ����
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
	{//���Ա�Ԫch��ʼ����Ŀ�������뵽itemset��
		set<string> rec;//��¼����ķ��ս��,���ڲ���У��
		set<Item> itemSet;//��Ŀ��
		queue<string> que;//���ڱ���ѭ�������ײ����ս��
		set<string>::iterator c_it;
		que.push(ch);
		while (!que.empty())
		{
			string tempCh = que.front();//ȡ����ͷ
			que.pop();//��ͷ����
			int gPos = gram->firstPos[tempCh];
			for (int i = 0; gPos < (int)gram->genList.size() && gram->genList[gPos].genL == tempCh; gPos++)
			{//����ÿ���ķ��Ҳ����ս������itemSet
				itstPtr->itemset.insert(Item(gPos, 0));
				if (gram->isNon(gram->genList[gPos].genR[0]))//�鿴�ķ�����ʽ�Ҳ��Ƿ�Ϊ���ս��
				{
					c_it = rec.find(gram->genList[gPos].genR[0]);
					if (c_it == rec.end())//δ�ڼ�¼���ظ�����
					{
						que.push(gram->genList[gPos].genR[0]);
						rec.insert(gram->genList[gPos].genR[0]);
					}
				}
			}//for
		}//while
	}
	///�������
	bool fillChart(int src, string str, int des, bool isRDC)
	{//����isRDC��״̬��ȷ���Ƿ��ǹ�Լ������ƽ����
	 //��Ϊ��Լ���chΪ��Լʽ�󲿣�desΪ��Լʽλ��
		cout << "next we fill chart with";
		cout << "(" << src << "," << str << "," << des << ")" << endl;
		bool flag = true;
		if (gram->symPos[str] == 0 && str != gram->start)//�Ƿ��ַ����
		{
			gram->symPos.erase(str);
			printf("###########################################################################illigel%s\n", str.c_str());
			return false;
		}
		while ((int)chart.size() <= max(src, des))//����չ���
		{
			vector<SLRNode> slrList(gram->symPos.size());
			chart.push_back(slrList);
		}
		if (isRDC)//��Լ���
		{
			set<string>::iterator fit = followSet[str].begin();//����follow�������
			//cout << "a reduce Item in fillchart I" << src << "(" << src << ",";
			for (; fit != followSet[str].end(); fit++)
			{
				SLRNode lrnR(des, true);
				if (chart.at(src).at(gram->symPos[*fit]).state == -1)//�ñ���Ϊ��
				{
					chart.at(src).at(gram->symPos[*fit]) = lrnR;
				}
				else if (!(chart.at(src).at(gram->symPos[*fit]) == lrnR))
				{//�����Ѿ�������������,�������뽫Ҫ��д��ֵ��������ֳ�ͻ
					flag = false;
				}
				//cout << *fit << " ";
			}//cout << "," << des << ")" << endl;
			if (!flag)
				cerr << "--------------------------------------reduce collision!" << endl;
		}
		else//�ƽ����
		{
			SLRNode lrnM(des, isRDC);
			if (chart.at(src).at(gram->symPos[str]).state == -1)//�ñ���Ϊ��
			{
				chart.at(src).at(gram->symPos[str]) = lrnM;
			}
			else if (!(chart.at(src).at(gram->symPos[str]) == lrnM))//�Ѿ�����
			{//������Լ��Լ���ƽ���Լ��ͻ
				flag = false;
			}
			if (!flag)
				cerr << "--------------------------------------move collision!" << endl;
			chart[src][gram->symPos[str]] = lrnM;
		}
		return flag;
	}

	int makeGo(ItemSet *srcSet, string ch, ItemSet *desSet)//ɨ��Go����,����Ŀ���еķ���ch��Go
	{//Go����ɨ��һ���ַ�������һ����state=1��ʾ��Ŀ�����ڹ�Լ
		int state = 0;//��Go��״̬
		set<Item>::iterator sit = srcSet->itemset.begin();
		for (; sit != srcSet->itemset.end(); sit++)//����Դ��Ŀ����ÿһ����Ŀ
		{
			if ((int)gram->genList[sit->genPos].genR.size() == sit->curPos
				|| gram->genList[sit->genPos].genR[0] == "$")//����ĩβӦ����Լ
			{
				///��follow��������ж����޳�ͻ,ע���ǵ�ǰ��Ŀ����follow
				if (followSet[gram->genList[sit->genPos].genL].empty())
					follow(gram->genList[sit->genPos].genL);//�����Լ��Ŀ��follow����
				fillChart(srcSet->id, gram->genList[sit->genPos].genL, sit->genPos, true);/**�˴�Ǳ�ڴ�������*/
				state = 1;//���ڹ�Լ��Ŀ
			}
			else if (gram->genList[sit->genPos].genR[sit->curPos] == ch)//��ǰ�ַ�ƥ�����ƽ�����һ��
			{
				int nextPos = sit->curPos + 1;
				desSet->itemset.insert(Item(sit->genPos, nextPos));
				if (nextPos < (int)gram->genList[sit->genPos].genR.size() &&
					gram->isNon(gram->genList[sit->genPos].genR[nextPos]))//��һ������Ϊ���ս��
				{
					addToItemSet(gram->genList[sit->genPos].genR[nextPos], desSet);
					state = 2;//������չ��Ŀ
				}
			}
		}
		return state;
	}

	///����Ŀ�����ص���Ŀ�����б���
	void traverse()
	{
		queue<ItemSet> itemsQ;//��Ŀ��ָ�����
		ItemSet src;
		addToItemSet(gram->start, &src);//��ʼ������Ŀ��
		cluster.insert(ItemSet(src.itemset, cursor++));//���뵽��Ŀ����
		itemsQ.push(src);
		while (!itemsQ.empty())
		{
			src = itemsQ.front();
			///�����src�����ĸ����Щ��Ȼ���ٱ���
			set<string> reap;//��Ŀ��������ĸ����
			set<Item>::iterator sit = src.itemset.begin();
			for (; sit != src.itemset.end(); sit++)
			{
				if (sit->curPos < (int)gram->genList[sit->genPos].genR.size())//��ǰλ�ò��ڲ���ʽĩβ
					reap.insert(gram->genList[sit->genPos].genR[sit->curPos]);//����ȥ�صļ���
			}
			if (reap.empty()) reap.insert("$");//ȥ�ؼ�����ֻ�й�Լ����Ŀ��
			itemsQ.pop();
			//�Ե�ǰ��Ŀ��������ĸ����ÿ����ĸ������Go
			//cout << "---------------------------------------Go a ItemSet------------------------------------------" << endl;
			set<string>::iterator pit = reap.begin();
			for (; pit != reap.end(); pit++)
			{
				ItemSet des;
				makeGo(&src, *pit, &des);//����Ŀ�������ַ�����Go��Ŀ�꼯�ϼ��뵽des
				set<ItemSet>::iterator cit = cluster.end();
				if (!des.itemset.empty() && (cit = cluster.find(des)) == cluster.end())//��Ŀ����������Ŀ��des
				{
					des.id = cursor;//Ŀ����Ŀ�����
					itemsQ.push(des);//des��Ŀ�����
					cluster.insert(ItemSet(des.itemset, cursor++));
					fillChart(src.id, *pit, des.id, false);//�ƽ����
				}
				else if (cit != cluster.end())//�ҵ���Ŀ��
				{
					fillChart(src.id, *pit, cit->id, false);//�ƽ����
				}
			}
		}
	}

	void showCluster()//��ʾ��Ŀ��������
	{
		///��ʾ��Ŀ��
		
		set<ItemSet>::iterator clusIt = cluster.begin();//ָ��ÿһ��ItemSet
		for (; clusIt != cluster.end(); clusIt++)
		{//��Ŀ����
			printf("\nItemSet:%-3d:\n", clusIt->id);
			set<Item>::iterator itemIt = clusIt->itemset.begin();
			for (; itemIt != clusIt->itemset.end(); itemIt++)
			{//ÿ����Ŀ
				printf("(%s->", gram->genList[itemIt->genPos].genL.c_str());
				for (int j = 0; j < (int)gram->genList[itemIt->genPos].genR.size(); j++)
					printf(" %s", gram->genList[itemIt->genPos].genR[j].c_str());
				printf(",%d)\n", itemIt->curPos);
			}
		}
		gram->showGenList();///��ʾ�ķ�����ʽ
		///��ʾ������
		map<int, string> order;
		map<string, int>::iterator mit = gram->symPos.begin();
		for (; mit != gram->symPos.end(); mit++)
			order[mit->second] = mit->first;//����ֵ����
		cout << "      ";
		for (int i = 0; i < (int)gram->symPos.size(); i++)
		{//����б�ͷ
			cout << setw(9) << order[i];
		}cout << endl;
		int anotherLine = 0;/*
		for (int i = 0; i < (int)chart.size(); i++)
		{//���������
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
