#ifndef DAGOPTIMAL_H
#define DAGOPTIMAL_H
#include "SymbolTable.h"
#include <list>
struct DAGNode
{
	int id;//���ı��
	Elem op;//�����
	Elem first;//�����
	list<Elem> sub;//���ӱ��
	int lChild;//�ڵ������
	int rChild;//�ڵ��Ҷ���
	DAGNode(int ID, Elem OP, Elem FIRST) :op(OP), first(FIRST)
	{
		id = ID;//�ڵ���
		lChild = rChild = -1;//��ʼ��
	}
	DAGNode(int ID, Elem OP, int L, int R) :op(OP), lChild(L), rChild(R)
	{
		id = ID;//�ڵ���
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

//ֻҪ�����˵��ڷ��ŽԿ����ô˺�������
class DagOptimal
{
public:
	int ID = 1;//�ڵ���
	vector<DAGNode> DAGTree;//DAG��
	vector<Quarter> Quad;//�Ż������Ԫʽ��
	map<Elem, int>  elemMap;//Ԫ�����ڽڵ��λ������
	DagOptimal() {//���캯������ʼ��DAG���������±��1��ʼ
		DAGTree.push_back(DAGNode(-1,Elem(DefaultW,-1), Elem(DefaultW, -1)));//��0��λ��Ч�ڵ�
	}
	void attachTo(int d, const Elem &e)//��Ԫ��e���ӵ���i��DAG�ڵ���
	{
		if (DAGTree[d].first.addrPtr == -1)//�����Ϊ��
		{
			DAGTree[d].first = e;
		}
		else if (DAGTree[d].first.kind == nT)//������ǳ���
		{
			if(e.kind == iT)//�û������Ÿ��ӱ�ǵ�ǰ��
				DAGTree[d].sub.push_front(e);
			else//��ʱ�����ź���
				DAGTree[d].sub.push_front(e);
		}
		else if (DAGTree[d].first.kind == Temp)//���������ʱ����������ʱ�������븽�ӱ��
		{
			DAGTree[d].sub.push_back(DAGTree[d].first);
			DAGTree[d].first = e;
		}
		else if (DAGTree[d].first.kind == iT)//��������û�����
		{
			if (e.kind == nT)//�����ӵı���ǳ���
			{
				DAGTree[d].sub.push_back(DAGTree[d].first);
				DAGTree[d].first = e;
			}
			else
				DAGTree[d].sub.push_front(e);
		}
		elemMap[e] = d;//��¼e���ڽڵ�
	}
	int calc(int num1, int num2,string op)//���ݲ������㳣ֵ
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
	void resetDAG()//�Ż���һ���������ɽ������
	{
		DAGTree.clear();//DAG���ڵ����
		DAGTree.push_back(DAGNode(-1, Elem(DefaultW, -1), Elem(DefaultW, -1)));//��0��λ��Ч�ڵ�
		elemMap.clear();//��¼���
		ID = 1;
	}
	/**ע�������ɽڵ�ʱ,Ҷ�ӽڵ��op�ֶζ���ΪElem(Default,-1)
	*/
	void genEqual(const Elem &B, const Elem &A)//������ֵ��Ԫʽ��= ,B,_,A����Ӧ��DAG�ڵ�
	{
		Elem op(DefaultW, -1);
		//����B�Ƿ���DAGTree��
		int posB = elemMap[B];
		if (posB == 0)//����DAGTree�����½�
		{
			elemMap[B] = ID;//��¼B���ڽڵ���
			DAGNode node(ID, op, B);//�����½ڵ�
			DAGTree.push_back(node);//����½ڵ�
			attachTo(DAGTree.size()-1, A);//A���뵽���ӵ��½��ڵ�����
			ID++;//ID��1�����´�ʹ��
		}
		else//posB����0����A������B
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
	void createDAG(int q)//������q����Ԫʽ�Ľڵ�
	{
		if (Quarts[q].op.kind == pT && (isSuan(Quarts[q].op) || PList[Quarts[q].op.addrPtr] == "="))
		{
			string op = PList[Quarts[q].op.addrPtr];
			if (op == "=")//(=,B,_,A),A������B��
				genEqual(Quarts[q].opl, Quarts[q].des);
			else if (op == "+" || op == "-" || op == "*" || op == "/")
			{
				//if (Quarts[q].opl.kind == aTemp || Quarts[q].opr.kind == aTemp || Quarts[q].des.kind == aTemp)
				//{
				//	DAGTree.push_back(DAGNode(-5, Quarts[q].op, q, q));//��������룬�ݲ��Ż�
				//	ID++;
				//	return;
				//}
				if (Quarts[q].opl.kind == nT && Quarts[q].opr.kind == nT)//���Ҳ��������ǳ���
				{
					int num1 = IntList[Quarts[q].opl.addrPtr];//����
					int num2 = IntList[Quarts[q].opr.addrPtr];//�ҳ���
					///�����ȡ����������Ԫ��
					int rlt = calc(num1, num2, op);
					int posN = FindList(rlt, IntList);
					if (posN == -1)
					{
						posN = IntList.size();
						IntList.push_back(rlt);
					}
					Elem elem(nT, posN);//����Ԫ��
					genEqual(elem, Quarts[q].des);//��������Ŀ����
				}
				else
				{
					int posB = elemMap[Quarts[q].opl];//����B�Ƿ����
					if (posB == 0)//Bû�б������
					{
						elemMap[Quarts[q].opl] = ID;
						DAGNode node(ID, Elem(DefaultW, -1), Quarts[q].opl);//�����µ�Ҷ�ӽڵ�
						DAGTree.push_back(node);
						posB = ID;
						ID++;
					}
					int posC = elemMap[Quarts[q].opr];
					if (posC == 0)//C û�б������
					{
						elemMap[Quarts[q].opr] = ID;
						DAGNode node(ID, Elem(DefaultW, -1), Quarts[q].opr);//�����µ�Ҷ�ӽڵ�
						DAGTree.push_back(node);
						posC = ID;
						ID++;
					}
					//�ж�Ŀ��ڵ�A
					DAGNode node(ID, Quarts[q].op, posB, posC);
					int posA = FindList(node, DAGTree);
					if (posA == -1)//û�����нڵ㣬��Ҫ�½�
					{
						node.first = Quarts[q].des;//��Ŀ������Ϊ����ӽڵ������
						DAGTree.push_back(node);
						elemMap[Quarts[q].des] = ID;//��¼�½��ڵ�Ľڵ�λ��
						ID++;
					}
					else//��A���ӵ��ҵ���Ӧ����Ľڵ�
						attachTo(posA, Quarts[q].des);
				}
			}
		}// if pT
		else//op.kind = opK
		{
			DAGTree.push_back(DAGNode(-5, Quarts[q].op, q, q));//�û���������,�ݲ����Ż�����
			ID++;
		}
	}
	void backToQuad()//��DAGTree�����Ż������Ԫʽ
	{
		for (size_t i = 1; i < DAGTree.size(); i++)//���ڵ����˳���ȡÿһ�ڵ����Ϣ
		{
			if (DAGTree[i].id == -5)//��ԭ�û�����
			{
				Quad.push_back(Quarts[DAGTree[i].lChild]);
			}
			else
			{
				if (isSuan(DAGTree[i].op))//�������������
				{//���ն��ӽڵ������������
					int L = DAGTree[i].lChild;
					int R = DAGTree[i].rChild;
					Quad.push_back(Quarter(DAGTree[i].op,DAGTree[L].first,DAGTree[R].first,DAGTree[i].first));
				}
				if (!DAGTree[i].sub.empty())
				{
					list<Elem>::iterator jt = DAGTree[i].sub.begin();
					for (; jt != DAGTree[i].sub.end();jt++)
					{//�������ӱ�ǣ����û������Ļ����ɸ�ֵ��Ԫʽ
						if (jt->kind != Temp && elemMap[*jt]== i)//����ʱ��������Ч��δ������
						{//����A = B
							Quad.push_back(Quarter(Elem(pT, FindList("=", PList)), DAGTree[i].first, Elem(DefaultW, -1), *jt));
						}
					}//for
				}//if sub
			}//else
		}//for DAG�ڵ����
	}
	void optimal(const vector<int> seg)
	{
		for (size_t i = 0; i < seg.size(); i += 2)//��ʾԴ��Ԫʽ������Ϣ
		{
			for (int j = seg[i]; j <= seg[i + 1]; j++)
				createDAG(j);
			backToQuad();
			resetDAG();
		}
	}
};
#endif // !DAGOPTIMAL_H

