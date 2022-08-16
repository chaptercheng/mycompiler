#ifndef DAGOPTIMAL_H
#define DAGOPTIMAL_H
#include "SymbolTable.h"
#include <list>
struct DAGNode
{
	int id;//结点的编号
	Elem op;//运算符
	Elem first;//主标记
	list<Elem> sub;//附加标记
	int lChild;//节点左儿子
	int rChild;//节点右儿子
	DAGNode(int ID, Elem OP, Elem FIRST) :op(OP), first(FIRST)
	{
		id = ID;//节点编号
		lChild = rChild = -1;//初始化
	}
	DAGNode(int ID, Elem OP, int L, int R) :op(OP), lChild(L), rChild(R)
	{
		id = ID;//节点编号
	}
	bool operator==(const DAGNode &right)const
	{
		return ((op == right.op) && (lChild == right.lChild) && (rChild == right.rChild) );
	}
	bool operator!=(const DAGNode &right)const
	{
		return !(*this == right);
	}
};

//只要重载了等于符号皆可以用此函数查找
class DagOptimal
{
public:
	int ID = 1;//节点标号
	vector<DAGNode> DAGTree;//DAG树
	vector<Quarter> Quad;//优化后的四元式区
	map<Elem, int>  elemMap;//元素所在节点的位置索引
	DagOptimal() {//构造函数，初始化DAG数，数组下标从1开始
		DAGTree.push_back(DAGNode(-1,Elem(DefaultW,-1), Elem(DefaultW, -1)));//第0个位无效节点
	}
	void attachTo(int d, const Elem &e)//将元素e附加到第i个DAG节点上
	{
		if (DAGTree[d].first.addrPtr == -1)//主标记为空
		{
			DAGTree[d].first = e;
		}
		else if (DAGTree[d].first.kind == nT)//主标记是常数
		{
			if(e.kind == iT)//用户变量放附加标记的前面
				DAGTree[d].sub.push_front(e);
			else//临时变量放后面
				DAGTree[d].sub.push_front(e);
		}
		else if (DAGTree[d].first.kind == Temp)//主标记是临时变量，将临时变量放入附加标记
		{
			DAGTree[d].sub.push_back(DAGTree[d].first);
			DAGTree[d].first = e;
		}
		else if (DAGTree[d].first.kind == iT)//主标记是用户变量
		{
			if (e.kind == nT)//待附加的标记是常数
			{
				DAGTree[d].sub.push_back(DAGTree[d].first);
				DAGTree[d].first = e;
			}
			else
				DAGTree[d].sub.push_front(e);
		}
		elemMap[e] = d;//记录e所在节点
	}
	int calc(int num1, int num2,string op)//根据操作计算常值
	{
		int rel = num1 / num2;
		if (op == "+")
			rel = num1 + num2;
		else if (op == "*")
			rel = num1 * num2;
		else if (op == "-")
			rel = num1 - num2;
		return rel;
	}
	void resetDAG()//优化完一个基本块后可进行清除
	{
		DAGTree.clear();//DAG树节点清空
		DAGTree.push_back(DAGNode(-1, Elem(DefaultW, -1), Elem(DefaultW, -1)));//第0个位无效节点
		elemMap.clear();//记录清空
		ID = 1;
	}
	/**注意在生成节点时,叶子节点的op字段定义为Elem(Default,-1)
	*/
	void genEqual(const Elem &B, const Elem &A)//产生赋值四元式（= ,B,_,A）对应的DAG节点
	{
		Elem op(DefaultW, -1);
		//查找B是否在DAGTree中
		int posB = elemMap[B];
		if (posB == 0)//不在DAGTree中则新建
		{
			elemMap[B] = ID;//记录B所在节点编号
			DAGNode node(ID, op, B);//生成新节点
			DAGTree.push_back(node);//添加新节点
			attachTo(DAGTree.size()-1, A);//A加入到附加到新建节点中中
			ID++;//ID加1便于下次使用
		}
		else//posB不是0，把A附加于B
			attachTo(posB, A);
	}
	bool isSuan(const Elem &op)
	{
		bool flag = false;
		if (op.kind == pT )
			if (PList[op.addrPtr] == "+" || PList[op.addrPtr] == "-" || PList[op.addrPtr] == "*" || PList[op.addrPtr] == "/")
				flag = true;
		return flag;
	}
	void createDAG(int q)//产生第q个四元式的节点
	{
		if (Quarts[q].op.kind == pT && (isSuan(Quarts[q].op) || PList[Quarts[q].op.addrPtr] == "="))
		{
			string op = PList[Quarts[q].op.addrPtr];
			if (op == "=")//(=,B,_,A),A附加于B上
				genEqual(Quarts[q].opl, Quarts[q].des);
			else if (op == "+" || op == "-" || op == "*" || op == "/")
			{
				//if (Quarts[q].opl.kind == aTemp || Quarts[q].opr.kind == aTemp || Quarts[q].des.kind == aTemp)
				//{
				//	DAGTree.push_back(DAGNode(-5, Quarts[q].op, q, q));//有数组参与，暂不优化
				//	ID++;
				//	return;
				//}
				if (Quarts[q].opl.kind == nT && Quarts[q].opr.kind == nT)//左右操作数都是常数
				{
					int num1 = IntList[Quarts[q].opl.addrPtr];//左常数
					int num2 = IntList[Quarts[q].opr.addrPtr];//右常数
					///下面获取运算结果常数元素
					int rlt = calc(num1, num2, op);
					int posN = FindList(rlt, IntList);
					if (posN == -1)
					{
						posN = IntList.size();
						IntList.push_back(rlt);
					}
					Elem elem(nT, posN);//常数元素
					genEqual(elem, Quarts[q].des);//常数赋给目标数
				}
				else
				{
					int posB = elemMap[Quarts[q].opl];//查找B是否定义过
					if (posB == 0)//B没有被定义过
					{
						elemMap[Quarts[q].opl] = ID;
						DAGNode node(ID, Elem(DefaultW, -1), Quarts[q].opl);//生成新的叶子节点
						DAGTree.push_back(node);
						posB = ID;
						ID++;
					}
					int posC = elemMap[Quarts[q].opr];
					if (posC == 0)//C 没有被定义过
					{
						elemMap[Quarts[q].opr] = ID;
						DAGNode node(ID, Elem(DefaultW, -1), Quarts[q].opr);//生成新的叶子节点
						DAGTree.push_back(node);
						posC = ID;
						ID++;
					}
					//判断目标节点A
					DAGNode node(ID, Quarts[q].op, posB, posC);
					int posA = FindList(node, DAGTree);
					if (posA == -1)//没有已有节点，需要新建
					{
						node.first = Quarts[q].des;//将目标数设为新添加节点主标记
						DAGTree.push_back(node);
						elemMap[Quarts[q].des] = ID;//记录新建节点的节点位置
						ID++;
					}
					else//将A附加到找到相应运算的节点
						attachTo(posA, Quarts[q].des);
				}
			}
		}// if pT
		else//op.kind = opK
		{
			DAGTree.push_back(DAGNode(-5, Quarts[q].op, q, q));//用户操作类型,暂不予优化处理
			ID++;
		}
	}
	void backToQuad()//由DAGTree生成优化后的四元式
	{
		for (size_t i = 1; i < DAGTree.size(); i++)//按节点编码顺序读取每一节点的信息
		{
			if (DAGTree[i].id == -5)//还原用户操作
			{
				Quad.push_back(Quarts[DAGTree[i].lChild]);
			}
			else
			{
				if (isSuan(DAGTree[i].op))//是算术运算操作
				{//按照儿子节点进行算术运算
					int L = DAGTree[i].lChild;
					int R = DAGTree[i].rChild;
					Quad.push_back(Quarter(DAGTree[i].op,DAGTree[L].first,DAGTree[R].first,DAGTree[i].first));
				}
				if (!DAGTree[i].sub.empty())
				{
					list<Elem>::iterator jt = DAGTree[i].sub.begin();
					for (; jt != DAGTree[i].sub.end();jt++)
					{//遍历附加标记，是用户变量的话生成赋值四元式
						if (jt->kind != Temp && elemMap[*jt]== i)//非临时变量且有效，未被覆盖
						{//生成A = B
							Quad.push_back(Quarter(Elem(pT, FindList("=", PList)), DAGTree[i].first, Elem(DefaultW, -1), *jt));
						}
					}//for
				}//if sub
			}//else
		}//for DAG节点遍历
	}
	void optimal(const vector<int> seg)
	{
		for (size_t i = 0; i < seg.size(); i += 2)//显示源四元式基本信息
		{
			for (int j = seg[i]; j <= seg[i + 1]; j++)
				createDAG(j);
			backToQuad();
			resetDAG();
		}
	}
};
#endif // !DAGOPTIMAL_H

