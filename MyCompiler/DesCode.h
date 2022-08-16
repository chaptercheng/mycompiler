#ifndef DESCODE_H
#define DESCODE_H
#include "SymbolTable.h"
/*本目标代码的生成是基于8086指令系统的
**为了简化程序设计，本目标代码的生成只
**支持对整型变量进行算数运算，且为不超过
**两个字节的带符号数
*/
struct RDL//寄存器描述项
{
	Elem data;//寄存器保存的哪个数据
	string name;//寄存器的名字
	int L;//寄存器当前所保存的变量活跃信息记录
	RDL(Elem E, string Name) :data(E), name(Name) { L = -2; }
};

class DesCoder//目标代码生成器
{
public:
	vector<string> desCode;
	vector<RDL> reg;//寄存器信息
	stack<int> semStk;//返现信息语义栈
	const int N = 4;//通用寄存器数目
	vector<Quarter> *optQ;//对优化后的四元式生成目标代码
	DesCoder(vector<Quarter> *optq) :optQ(optq) {
		reg.push_back(RDL(Elem(DefaultW, -1), "AX"));
		reg.push_back(RDL(Elem(DefaultW, -1), "BX"));
		reg.push_back(RDL(Elem(DefaultW, -1), "CX"));
		reg.push_back(RDL(Elem(DefaultW, -1), "DX"));
		reg.push_back(RDL(Elem(DefaultW, -1), "BP"));//非通用寄存器
		reg.push_back(RDL(Elem(DefaultW, -1), "SI"));//非通用寄存器
		reg.push_back(RDL(Elem(DefaultW, -1), "DI"));//非通用寄存器
	}
	bool isVT(const Elem &e)//是变量或临时单元（填写活跃信息时的类型判断）
	{
		if (e.kind != iT && e.kind != Temp)//
			return false;
		return (SymbolTable[e.addrPtr].cat == VAR || SymbolTable[e.addrPtr].cat == VARF
			|| SymbolTable[e.addrPtr].cat == VARN || SymbolTable[e.addrPtr].cat == TEMP);
	}
	void update(unsigned i)//根据第i个四元式更新寄存器活跃信息描述项
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
	void enroll(int r, const Elem &e, int L)//第q个四元式的某项的信息登记在寄存器r中
	{
		reg[r].data = e;
		reg[r].L = L;
	}
	int selectR()//寄存器选择,先选择负数（不活跃的）
	{
		int maxx = reg[0].L;
		int s = 0;
		for (int i = 0; i < N; i++)
		{
			if (reg[i].L < -1)//空或不活跃
			{
				s = i; break;
			}
			else if (reg[i].L == -1)//活跃
			{
				maxx = SIZE_MAX;//最远处理
			}
			else if (maxx < reg[i].L)//按距离远近选择
			{
				maxx = reg[i].L;
				s = i;
			}
		}
		return s;
	}
	int areSave(const Elem &e, int n)//返回第几个通用寄存器保存有数据e的值，没有返回-1
	{
		int flag = n - 1;
		while (flag > -1 && reg[flag].data != e)
			flag--;
		return flag;
	}
	int areIdle()//是否有空闲寄存器
	{
		int flag = N - 1;//非通用寄存器不参与到空闲寄存器的分配上来
		while (flag > -1 && reg[flag].data.addrPtr != -1)//从后往前找空寄存器
			flag--;
		return flag;
	}

