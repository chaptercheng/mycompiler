#ifndef DESCODE_H
#define DESCODE_H
#include "SymbolTable.h"
/*��Ŀ�����������ǻ���8086ָ��ϵͳ��
**Ϊ�˼򻯳�����ƣ���Ŀ����������ֻ
**֧�ֶ����ͱ��������������㣬��Ϊ������
**�����ֽڵĴ�������
*/
struct RDL//�Ĵ���������
{
	Elem data;//�Ĵ���������ĸ�����
	string name;//�Ĵ���������
	int L;//�Ĵ�����ǰ������ı�����Ծ��Ϣ��¼
	RDL(Elem E, string Name) :data(E), name(Name) { L = -2; }
};

class DesCoder//Ŀ�����������
{
public:
	vector<string> desCode;
	vector<RDL> reg;//�Ĵ�����Ϣ
	stack<int> semStk;//������Ϣ����ջ
	const int N = 4;//ͨ�üĴ�����Ŀ
	vector<Quarter> *optQ;//���Ż������Ԫʽ����Ŀ�����
	DesCoder(vector<Quarter> *optq) :optQ(optq) {
		reg.push_back(RDL(Elem(DefaultW, -1), "AX"));
		reg.push_back(RDL(Elem(DefaultW, -1), "BX"));
		reg.push_back(RDL(Elem(DefaultW, -1), "CX"));
		reg.push_back(RDL(Elem(DefaultW, -1), "DX"));
		reg.push_back(RDL(Elem(DefaultW, -1), "BP"));//��ͨ�üĴ���
		reg.push_back(RDL(Elem(DefaultW, -1), "SI"));//��ͨ�üĴ���
		reg.push_back(RDL(Elem(DefaultW, -1), "DI"));//��ͨ�üĴ���
	}
	bool isVT(const Elem &e)//�Ǳ�������ʱ��Ԫ����д��Ծ��Ϣʱ�������жϣ�
	{
		if (e.kind != iT && e.kind != Temp)//
			return false;
		return (SymbolTable[e.addrPtr].cat == VAR || SymbolTable[e.addrPtr].cat == VARF
			|| SymbolTable[e.addrPtr].cat == VARN || SymbolTable[e.addrPtr].cat == TEMP);
	}
	void update(unsigned i)//���ݵ�i����Ԫʽ���¼Ĵ�����Ծ��Ϣ������
	{
		int j = areSave((*optQ)[i].opl, reg.size());
		if (j != -1)
			reg[j].L = (*optQ)[i].leftL;
		j = areSave((*optQ)[i].opr, reg.size());
		if (j != -1)
			reg[j].L = (*optQ)[i].rightL;
		j = areSave((*optQ)[i].des, reg.size());
		if (j != -1)
			reg[j].L = (*optQ)[i].desL;
	}
	void enroll(int r, const Elem &e, int L)//��q����Ԫʽ��ĳ�����Ϣ�Ǽ��ڼĴ���r��
	{
		reg[r].data = e;
		reg[r].L = L;
	}
	int selectR()//�Ĵ���ѡ��,��ѡ����������Ծ�ģ�
	{
		int maxx = reg[0].L;
		int s = 0;
		for (int i = 0; i < N; i++)
		{
			if (reg[i].L < -1)//�ջ򲻻�Ծ
			{
				s = i; break;
			}
			else if (reg[i].L == -1)//��Ծ
			{
				maxx = SIZE_MAX;//��Զ����
			}
			else if (maxx < reg[i].L)//������Զ��ѡ��
			{
				maxx = reg[i].L;
				s = i;
			}
		}
		return s;
	}
	int areSave(const Elem &e, int n)//���صڼ���ͨ�üĴ�������������e��ֵ��û�з���-1
	{
		int flag = n - 1;
		while (flag > -1 && reg[flag].data != e)
			flag--;
		return flag;
	}
	int areIdle()//�Ƿ��п��мĴ���
	{
		int flag = N - 1;//��ͨ�üĴ��������뵽���мĴ����ķ�������
		while (flag > -1 && reg[flag].data.addrPtr != -1)//�Ӻ���ǰ�ҿռĴ���
			flag--;
		return flag;
	}

	//�Ի�����Ϊ��λ���л�Ծ��Ϣ�ı��
	void actives(int s, int d)
	{
		for (int i = d; i >= s; i--)//����ɨ�������������Ԫʽ
		{
			if (isVT((*optQ)[i].des) || (*optQ)[i].des.kind == aTemp)//Ŀ�����Ǳ���
			{
				(*optQ)[i].desL = SymbolTable[(*optQ)[i].des.addrPtr].activeL;
				SymbolTable[(*optQ)[i].des.addrPtr].activeL = -2;//n
			}
			if (isVT((*optQ)[i].opr) || (*optQ)[i].opr.kind == aTemp)//�Ҳ������Ǳ���
			{
				(*optQ)[i].rightL = SymbolTable[(*optQ)[i].opr.addrPtr].activeL;
				SymbolTable[(*optQ)[i].opr.addrPtr].activeL = i;//�õ���λ��
			}
			else if ((*optQ)[i].opl.kind == nT) (*optQ)[i].leftL = 0;
			if (isVT((*optQ)[i].opl) || (*optQ)[i].opl.kind == aTemp)//��������Ǳ���
			{
				(*optQ)[i].leftL = SymbolTable[(*optQ)[i].opl.addrPtr].activeL;
				SymbolTable[(*optQ)[i].opl.addrPtr].activeL = i;//�õ���λ��
			}
			else if ((*optQ)[i].opr.kind == nT) (*optQ)[i].rightL = 0;
			if ((*optQ)[i].op.kind == opK && OpList[(*optQ)[i].op.addrPtr] == "of")//�Լ��������ڴ��ַ����Ԫʽ���⴦��
			{
				(*optQ)[i].leftL = -2;//���������Ծ��Ϣ
				if ((*optQ)[i].desL < -1)
					(*optQ)[i].desL = -1;//Ŀ������Ծ��Ϣ
			}
		}ResetL();
	}
	void labelL(const vector<int> &seg)//Ϊÿ����Ԫʽ��ע��Ծ��Ϣ
	{
		for (size_t i = 0; i < seg.size(); i += 2)
		{
			actives(seg[i], seg[i + 1]);
		}
	}
	void beginDC(const vector<int> &seg)//Ŀ�����Ķ��岿�ֵ�
	{
		desCode.push_back("DATAS SEGMENT\n");
		for (size_t i = 0; i < SymbolTable.size(); i++)//��ÿһ����������DUP
		{
			stringstream codes;
			int unit = 0;
			if (SymbolTable[i].cat == VAR || SymbolTable[i].cat == VARF
				|| SymbolTable[i].cat == VARN || SymbolTable[i].cat == TEMP)//ֻ�Ա������з�������
			{
				if (TypeList[SymbolTable[i].typePtr].typeName <= VOID)//��������
				{
					unit = TypeList[SymbolTable[i].typePtr].listPtr;
					codes << "\t" << SymbolTable[i].name << " DB " << unit << " DUP(0)\n";//���ݶ���
				}
				else if (TypeList[SymbolTable[i].typePtr].typeName == ARRAY)//��������
				{
					int p = TypeList[SymbolTable[i].typePtr].listPtr;//��ȡָ��������ָ��
					unit = ArrayList[p].valNum;
					if (TypeList[ArrayList[p].typePtr].typeName < VOID && !nTVec.empty())//��1ά�����ұ���ʼ���ˣ�
					{
						int num = unit / TypeList[ArrayList[p].typePtr].listPtr;
						if (nTVec.size() == num)
						{
							codes << "\t" << SymbolTable[i].name << " DD " << nTVec[0];
							for (size_t n = 1; n < nTVec.size(); n++)
							{
								codes << ", " << nTVec[n];
							}
							codes << endl;
						}
						else //����ĸ������ԣ���ʧЧ
						{
							printf("your array's initial is invalid due to the number is not correct.\n");
							codes << "\t" << SymbolTable[i].name << " DB " << unit << " DUP(0)\n";//���ݶ���
						}
					}
					else
					{
						codes << "\t" << SymbolTable[i].name << " DB " << unit << " DUP(0)\n";//���ݶ���
					}
				}
				desCode.push_back(codes.str());//��ӵ�������
				codes.str("");
			}
		}
		desCode.push_back("DATAS ENDS\n");
		desCode.push_back("STACKS SEGMENT\n");
		desCode.push_back("STACKS ENDS\n");
		desCode.push_back("CODES SEGMENT\n");
		desCode.push_back("\tASSUME CS : CODES, DS : DATAS, SS : STACKS\n");
		desCode.push_back("START :\n");
		desCode.push_back("\tMOV AX, DATAS\n");
		desCode.push_back("\tMOV DS, AX\n");
		desCode.push_back("\tXOR DX, DX\n");//��ʼ������
		///Ŀ����벿��
		for (size_t i = 0; i < seg.size(); i += 2)
		{
			genDesCode(seg[i], seg[i + 1]);
		}
		desCode.push_back("\tMOV AH, 4CH\n");
		desCode.push_back("\tINT 21H\n");
		desCode.push_back("CODES ENDS\n");
		desCode.push_back("\tEND START\n");
	}
	void jumpF(int i)//����if��do����Ԫʽ��ת
	{
		stringstream codes;
		saveActive();//�����Ծ����
		if ((*optQ)[i].opl.kind == nT)//����
		{
			codes << "\tMOV BP, " << IntList[(*optQ)[i].opl.addrPtr] << endl;
		}
		else if (isVT((*optQ)[i].opl))//����
		{
			if (reg[BP].data.kind == iT)//opl��ֵ����BP�Ĵ���
			{
				codes << "\tMOV BP, " << "WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
			}
		}
		else { printf("not Int nor Variable.\n"); }
		codes << "\tOR BP, 0\n";//���Ա�־λ
		desCode.push_back(codes.str());
		codes.str("");
		semStk.push(desCode.size());//��������ѹջ
		codes << "\tJZ SEG";//JF��ת��������
		desCode.push_back(codes.str());
		enroll(BP, Elem(DefaultW, -1), -2);//BP�Ĵ������
		codes.str("");//���
	}
	void saveActive()//��������������Ծ����
	{
		stringstream codes;
		for (int i = 0; i < (int)reg.size() - 1; i++)
		{
			if (reg[i].data.addrPtr > -1)
			{
				if (reg[i].L > -2 && isVT(reg[i].data))//��Ծ
					codes << "\tMOV WORD PTR " << SymbolTable[reg[i].data.addrPtr].name << ", " << reg[i].name << endl;
				else if (reg[i].L > -2 && reg[i].data.kind == aTemp)
				{
					moveToArr(reg[i].data, i, codes);
				}
				reg[i].data = Elem(DefaultW, -1);//��ռĴ���
				reg[i].L = -2;
			}
		}
		desCode.push_back(codes.str());
		codes.str("");
		///�������Ծ������Ĵ������
	}
	void haveB(unsigned i, int bSave)//��i����Ԫʽ�������������flaSaveָ���ļĴ������������ͷ�
	{
		stringstream codes;
		if ((*optQ)[i].leftL > -2)//B ��Ծ
		{
			int flagIdle = areIdle();
			if (flagIdle != -1)//�п��мĴ���
			{
				if (isVT((*optQ)[i].opl) || (*optQ)[i].opl.kind == aTemp)//����
				{
					codes << "\tMOV " << reg[flagIdle].name << ", " << reg[bSave].name << endl;
					enroll(flagIdle, reg[bSave].data, reg[bSave].L);//B�Ǽǵ���һ�Ĵ���
				}
				else { printf("not is a variable!\n"); }
			}
			else//û�п��мĴ�������B�������ڴ���
			{
				if (isVT((*optQ)[i].opl))//�Ǳ���,�һ�Ծ�����浽M
					codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << "," << reg[bSave].name << endl;
				else if ((*optQ)[i].opl.kind == aTemp)
					moveToArr((*optQ)[i].opl, bSave, codes);
			}
		}
		// B ����Ծ,ֱ�ӽ�B��ֵ����
		if ((*optQ)[i].op.kind == pT)
		{
			if (PList[(*optQ)[i].op.addrPtr] == "+" || PList[(*optQ)[i].op.addrPtr] == "-")//+����
			{
				string opStr = "\tADD ";
				if (PList[(*optQ)[i].op.addrPtr] == "-") opStr = "\tSUB ";
				if ((*optQ)[i].opr.kind == nT)//�Ҳ���������
					codes << opStr << reg[bSave].name << "," << IntList[(*optQ)[i].opr.addrPtr] << endl;
				else if (isVT((*optQ)[i].opr))//����
				{
					int rSave = areSave((*optQ)[i].opr, N);
					if (rSave == -1)//û�мĴ�������C
						codes << opStr << reg[bSave].name << ", WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
					else
						codes << opStr << reg[bSave].name << ", " << reg[rSave].name << endl;
				}
				else if ((*optQ)[i].opr.kind == aTemp)//�Ҳ�����Ϊ�����ڴ��ַ
				{
					int rSave = areSave((*optQ)[i].opr, reg.size());
					if (rSave == -1)//û�мĴ�������C
					{
						codes << "\tMOV DI" << ", WORD PTR" << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
						codes << opStr << reg[bSave].name << ", [DI]" << endl;
					}
					else if (rSave == SI)
						codes << opStr << reg[bSave].name << ", [SI]" << endl;
					else//ͨ�üĴ���
						codes << opStr << reg[bSave].name << ", " << reg[rSave].name << endl;
				}
				else { printf("not Int nor Variable\n"); }
				enroll(bSave, (*optQ)[i].des, (*optQ)[i].desL);//��q����Ԫʽ��des��Ϣ�Ǽ��ڼĴ���r��
			}
			else if (PList[(*optQ)[i].op.addrPtr] == "*" || PList[(*optQ)[i].op.addrPtr] == "/")
			{
				//��Ҫ���DX������AX����ǰ�������ǵ�ֵ
				if (reg[DX].data.addrPtr != -1)//DX����
				{
					if (reg[DX].data.kind == aTemp)
						moveToArr(reg[DX].data, DX, codes);
					else
						codes << "\tMOV WORD PTR " << SymbolTable[reg[DX].data.addrPtr].name << ", " << reg[DX].name << endl;
					enroll(DX, Elem(DefaultW, -1), -2);
				}
				if (reg[AX].data.addrPtr != -1 && bSave != 0)//AX���գ���flagSave����AX��ֵ
				{//����AX��ֵ�������������AX
					if (reg[AX].data.kind == aTemp)
						moveToArr(reg[AX].data, AX, codes);
					else //һ�����
						codes << "\tMOV WORD PTR " << SymbolTable[reg[AX].data.addrPtr].name << ", " << reg[AX].name << endl;
					codes << "\tMOV AX," << reg[bSave].name << endl;
				}
				int rSave = areSave((*optQ)[i].opr, reg.size());
				if (rSave == -1)//û�мĴ�������C
				{
					if ((*optQ)[i].opr.kind == nT)//����
					{
						codes << "\tMOV BP," << IntList[(*optQ)[i].opr.addrPtr] << endl;
						if (PList[(*optQ)[i].op.addrPtr] == "/")
						{
							codes << "\tXOR DX, DX\n";
							codes << "\tDIV BP" << endl;
						}
						else { codes << "\tMUL BP" << endl; }
					}
					else if (isVT((*optQ)[i].opr))
					{
						if (PList[(*optQ)[i].op.addrPtr] == "/")
						{
							codes << "\tXOR DX, DX\n";
							codes << "\tDIV WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
						}
						else
						{
							codes << "\tMUL WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
						}
					}
					else if ((*optQ)[i].opr.kind == aTemp)//�����ڴ��ַ
					{
						codes << "\tMOV DI, WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
						codes << "\tXOR DX, DX\n";
						if (PList[(*optQ)[i].op.addrPtr] == "/")
							codes << "\tDIV WORD PTR[DI]" << endl;
						else// *
							codes << "\tMUL WORD PTR[DI]" << endl;
					}
					else { printf("not Int nor Variable.\n"); }
				}
				else//C����rSave����ļĴ�����
				{
					codes << "\tXOR DX, DX\n";
					if (PList[(*optQ)[i].op.addrPtr] == "/")
						codes << "\tDIV " << reg[rSave].name << endl;
					else codes << "\tMUL " << reg[rSave].name << endl;
				}///ע��Ϊ�����������������DX�������
				enroll(AX, (*optQ)[i].des, (*optQ)[i].desL);//�Ǽ���������AX
			}
		}
		else { printf("not + -  * /\n"); };
		desCode.push_back(codes.str());//���������
		codes.str("");//��������գ��ͷ��ڴ�
	}//haveB
	void forceB(unsigned i, int r)//ͨ��ǿ���ͷ���B��ֵ��rָ��ļĴ�����
	{
		stringstream codes;
		if (PList[(*optQ)[i].op.addrPtr] == "+" || PList[(*optQ)[i].op.addrPtr] == "-")//+����
		{
			string opStr = "\tADD ";//ȡ����ָ��
			if (PList[(*optQ)[i].op.addrPtr] == "-")
				opStr = "\tSUB ";
			int rSave = areSave((*optQ)[i].opr, reg.size());//�Ĵ�������Ѱ���Ҳ�����
			if ((*optQ)[i].opr.kind == nT)//�Ҳ���������
				codes << opStr << reg[r].name << "," << IntList[(*optQ)[i].opr.addrPtr] << endl;
			else if (isVT((*optQ)[i].opr))//����
			{
				if (rSave == -1)//û�мĴ�������C
					codes << opStr << reg[r].name << ", WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
				else
					codes << opStr << reg[r].name << ", " << reg[rSave].name << endl;
			}
			else if ((*optQ)[i].opr.kind == aTemp)//�����ڴ��ַ
			{
				if (rSave != -1 && rSave != SI)//��ͨ�üĴ����б���C
				{
					codes << opStr << reg[r].name << ", " << reg[rSave].name << endl;
				}
				else if (rSave == SI)
					codes << opStr << reg[r].name << ", WORD PTR[SI]" << endl;
				else
				{
					codes << "\tMOV DI, WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
					codes << opStr << reg[r].name << ", WORD PTR[DI]" << endl;
				}
			}
			else { printf("not Int nor Variable\n"); }
			enroll(r, (*optQ)[i].des, (*optQ)[i].desL);//��i����Ԫʽ��des��Ϣ�Ǽ��ڼĴ���r��
		}
		else if (PList[(*optQ)[i].op.addrPtr] == "*" || PList[(*optQ)[i].op.addrPtr] == "/")
		{
			//��Ҫ��ǰ���DX��AX
			if (reg[DX].data.addrPtr != -1)//DX����
			{
				if (reg[DX].data.kind == aTemp)
					moveToArr(reg[DX].data, DX, codes);
				else
					codes << "\tMOV WORD PTR " << SymbolTable[reg[DX].data.addrPtr].name << ", " << reg[DX].name << endl;
				enroll(DX, Elem(DefaultW, -1), -2);
			}
			if (reg[AX].data.addrPtr != -1 && r != AX)//AX���գ�����r����AX��ֵ
			{//����AX��ֵ�������������AX
				if (reg[AX].data.kind == aTemp)
					moveToArr(reg[AX].data, AX, codes);
				else//һ�����
					codes << "\tMOV WORD PTR " << SymbolTable[reg[AX].data.addrPtr].name << ", " << reg[AX].name << endl;
				codes << "\tMOV AX," << reg[r].name << endl;
			}
			int rSave = areSave((*optQ)[i].opr, N);
			if (rSave == -1)//û�мĴ�������C
			{
				if ((*optQ)[i].opr.kind == nT)//����
				{
					codes << "\tXOR DX, DX\n";
					codes << "\tMOV BP," << IntList[(*optQ)[i].opr.addrPtr] << endl;
					if (PList[(*optQ)[i].op.addrPtr] == "/")
						codes << "\tDIV BP" << endl;
					else { codes << "\tMUL BP" << endl; }
				}
				else if (isVT((*optQ)[i].opr))
				{
					codes << "\tXOR DX, DX\n";
					if (PList[(*optQ)[i].op.addrPtr] == "/")
						codes << "\tDIV WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
					else
						codes << "\tMUL WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
				}
				else if ((*optQ)[i].opr.kind == aTemp)
				{
					codes << "\tMOV DI, WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
					codes << "\tXOR DX, DX\n";
					if (PList[(*optQ)[i].op.addrPtr] == "/")
						codes << "\tDIV WORD PTR[DI]" << endl;
					else// *
						codes << "\tMUL WORD PTR[DI]" << endl;
				}
			}
			else//C����rSave����ļĴ�����
			{
				codes << "\tXOR DX, DX\n";
				if (PList[(*optQ)[i].op.addrPtr] == "/")
					codes << "\tDIV " << reg[rSave].name << endl;
				else codes << "\tMUL " << reg[rSave].name << endl;
			}///ע��Ϊ�����������������DX�������
			enroll(AX, (*optQ)[i].des, (*optQ)[i].desL);//�Ǽ���������AX
		}
		desCode.push_back(codes.str());//���������
		codes.str("");//��������գ��ͷ��ڴ�
	}//forceB