	//以基本块为单位进行活跃信息的标记
	void actives(int s, int d)
	{
		for (int i = d; i >= s; i--)//逆序扫描各基本块内四元式
		{
			if (isVT((*optQ)[i].des) || (*optQ)[i].des.kind == aTemp)//目标数是变量
			{
				(*optQ)[i].desL = SymbolTable[(*optQ)[i].des.addrPtr].activeL;
				SymbolTable[(*optQ)[i].des.addrPtr].activeL = -2;//n
			}
			if (isVT((*optQ)[i].opr) || (*optQ)[i].opr.kind == aTemp)//右操作数是变量
			{
				(*optQ)[i].rightL = SymbolTable[(*optQ)[i].opr.addrPtr].activeL;
				SymbolTable[(*optQ)[i].opr.addrPtr].activeL = i;//用到的位置
			}
			else if ((*optQ)[i].opl.kind == nT) (*optQ)[i].leftL = 0;
			if (isVT((*optQ)[i].opl) || (*optQ)[i].opl.kind == aTemp)//左操作数是变量
			{
				(*optQ)[i].leftL = SymbolTable[(*optQ)[i].opl.addrPtr].activeL;
				SymbolTable[(*optQ)[i].opl.addrPtr].activeL = i;//用到的位置
			}
			else if ((*optQ)[i].opr.kind == nT) (*optQ)[i].rightL = 0;
			if ((*optQ)[i].op.kind == opK && OpList[(*optQ)[i].op.addrPtr] == "of")//对计算数组内存地址的四元式特殊处理
			{
				(*optQ)[i].leftL = -2;//左操作数活跃信息
				if ((*optQ)[i].desL < -1)
					(*optQ)[i].desL = -1;//目标数活跃信息
			}
		}ResetL();
	}
	void labelL(const vector<int> &seg)//为每个四元式标注活跃信息
	{
		for (size_t i = 0; i < seg.size(); i += 2)
		{
			actives(seg[i], seg[i + 1]);
		}
	}
	void beginDC(const vector<int> &seg)//目标代码的定义部分的
	{
		desCode.push_back("DATAS SEGMENT\n");
		for (size_t i = 0; i < SymbolTable.size(); i++)//对每一个变量定义DUP
		{
			stringstream codes;
			int unit = 0;
			if (SymbolTable[i].cat == VAR || SymbolTable[i].cat == VARF
				|| SymbolTable[i].cat == VARN || SymbolTable[i].cat == TEMP)//只对变量进行分配数据
			{
				if (TypeList[SymbolTable[i].typePtr].typeName <= VOID)//基本类型
				{
					unit = TypeList[SymbolTable[i].typePtr].listPtr;
					codes << "\t" << SymbolTable[i].name << " DB " << unit << " DUP(0)\n";//数据定义
				}
				else if (TypeList[SymbolTable[i].typePtr].typeName == ARRAY)//数组类型
				{
					int p = TypeList[SymbolTable[i].typePtr].listPtr;//获取指向数组表的指针
					unit = ArrayList[p].valNum;
					if (TypeList[ArrayList[p].typePtr].typeName < VOID && !nTVec.empty())//是1维数组且被初始化了？
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
						else //定义的个数不对，则失效
						{
							printf("your array's initial is invalid due to the number is not correct.\n");
							codes << "\t" << SymbolTable[i].name << " DB " << unit << " DUP(0)\n";//数据定义
						}
					}
					else
					{
						codes << "\t" << SymbolTable[i].name << " DB " << unit << " DUP(0)\n";//数据定义
					}
				}
				desCode.push_back(codes.str());//添加到代码区
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
		desCode.push_back("\tXOR DX, DX\n");//初始化部分
		///目标代码部分
		for (size_t i = 0; i < seg.size(); i += 2)
		{
			genDesCode(seg[i], seg[i + 1]);
		}
		desCode.push_back("\tMOV AH, 4CH\n");
		desCode.push_back("\tINT 21H\n");
		desCode.push_back("CODES ENDS\n");
		desCode.push_back("\tEND START\n");
	}
	void jumpF(int i)//用于if与do的四元式跳转
	{
		stringstream codes;
		saveActive();//保存活跃变量
		if ((*optQ)[i].opl.kind == nT)//常数
		{
			codes << "\tMOV BP, " << IntList[(*optQ)[i].opl.addrPtr] << endl;
		}
		else if (isVT((*optQ)[i].opl))//变量
		{
			if (reg[BP].data.kind == iT)//opl的值不在BP寄存器
			{
				codes << "\tMOV BP, " << "WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
			}
		}
		else { printf("not Int nor Variable.\n"); }
		codes << "\tOR BP, 0\n";//测试标志位
		desCode.push_back(codes.str());
		codes.str("");
		semStk.push(desCode.size());//待返填项压栈
		codes << "\tJZ SEG";//JF跳转，待回填
		desCode.push_back(codes.str());
		enroll(BP, Elem(DefaultW, -1), -2);//BP寄存器清空
		codes.str("");//清空
	}
	void saveActive()//基本块结束保存活跃变量
	{
		stringstream codes;
		for (int i = 0; i < (int)reg.size() - 1; i++)
		{
			if (reg[i].data.addrPtr > -1)
			{
				if (reg[i].L > -2 && isVT(reg[i].data))//活跃
					codes << "\tMOV WORD PTR " << SymbolTable[reg[i].data.addrPtr].name << ", " << reg[i].name << endl;
				else if (reg[i].L > -2 && reg[i].data.kind == aTemp)
				{
					moveToArr(reg[i].data, i, codes);
				}
				reg[i].data = Elem(DefaultW, -1);//清空寄存器
				reg[i].L = -2;
			}
		}
		desCode.push_back(codes.str());
		codes.str("");
		///保存完活跃变量后寄存器清空
	}
	void haveB(unsigned i, int bSave)//第i个四元式左操作数保存在flaSave指出的寄存器，即主动释放
	{
		stringstream codes;
		if ((*optQ)[i].leftL > -2)//B 活跃
		{
			int flagIdle = areIdle();
			if (flagIdle != -1)//有空闲寄存器
			{
				if (isVT((*optQ)[i].opl) || (*optQ)[i].opl.kind == aTemp)//变量
				{
					codes << "\tMOV " << reg[flagIdle].name << ", " << reg[bSave].name << endl;
					enroll(flagIdle, reg[bSave].data, reg[bSave].L);//B登记到另一寄存器
				}
				else { printf("not is a variable!\n"); }
			}
			else//没有空闲寄存器，将B保存在内存中
			{
				if (isVT((*optQ)[i].opl))//是变量,且活跃，保存到M
					codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << "," << reg[bSave].name << endl;
				else if ((*optQ)[i].opl.kind == aTemp)
					moveToArr((*optQ)[i].opl, bSave, codes);
			}
		}
		// B 不活跃,直接将B的值覆盖
		if ((*optQ)[i].op.kind == pT)
		{
			if (PList[(*optQ)[i].op.addrPtr] == "+" || PList[(*optQ)[i].op.addrPtr] == "-")//+运算
			{
				string opStr = "\tADD ";
				if (PList[(*optQ)[i].op.addrPtr] == "-") opStr = "\tSUB ";
				if ((*optQ)[i].opr.kind == nT)//右操作数常数
					codes << opStr << reg[bSave].name << "," << IntList[(*optQ)[i].opr.addrPtr] << endl;
				else if (isVT((*optQ)[i].opr))//变量
				{
					int rSave = areSave((*optQ)[i].opr, N);
					if (rSave == -1)//没有寄存器保存C
						codes << opStr << reg[bSave].name << ", WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
					else
						codes << opStr << reg[bSave].name << ", " << reg[rSave].name << endl;
				}
				else if ((*optQ)[i].opr.kind == aTemp)//右操作数为数组内存地址
				{
					int rSave = areSave((*optQ)[i].opr, reg.size());
					if (rSave == -1)//没有寄存器保存C
					{
						codes << "\tMOV DI" << ", WORD PTR" << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
						codes << opStr << reg[bSave].name << ", [DI]" << endl;
					}
					else if (rSave == SI)
						codes << opStr << reg[bSave].name << ", [SI]" << endl;
					else//通用寄存器
						codes << opStr << reg[bSave].name << ", " << reg[rSave].name << endl;
				}
				else { printf("not Int nor Variable\n"); }
				enroll(bSave, (*optQ)[i].des, (*optQ)[i].desL);//第q个四元式的des信息登记在寄存器r中
			}
			else if (PList[(*optQ)[i].op.addrPtr] == "*" || PList[(*optQ)[i].op.addrPtr] == "/")
			{
				//需要清空DX，覆盖AX，提前保存他们的值
				if (reg[DX].data.addrPtr != -1)//DX不空
				{
					if (reg[DX].data.kind == aTemp)
						moveToArr(reg[DX].data, DX, codes);
					else
						codes << "\tMOV WORD PTR " << SymbolTable[reg[DX].data.addrPtr].name << ", " << reg[DX].name << endl;
					enroll(DX, Elem(DefaultW, -1), -2);
				}
				if (reg[AX].data.addrPtr != -1 && bSave != 0)//AX不空，且flagSave不是AX的值
				{//保存AX的值，将左操作数送AX
					if (reg[AX].data.kind == aTemp)
						moveToArr(reg[AX].data, AX, codes);
					else //一般变量
						codes << "\tMOV WORD PTR " << SymbolTable[reg[AX].data.addrPtr].name << ", " << reg[AX].name << endl;
					codes << "\tMOV AX," << reg[bSave].name << endl;
				}
				int rSave = areSave((*optQ)[i].opr, reg.size());
				if (rSave == -1)//没有寄存器保存C
				{
					if ((*optQ)[i].opr.kind == nT)//常数
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
					else if ((*optQ)[i].opr.kind == aTemp)//数组内存地址
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
				else//C已在rSave保存的寄存器中
				{
					codes << "\tXOR DX, DX\n";
					if (PList[(*optQ)[i].op.addrPtr] == "/")
						codes << "\tDIV " << reg[rSave].name << endl;
					else codes << "\tMUL " << reg[rSave].name << endl;
				}///注意为简化起见，运算结果不对DX检验输出
				enroll(AX, (*optQ)[i].des, (*optQ)[i].desL);//登记运算结果在AX
			}
		}
		else { printf("not + -  * /\n"); };
		desCode.push_back(codes.str());//放入代码区
		codes.str("");//代码流清空，释放内存
	}//haveB
	void forceB(unsigned i, int r)//通过强迫释放让B的值在r指向的寄存器中
	{
		stringstream codes;
		if (PList[(*optQ)[i].op.addrPtr] == "+" || PList[(*optQ)[i].op.addrPtr] == "-")//+运算
		{
			string opStr = "\tADD ";//取操作指令
			if (PList[(*optQ)[i].op.addrPtr] == "-")
				opStr = "\tSUB ";
			int rSave = areSave((*optQ)[i].opr, reg.size());//寄存器组中寻找右操作数
			if ((*optQ)[i].opr.kind == nT)//右操作数常数
				codes << opStr << reg[r].name << "," << IntList[(*optQ)[i].opr.addrPtr] << endl;
			else if (isVT((*optQ)[i].opr))//变量
			{
				if (rSave == -1)//没有寄存器保存C
					codes << opStr << reg[r].name << ", WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
				else
					codes << opStr << reg[r].name << ", " << reg[rSave].name << endl;
			}
			else if ((*optQ)[i].opr.kind == aTemp)//数组内存地址
			{
				if (rSave != -1 && rSave != SI)//在通用寄存器中保存C
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
			enroll(r, (*optQ)[i].des, (*optQ)[i].desL);//第i个四元式的des信息登记在寄存器r中
		}
		else if (PList[(*optQ)[i].op.addrPtr] == "*" || PList[(*optQ)[i].op.addrPtr] == "/")
		{
			//需要提前清空DX，AX
			if (reg[DX].data.addrPtr != -1)//DX不空
			{
				if (reg[DX].data.kind == aTemp)
					moveToArr(reg[DX].data, DX, codes);
				else
					codes << "\tMOV WORD PTR " << SymbolTable[reg[DX].data.addrPtr].name << ", " << reg[DX].name << endl;
				enroll(DX, Elem(DefaultW, -1), -2);
			}
			if (reg[AX].data.addrPtr != -1 && r != AX)//AX不空，或则r不是AX的值
			{//保存AX的值，将左操作数送AX
				if (reg[AX].data.kind == aTemp)
					moveToArr(reg[AX].data, AX, codes);
				else//一般变量
					codes << "\tMOV WORD PTR " << SymbolTable[reg[AX].data.addrPtr].name << ", " << reg[AX].name << endl;
				codes << "\tMOV AX," << reg[r].name << endl;
			}
			int rSave = areSave((*optQ)[i].opr, N);
			if (rSave == -1)//没有寄存器保存C
			{
				if ((*optQ)[i].opr.kind == nT)//常数
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
			else//C已在rSave保存的寄存器中
			{
				codes << "\tXOR DX, DX\n";
				if (PList[(*optQ)[i].op.addrPtr] == "/")
					codes << "\tDIV " << reg[rSave].name << endl;
				else codes << "\tMUL " << reg[rSave].name << endl;
			}///注意为简化起见，运算结果不对DX检验输出
			enroll(AX, (*optQ)[i].des, (*optQ)[i].desL);//登记运算结果在AX
		}
		desCode.push_back(codes.str());//放入代码区
		codes.str("");//代码流清空，释放内存
	}//forceB

	void moveToReg(int r, const Elem &arr, stringstream &codes)//通用寄存器值<-数组元素
	{
		//先要将索引的下标值放在DI中
		codes << "\tMOV DI, WORD PTR " << SymbolTable[arr.addrPtr].name << endl;
		codes << "\tMOV " << reg[r].name << ", [DI]" << endl;
	}
	void moveToArr(const Elem &arr, int r, stringstream &codes)//数组元素<-通用寄存器值
	{

		if (areSave(arr, reg.size()) == SI && r != SI)//下标存放在SI中
			codes << "\tMOV [SI]," << reg[r].name << endl;
		else if (r != SI)//将索引的下标值放在DI中,存入数据
		{
			codes << "\tMOV DI, WORD PTR " << SymbolTable[arr.addrPtr].name << endl;
			codes << "\tMOV [DI]," << reg[r].name << endl;
		}

	}
	bool genDesCode(int s, int d)//以基本块为单位生成目标代码
	{
		for (int i = s; i <= d; i++)
		{
			stringstream codes;
			if ((*optQ)[i].op.kind == pT)//是界符（+ - * / > < ==）
			{
				string op(PList[(*optQ)[i].op.addrPtr]);
				//q:(w,B,C,A)
				update(i);//更新寄存器的活跃信息
				if (op == "+" || op == "-" || op == "*" || op == "/" )//算术运算,比较运算
				{
					int flagSave = areSave((*optQ)[i].opl, N);
					if (flagSave != -1)//有寄存器保存着B
					{
						haveB(i, flagSave);
					}
					else//没有寄存器保存B（B可能是变量或常数）抢夺
					{
						int s = selectR();
						if (reg[s].data.addrPtr > -1)//有数据，保存原有寄存器的值,然后寄存器内容清空
						{
							//判断数据是一般变量还是数组的内存地址
							if (reg[s].data.kind == aTemp)
								moveToArr(reg[s].data, s, codes);
							else if (reg[s].L > -2 || reg[s].data == (*optQ)[i].opr)//一般变量活跃或者是右操作数
							{
								codes << "\tMOV WORD PTR " << SymbolTable[reg[s].data.addrPtr].name << ", " << reg[s].name << endl;
								enroll(s, (*optQ)[i].des, (*optQ)[i].desL);
							}
						}
						///下面将B 值放入抢夺到的寄存器中
						if ((*optQ)[i].opl.kind == nT)//常数
						{
							codes << "\tMOV " << reg[s].name << ", " << IntList[(*optQ)[i].opl.addrPtr] << endl;
						}
						else if (isVT((*optQ)[i].opl)) //变量
						{
							codes << "\tMOV " << reg[s].name << ", WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
						}
						else if ((*optQ)[i].opl.kind == aTemp)//数组的内存地址
						{
							moveToReg(s, (*optQ)[i].opl, codes);
						}
						else { printf("not is a Int nor Variable\n"); }
						desCode.push_back(codes.str());
						codes.str("");//代码流清空
						forceB(i, s);//B在s指向的寄存器
					}
				}//算术运算
				else if (op == "=")//q:(=,B,_,A)赋值运算
				{
					if ((*optQ)[i].opl.kind == nT)//常数赋值给变量
					{
						if ((*optQ)[i].des.kind == aTemp)
						{
							if (areSave((*optQ)[i].des, reg.size()) != SI)//数组地址已在SI寄存器
							{
								codes << "\tMOV SI , WORD PTR " << SymbolTable[(*optQ)[i].des.addrPtr].name << endl;
								enroll(SI, (*optQ)[i].des, (*optQ)[i].desL);
							}
							codes << "\tMOV WORD PTR[SI], " << IntList[(*optQ)[i].opl.addrPtr] << endl;
						}
						else//一般变量
							codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].des.addrPtr].name << ", "
							<< IntList[(*optQ)[i].opl.addrPtr] << endl;
					}
					else if (isVT((*optQ)[i].opl) || (*optQ)[i].opl.kind == aTemp)//变量
					{
						int flagSave = areSave((*optQ)[i].opl, N);//查找寄存器有无B值
						if (flagSave != -1)//B 保存在寄存器中
						{
							if ((*optQ)[i].leftL > -2)//B活跃,保存
							{
								if ((*optQ)[i].opl.kind == aTemp && flagSave != SI)
								{
									moveToArr((*optQ)[i].opl, flagSave, codes);
								}
								else//一般变量
									codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << ", " << reg[flagSave].name << endl;
							}
							//B不活跃，直接覆盖
							enroll(flagSave, (*optQ)[i].des, (*optQ)[i].desL);
						}
						else //B=0不寄存器中
						{
							int s = selectR();//抢夺一个寄存器
							if (reg[s].data.addrPtr > -1)//有数据，保存原有寄存器的值,然后寄存器内容清空
							{
								if (reg[s].data.kind == aTemp)
									moveToArr(reg[s].data, s, codes);
								else//一般变量
									codes << "\tMOV WORD PTR " << SymbolTable[reg[s].data.addrPtr].name << ", " << reg[s].name << endl;
								//reg[s].data = Elem(DefaultW, -1);/**********************************************************************/
								//reg[s].L = Quarts[i].desL;
							}
							///取B的值到s寄存器
							if ((*optQ)[i].opl.kind == aTemp)
								moveToReg(s, (*optQ)[i].opl, codes);
							else//一般变量
								codes << "\tMOV " << reg[s].name << ", WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
							enroll(s, (*optQ)[i].des, (*optQ)[i].desL);
						}
					}
					else printf("not Int not Variable.\n");
					desCode.push_back(codes.str());
					codes.str("");
				}//=等于运算
				else if (op == ">" || op == "<" || op == "==" || op == "!=")
				{
					int flagSave = areSave((*optQ)[i].opl, N);
					if (flagSave == -1)//左操作数没有在寄存器中
					{
						if ((*optQ)[i].opl.kind == nT)//常数
						{
							codes << "\tMOV BP, " << IntList[(*optQ)[i].opl.addrPtr] << endl;
						}
						else if (isVT((*optQ)[i].opl))//变量
							codes << "\tMOV BP, WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
						else if ((*optQ)[i].opl.kind == aTemp)
							moveToReg(BP, (*optQ)[i].opl, codes);
						else { printf("opl not Int nor Variable.\n"); }
					}
					else//左操作数已在寄存器中
					{
						codes << "\tMOV BP, " << reg[flagSave].name << endl;
					}
					///右操作数
					flagSave = areSave((*optQ)[i].opr, N);
					if (flagSave == -1)//右操作数不再寄存器中
					{
						if ((*optQ)[i].opr.kind == nT)//常数
						{
							codes << "\tSUB BP, " << IntList[(*optQ)[i].opr.addrPtr] << endl;
						}
						else if (isVT((*optQ)[i].opr))//变量
							codes << "\tSUB BP, WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
						else if ((*optQ)[i].opr.kind == aTemp)//数组内存地址
						{
							codes << "\tMOV DI, WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
							codes << "\tSUB BP, WORD PTR[DI]" << endl;
						}
						else { printf("opl not Int nor Variable.]\n"); }
					}
					else//在寄存器中
					{
						codes << "\tSUB BP, " << reg[flagSave].name << endl;
					}
					codes << "\tMOV BP, 1" << endl;
					if (op == ">")//大于
						codes << "\tJG SEG" << Seg << endl;
					else if (op == "<")//小于
						codes << "\tJL SEG" << Seg << endl;
					else if (op == "==")//不等于
						codes << "\tJZ SEG" << Seg << endl;
					else if (op == "!=")//等于
						codes << "\tJNZ SEG" << Seg << endl;
					codes << "\tMOV BP, 0\n";
					codes << "  SEG" << Seg++ << ":" << endl;//程序段标号
					//codes << "\tMOV WORD PTR " << SymbolTable[Quarts[i].des.addrPtr].name << ", BP" << endl;///比较操作的话目标数是临时变量
					desCode.push_back(codes.str());
					enroll(BP, (*optQ)[i].des, (*optQ)[i].desL);//登记BP的值
					codes.str("");
				}
			}//是界符
			else if ((*optQ)[i].op.kind == opK)
			{
				string op(OpList[(*optQ)[i].op.addrPtr]);
				if (op == "pf")//打印变量B的值
				{//输出一个双字节的变量
					update(i);//更新寄存器的活跃信息
					stringstream codes;
					codes << "\tPUSH DX\n\tPUSH AX\n\tPUSH SI" << endl;//保存寄存器的值
					int rSave = areSave((*optQ)[i].opl, reg.size());
					if ((*optQ)[i].opl.kind == aTemp)//数组内存地址
					{
						if (rSave != -1 && rSave != SI)//若要输出的值在通用寄存器中
						{
							moveToArr(reg[rSave].data, rSave, codes);//通过DI将寄存器的值送入数组内存
							codes << "\tMOV SI, DI\n";
						}//若已在SI中无需处理
					}
					else//待输出是一般变量
					{
						if (rSave != -1 && reg[rSave].L > -2)//在寄存其中且活跃
							codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].opl.addrPtr].name << ", " << reg[rSave].name << endl;
						codes << "\tMOV SI, OFFSET " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
					}
					codes << "\tMOV DL, BYTE PTR[SI+1]" << endl;//输出字节高位
					codes << "\tADD DL,'0'" << endl;
					codes << "\tMOV AH,02H" << endl;
					codes << "\tINT 21H" << endl;
					codes << "\tMOV DL, BYTE PTR[SI]" << endl;//输出字节高位
					codes << "\tADD DL,'0'" << endl;
					codes << "\tMOV AH,02H" << endl;
					codes << "\tINT 21H" << endl;
					codes << "\tPOP SI\n\tPOP AX\n\tPOP DX" << endl;
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "if")
				{
					update(i);//更新寄存器的活跃信息
					jumpF(i);
				}
				else if (op == "el")
				{
					saveActive();//保存活跃变量
					stringstream numss;//读取数字的字符流
					numss << Seg << endl;
					desCode[semStk.top()].append(numss.str());//返填信息
					semStk.pop();
					numss.str("");//清空
					semStk.push(desCode.size());//JMP跳转位置待返填
					codes << "\tJMP SEG";//待返填的信息
					desCode.push_back(codes.str());
					codes.str("");
					codes << "  SEG" << Seg++ << ":" << endl;///生成标号
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "ie")//返填，可能是if也可能是else的跳转
				{
					stringstream numss;//读取数字的字符流
					numss << Seg << endl;
					desCode[semStk.top()].append(numss.str());//返填信息
					semStk.pop();
					numss.str("");//清空
					codes << "  SEG" << Seg++ << ":" << endl;///生成标号
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "wh")
				{
					codes << "  SEG" << Seg << ":" << endl;
					semStk.push(Seg++);//记录回填的信息
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "do")
				{
					update(i);//更新寄存器的活跃信息
					jumpF(i);
				}
				else if (op == "we")
				{
					//当前跳转的位置在次栈顶，栈顶为do的跳转，do跳转到we的后面，当前指令无条件跳转至wh
					saveActive();
					stringstream numss;//读取数字的字符流
					numss << Seg << endl;
					desCode[semStk.top()].append(numss.str());//返填信息,跳转到当前标号
					semStk.pop();
					codes << "\tJMP SEG" << semStk.top() << endl;
					semStk.pop();
					codes << "  SEG" << Seg++ << ":" << endl;//添加当前编号
					numss.str("");//清空
					desCode.push_back(codes.str());
					codes.str("");
				}
				else if (op == "of")//计算数组的内存地址，例：（of,arr,t0,i0）
				{///取数组地址偏移量，然后计算数组内存地址,考虑是否需要抢夺寄存器
					update(i);//更新寄存器的活跃信息
					codes << "\tMOV SI, OFFSET " << SymbolTable[(*optQ)[i].opl.addrPtr].name << endl;
					if ((*optQ)[i].opl.kind == nT)//常数
						codes << "\tADD SI, " << IntList[(*optQ)[i].opr.addrPtr] << endl;
					else//临时变量
					{
						int rSave = areSave((*optQ)[i].opr, N);
						if (rSave != -1)//右操作数在寄存器中
							codes << "\tADD SI, " << reg[rSave].name << endl;
						else
							codes << "\tADD SI, WORD PTR " << SymbolTable[(*optQ)[i].opr.addrPtr].name << endl;
					}
					//登记目的数在SI中
					enroll(SI, (*optQ)[i].des, (*optQ)[i].desL);
					codes << "\tMOV WORD PTR " << SymbolTable[(*optQ)[i].des.addrPtr].name << ", SI" << endl;
					desCode.push_back(codes.str());
					codes.str("");
				}
			}
		}//for
		//基本块结束，保存活跃变量
		saveActive();//注意保存活跃变量的信息的时机，不一定严格按照此顺序
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
bool isJump(const Elem &op)//是否是跳转式子
{
	bool flag = false;
	if (op.kind != opK) return false;
	if (OpList[op.addrPtr] == "if" || OpList[op.addrPtr] == "do" || OpList[op.addrPtr] == "ie"
		|| OpList[op.addrPtr] == "el" || OpList[op.addrPtr] == "we")//跳转四元式语句
		flag = true;
	return flag;
}
bool isEntry(const Elem &op)
{
	bool flag = false;
	if (op.kind != opK) return false;
	if (OpList[op.addrPtr] == "wh")//入口四元式语句
		flag = true;
	return flag;
}
bool GetBlockSeg(const vector<Quarter> &quad, vector<int> &seg)//对四元式序列quad的划分基本块
{
	seg.push_back(0);//四元式开始
	for (size_t i = 1; i < quad.size(); i++)
	{
		if (isJump(quad[i].op))//跳转语句
		{
			seg.push_back(i);//当前跳转语句位置
			seg.push_back(i + 1);//下一条语句的位置
		}
		else if (isEntry(quad[i].op))
		{
			seg.push_back(i - 1);//上一条语句位置
			seg.push_back(i);//当前位置
		}
	}
	seg.push_back(quad.size() - 1);//结束
	return (seg.size() % 2 == 0);//segment是否两两配对
}

#  endif // !DESCODE_H