	void moveToReg(int r, const Elem &arr, stringstream &codes)//ͨ�üĴ���ֵ<-����Ԫ��
	{
		//��Ҫ���������±�ֵ����DI��
		codes << "\tMOV DI, WORD PTR " << SymbolTable[arr.addrPtr].name << endl;
		codes << "\tMOV " << reg[r].name << ", [DI]" << endl;
	}
	void moveToArr(const Elem &arr, int r, stringstream &codes)//����Ԫ��<-ͨ�üĴ���ֵ
	{

		if (areSave(arr, reg.size()) == SI && r != SI)//�±�����SI��
			codes << "\tMOV [SI]," << reg[r].name << endl;
		else if (r != SI)//���������±�ֵ����DI��,��������
		{
			codes << "\tMOV DI, WORD PTR " << SymbolTable[arr.addrPtr].name << endl;
			codes << "\tMOV [DI]," << reg[r].name << endl;
		}

	}
	bool genDesCode(int s, int d)//�Ի�����Ϊ��λ����Ŀ�����
	{
		for (int i = s; i <= d; i++)
		{
			stringstream codes;
			if ((*optQ)[i].op.kind == pT)//�ǽ����+ - * / > < ==��
			{
				string op(PList[(*optQ)[i].op.addrPtr]);
				//q:(w,B,C,A)
				update(i);//���¼Ĵ����Ļ�Ծ��Ϣ
				if (op == "+" || op == "-" || op == "*" || op == "/" )//��������,�Ƚ�����
				{
					int flagSave = areSave((*optQ)[i].opl, N);
					if (flagSave != -1)//�мĴ���������B
					{
						haveB(i, flagSave);
					}
					else//û�мĴ�������B��B�����Ǳ�������������
					{
						int s = selectR();
						if (reg[s].data.addrPtr > -1)//�����ݣ�����ԭ�мĴ�����ֵ,Ȼ��Ĵ����������
						{
							//�ж�������һ���������������ڴ��ַ
							if (reg[s].data.kind == aTemp)
								moveToArr(reg[s].data, s, codes);
							else if (reg[s].L > -2 || reg[s].data == (*optQ)[i].opr)//һ�������Ծ�������Ҳ�����
							{
								codes << "\tMOV WORD PTR " << SymbolTable[reg[s].data.addrPtr].name << ", " << reg[s].name << endl;
								enroll(s, (*optQ)[i].des, (*optQ)[i].desL);
							}
						}
						///���潫B ֵ�������ᵽ�ļĴ�����
						if ((*optQ)[i].opl.kind == nT)//����
						{
							codes << "\tMOV " << reg[s].name << ", " << IntList[(*optQ)[i].opl.addrPtr] << endl;
						}
						else if (isVT((*optQ)[i].opl)) //����
						{
							codes << "\tMOV " << reg[s].name << ", WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
						}
						else if ((*optQ)[i].opl.kind == aTemp)//������ڴ��ַ
						{
							moveToReg(s, (*optQ)[i].opl, codes);
						}
						else { printf("not is a Int nor Variable\n"); }
						desCode.push_back(codes.str());
						codes.str("");//���������
						forceB(i, s);//B��sָ��ļĴ���
					}
				}//��������
				else if (op == "=")//q:(=,B,_,A)��ֵ����
				{
					if ((*optQ)[i].opl.kind == nT)//������ֵ������
					{
						if ((*optQ)[i].des.kind == aTemp)
						{
							if (areSave((*optQ)[i].des, reg.size()) != SI)//�����ַ����SI�Ĵ���
							{
								codes << "\tMOV SI , WORD PTR " << SymbolTable[(*optQ)[i].des.addrPtr].name << endl;
								enroll(SI, (*optQ)[i].des, (*optQ)[i].desL);
							}
							codes << "\tMOV WORD PTR[SI], " << IntList[(*optQ)[i].opl.addrPtr] << endl;
						}
						else//һ�����
							codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].des.addrPtr].name << ", "
							<< IntList[(*optQ)[i].opl.addrPtr] << endl;
					}
					else if (isVT((*optQ)[i].opl) || (*optQ)[i].opl.kind == aTemp)//����
					{
						int flagSave = areSave((*optQ)[i].opl, N);//���ҼĴ�������Bֵ
						if (flagSave != -1)//B �����ڼĴ�����
						{
							if ((*optQ)[i].leftL > -2)//B��Ծ,����
							{
								if ((*optQ)[i].opl.kind == aTemp && flagSave != SI)
								{
									moveToArr((*optQ)[i].opl, flagSave, codes);
								}
								else//һ�����
									codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << ", " << reg[flagSave].name << endl;
							}
							//B����Ծ��ֱ�Ӹ���
							enroll(flagSave, (*optQ)[i].des, (*optQ)[i].desL);
						}
						else //B=0���Ĵ�����
						{
							int s = selectR();//����һ���Ĵ���
							if (reg[s].data.addrPtr > -1)//�����ݣ�����ԭ�мĴ�����ֵ,Ȼ��Ĵ����������
							{
								if (reg[s].data.kind == aTemp)
									moveToArr(reg[s].data, s, codes);
								else//һ�����
									codes << "\tMOV WORD PTR " << SymbolTable[reg[s].data.addrPtr].name << ", " << reg[s].name << endl;
								//reg[s].data = Elem(DefaultW, -1);/**********************************************************************/
								//reg[s].L = Quarts[i].desL;
							}
							///ȡB��ֵ��s�Ĵ���
							if ((*optQ)[i].opl.kind == aTemp)
								moveToReg(s, (*optQ)[i].opl, codes);
							else//һ�����
								codes << "\tMOV " << reg[s].name << ", WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
							enroll(s, (*optQ)[i].des, (*optQ)[i].desL);
						}
					}
					else printf("not Int not Variable.\n");
					desCode.push_back(codes.str());
					codes.str("");
				}//=��������
				else if (op == ">" || op == "<" || op == "==" || op == "!=")
				{
					int flagSave = areSave((*optQ)[i].opl, N);
					if (flagSave == -1)//�������û���ڼĴ�����
					{
						if ((*optQ)[i].opl.kind == nT)//����
						{
							codes << "\tMOV BP, " << IntList[(*optQ)[i].opl.addrPtr] << endl;
						}
						else if (isVT((*optQ)[i].opl))//����
							codes << "\tMOV BP, WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
						else if ((*optQ)[i].opl.kind == aTemp)
							moveToReg(BP, (*optQ)[i].opl, codes);
						else { printf("opl not Int nor Variable.\n"); }
					}
					else//����������ڼĴ�����
					{
						codes << "\tMOV BP, " << reg[flagSave].name << endl;
					}
					///�Ҳ�����
					flagSave = areSave((*optQ)[i].opr, N);
					if (flagSave == -1)//�Ҳ��������ټĴ�����
					{
						if ((*optQ)[i].opr.kind == nT)//����
						{
							codes << "\tSUB BP, " << IntList[(*optQ)[i].opr.addrPtr] << endl;
						}
						else if (isVT((*optQ)[i].opr))//����
							codes << "\tSUB BP, WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
						else if ((*optQ)[i].opr.kind == aTemp)//�����ڴ��ַ
						{
							codes << "\tMOV DI, WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
							codes << "\tSUB BP, WORD PTR[DI]" << endl;
						}
						else { printf("opl not Int nor Variable.]\n"); }
					}
					else//�ڼĴ�����
					{
						codes << "\tSUB BP, " << reg[flagSave].name << endl;
					}
					codes << "\tMOV BP, 1" << endl;
					if (op == ">")//����
						codes << "\tJG SEG" << Seg << endl;
					else if (op == "<")//С��
						codes << "\tJL SEG" << Seg << endl;
					else if (op == "==")//������
						codes << "\tJZ SEG" << Seg << endl;
					else if (op == "!=")//����
						codes << "\tJNZ SEG" << Seg << endl;
					codes << "\tMOV BP, 0\n";
					codes << "  SEG" << Seg++ << ":" << endl;//����α��
					//codes << "\tMOV WORD PTR " << SymbolTable[Quarts[i].des.addrPtr].name << ", BP" << endl;///�Ƚϲ����Ļ�Ŀ��������ʱ����
					desCode.push_back(codes.str());
					enroll(BP, (*optQ)[i].des, (*optQ)[i].desL);//�Ǽ�BP��ֵ
					codes.str("");
				}
			}//�ǽ��
			else if ((*optQ)[i].op.kind == opK)
			{
				string op(OpList[(*optQ)[i].op.addrPtr]);
				if (op == "pf")//��ӡ����B��ֵ
				{//���һ��˫�ֽڵı���
					update(i);//���¼Ĵ����Ļ�Ծ��Ϣ
					stringstream codes;
					codes << "\tPUSH DX\n\tPUSH AX\n\tPUSH SI" << endl;//����Ĵ�����ֵ
					int rSave = areSave((*optQ)[i].opl, reg.size());
					if ((*optQ)[i].opl.kind == aTemp)//�����ڴ��ַ
					{
						if (rSave != -1 && rSave != SI)//��Ҫ�����ֵ��ͨ�üĴ�����
						{
							moveToArr(reg[rSave].data, rSave, codes);//ͨ��DI���Ĵ�����ֵ���������ڴ�
							codes << "\tMOV SI, DI\n";
						}//������SI�����账��
					}
					else//�������һ�����
					{
						if (rSave != -1 && reg[rSave].L > -2)//�ڼĴ������һ�Ծ
							codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << ", " << reg[rSave].name << endl;
						codes << "\tMOV SI, OFFSET " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
					}
					codes << "\tMOV DL, BYTE PTR[SI+1]" << endl;//����ֽڸ�λ
					codes << "\tADD DL,'0'" << endl;
					codes << "\tMOV AH,02H" << endl;
					codes << "\tINT 21H" << endl;
					codes << "\tMOV DL, BYTE PTR[SI]" << endl;//����ֽڸ�λ
					codes << "\tADD DL,'0'" << endl;
					codes << "\tMOV AH,02H" << endl;
					codes << "\tINT 21H" << endl;
					codes << "\tPOP SI\n\tPOP AX\n\tPOP DX" << endl;
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "if")
				{
					update(i);//���¼Ĵ����Ļ�Ծ��Ϣ
					jumpF(i);
				}
				else if (op == "el")
				{
					saveActive();//�����Ծ����
					stringstream numss;//��ȡ���ֵ��ַ���
					numss << Seg << endl;
					desCode[semStk.top()].append(numss.str());//������Ϣ
					semStk.pop();
					numss.str("");//���
					semStk.push(desCode.size());//JMP��תλ�ô�����
					codes << "\tJMP SEG";//���������Ϣ
					desCode.push_back(codes.str());
					codes.str("");
					codes << "  SEG" << Seg++ << ":" << endl;///���ɱ��
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "ie")//���������ifҲ������else����ת
				{
					stringstream numss;//��ȡ���ֵ��ַ���
					numss << Seg << endl;
					desCode[semStk.top()].append(numss.str());//������Ϣ
					semStk.pop();
					numss.str("");//���
					codes << "  SEG" << Seg++ << ":" << endl;///���ɱ��
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "wh")
				{
					codes << "  SEG" << Seg << ":" << endl;
					semStk.push(Seg++);//��¼�������Ϣ
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "do")
				{
					update(i);//���¼Ĵ����Ļ�Ծ��Ϣ
					jumpF(i);
				}
				else if (op == "we")
				{
					//��ǰ��ת��λ���ڴ�ջ����ջ��Ϊdo����ת��do��ת��we�ĺ��棬��ǰָ����������ת��wh
					saveActive();
					stringstream numss;//��ȡ���ֵ��ַ���
					numss << Seg << endl;
					desCode[semStk.top()].append(numss.str());//������Ϣ,��ת����ǰ���
					semStk.pop();
					codes << "\tJMP SEG" << semStk.top() << endl;
					semStk.pop();
					codes << "  SEG" << Seg++ << ":" << endl;//��ӵ�ǰ���
					numss.str("");//���
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "of")//����������ڴ��ַ��������of,arr,t0,i0��
				{///ȡ�����ַƫ������Ȼ����������ڴ��ַ,�����Ƿ���Ҫ����Ĵ���
					update(i);//���¼Ĵ����Ļ�Ծ��Ϣ
					codes << "\tMOV SI, OFFSET " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
					if ((*optQ)[i].opl.kind == nT)//����
						codes << "\tADD SI, " << IntList[(*optQ)[i].opr.addrPtr] << endl;
					else//��ʱ����
					{
						int rSave = areSave((*optQ)[i].opr, N);
						if (rSave != -1)//�Ҳ������ڼĴ�����
							codes << "\tADD SI, " << reg[rSave].name << endl;
						else
							codes << "\tADD SI, WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
					}
					//�Ǽ�Ŀ������SI��
					enroll(SI, (*optQ)[i].des, (*optQ)[i].desL);
					codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].des.addrPtr].name << ", SI" << endl;
					desCode.push_back(codes.str());
					codes.str("");
				}
			}
		}//for
		//����������������Ծ����
		saveActive();//ע�Ᵽ���Ծ��������Ϣ��ʱ������һ���ϸ��մ�˳��
		return true;
	}
	void showDesCode()
	{
		ofstream out;
		out.open("desCode.asm");
		if (out.is_open())
		{
			for (size_t i = 0; i < desCode.size(); i++)
			{
				printf("%s", desCode[i].c_str());
				out << desCode[i];
			}
			out.close();
		}
	}
};
bool isJump(const Elem &op)//�Ƿ�����תʽ��
{
	bool flag = false;
	if (op.kind != opK) return false;
	if (OpList[op.addrPtr] == "if" || OpList[op.addrPtr] == "do" || OpList[op.addrPtr] == "ie"
		|| OpList[op.addrPtr] == "el" || OpList[op.addrPtr] == "we")//��ת��Ԫʽ���
		flag = true;
	return flag;
}
bool isEntry(const Elem &op)
{
	bool flag = false;
	if (op.kind != opK) return false;
	if (OpList[op.addrPtr] == "wh")//�����Ԫʽ���
		flag = true;
	return flag;
}
bool GetBlockSeg(const vector<Quarter> &quad, vector<int> &seg)//����Ԫʽ����quad�Ļ��ֻ�����
{
	seg.push_back(0);//��Ԫʽ��ʼ
	for (size_t i = 1; i < quad.size(); i++)
	{
		if (isJump(quad[i].op))//��ת���
		{
			seg.push_back(i);//��ǰ��ת���λ��
			seg.push_back(i + 1);//��һ������λ��
		}
		else if (isEntry(quad[i].op))
		{
			seg.push_back(i - 1);//��һ�����λ��
			seg.push_back(i);//��ǰλ��
		}
	}
	seg.push_back(quad.size() - 1);//����
	return (seg.size() % 2 == 0);//segment�Ƿ��������
}

#  endif // !DESCODE_H