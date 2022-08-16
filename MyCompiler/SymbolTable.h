#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "Grammer.h"
#include <sstream>
enum EnumWord { nT, fT, cT, kT, pT, sT, iT, Temp, aTemp, Entry, opK, DefaultW };//单词种类内码
const string WordMap[] = { "nT","fT","cT", "kT","pT","sT","iT","Temp", "aT", "Entry","opK","DefaultW" };
enum EnumTYPE { INT, FLOAT, CHAR, BOOL, VOID, STRING, ARRAY, STRUCT, DefaultT };//类型，包括int,float,char,bool型
const string TypeMap[] = { "INT", "FLOAT", "CHAR", "BOOL", "VOID", "STRING", "ARRAY", "STRUCT" };
enum EnumCAT { FUNC, CONST, TYPE, DOMIN, VAR, VARN, VARF , TEMP, DEFAULTC};//种类
const string CatMap[] = { "FUNC","CONST","TYPE","DOMIN","VAR","VARN","VARF","TEMP", "DEFAULTC" };
enum EnumADDR { PFINFL, LENL, CONSL, VALL };//地址表，包括函数表，活动记录表
enum Register { AX, BX, CX, DX, BP, SI, DI };
vector <int> nTVec;//记录数组的初始化值
const int INIOff = 0;//变量初始化区距
int OffSet = INIOff;//main函数内，变量的内存偏移量
int Seg = 1;//程序段位置，便于跳转的编号
int PrmN;//参数个数
//界符表2
const vector<string> PList = { "+","-","*","/", "{","[","]","}","(",")",
							   ",",".","<",">", ":",";","&","|","!","\\",
							  "!=","=","%","&","==",">=","<=","&&","||","#" };

const vector<string> OpList = { "wh","do","we","if","el","ie" ,"rt","pf","sf","bg","ed","of" };
vector<int> IntList;//整型常数表3
vector<float> FloatList;//浮点常数表4

vector<char> CharList;//常字符表6
vector<string> StrList;//常字符串表7

/// *************各种表格的节点元素*************** ///
	//类型种类栈的节点
	struct Type
	{
		EnumTYPE type;
		int valNum;
		Type(EnumTYPE typ,int VU) :type(typ),valNum(VU) {}
	};

	//数组表节点
	struct AINFL
	{
		int low;//上界
		int high;//下界
		int typePtr;//成分类型指针
		int valNum;//值单元个数
		AINFL(int high, int ptr, int valNum) :
		high(high),typePtr(ptr),valNum(valNum){low = 0; }
	};

	//结构体表节点
	struct RINFL
	{
		string name;//域名
		int off;//域变量偏移值
		int typePtr;//成分类型指针
		RINFL(string name, int off, int typeptr) :
			name(name), off(off), typePtr(typeptr) {}
	};

	//类型表节点
	struct TYPEL
	{
		EnumTYPE typeName;//类型
		int listPtr;//指向结构表或数组表指针
		TYPEL(EnumTYPE TN,int LP) :typeName(TN),listPtr(LP) {}
	};

	//函数表节点
	struct PINFL
	{
		int level;//函数定义层次号
		int off;//数据域偏移量
		int paramNum;//参数个数
		int entry;//入口地址
		int paramPtr;//形参指针
		PINFL() {}
		PINFL(int paramNum,int entry,int paramPtr):
			paramNum(paramNum), entry(entry), paramPtr(paramPtr) {
			level = 0; off = 0;
		}
	};

	//标识符列表节点
	struct TableNode
	{
		string name;//标识符名
		int typePtr;//标识符类型指针
		EnumCAT cat;//标识符种类
		int addr;//地址项
		int activeL;//符号表的活跃属性扩展(-5表示不相关，-2表示不活跃n，-1表示活跃y，等于0表示为常数，大于0表示在相应的四元式处活跃)
		TableNode(string name) :name(name)
		{
			typePtr = addr = -1; cat = VAR;
			initActive();
		}
		TableNode(string name,int TP,EnumCAT CAT,int ADDR) 
			:name(name), typePtr(TP), cat(CAT), addr(ADDR) 
		{ 
			initActive();
		}
		bool operator==(const TableNode &right)const
		{
			return name == right.name;
		}//按名字匹配
		void initActive()
		{
			activeL = -5;//默认不相关,包括常数
			if (cat == VAR || cat == VARF || cat == VARN)//用户变量活跃
			{
				activeL = -1;
			}
			else if (cat == TEMP)//临时变量不活跃
				activeL = -2;
		}
	};

	struct Elem//四元式元素节点,与token类型相似
	{/*说明：Elem实际上是指向各种表的指针
	 当kind=iT、Temp时指向符号表
	 当kind=nT、fT、cT、sT时指向相应常量表
	 当kind=opK时，指向自定义操作符表OpList
	 总之根据kind的类型指向的表的类型不一样
	 **/
		EnumWord kind;//元素指针类型
		int addrPtr;//元素地址指针,指向符号表或常数表
		Elem() { kind = Temp; addrPtr = -1; }
		Elem(EnumWord ew, int ap) :kind(ew), addrPtr(ap) {}
		bool operator==(const Elem &right)const
		{
			return (kind == right.kind && addrPtr == right.addrPtr);
		}
		bool operator!=(const Elem &right)const
		{
			return !(*this == right);
		}
		bool operator<(const Elem &e)const
		{
			bool flag = false;
			if (kind < e.kind)
				flag = true;
			else if (kind == e.kind && addrPtr < e.addrPtr)
				flag = true;
			return flag;
		}
	};

	struct Quarter
	{
		Elem op;//元素1，界符指针
		Elem opl;
		Elem opr;
		Elem des;//元素4,目的操作数
		int leftL, rightL, desL;//做操作数，右操作数的活跃信息,-2不活跃，-1活跃，等于0是常数，大于为在四元式q处活跃
		Quarter() {}
		Quarter(Elem OP) :op(OP) { opl = opr = des = Elem(DefaultW, -1); initL(); }//只有运算位的四元式
		Quarter(Elem OP, Elem OPL, Elem OPR, Elem DES) :op(OP), opl(OPL), opr(OPR), des(DES) { initL(); }
	private:void initL() { leftL = rightL = desL = -5; }//初始化为不相关
	};

	/// **********与语义动作相关的栈********** ///
	stack<Elem> SEM;//语义栈
	map<int, EnumTYPE> tempMap, aTempMap;//临时变量在类型表上的地址映射
	vector<Elem> iTVec;//保存定义性标识符向量
	stack<Type> TypeStk;//类型和种类栈
	vector<Quarter> Quarts;//四元式区
	/// *****************表*********************** ///
	vector<TableNode> SymbolTable;//符号表表5

	vector<int> LengthList;//长度表
	vector<TYPEL> TypeList;//类型表
	vector<AINFL> ArrayList;//数组表
	vector<RINFL> StructList;//结构表
	vector<PINFL> FuncList;//函数表
	vector<TableNode> PrmList;//形参表
	//关键字表1
	const vector<string> KeyList = { "double","int","struct","float","void","break",
							"case","main","char","return","string","printf","if",
							"continue","do","const","long","switch","else",
							"for","default","while", "static", "typedef" };

	template<class T, class P>
	//只要重载了等于符号皆可以用此函数查找
	int FindList(T elem, const P &SomeList)//列表查找函数
	{
		int flag = SomeList.size() - 1;
		while (flag > -1 && !(SomeList[flag] == elem))
			flag--;
		return flag;
	}
	void ResetL()//将符号表的活跃信息复位
	{
		for (size_t i = 0; i < SymbolTable.size(); i++)
			SymbolTable[i].initActive();
	}
	EnumTYPE TypeOf(const Elem &elem)//翻译token单词元素类型
	{
		switch (elem.kind)
		{
		case iT://标识符
			if (SymbolTable[elem.addrPtr].typePtr == -1)
			{
				printf("\"%s\" is not declared!\n", SymbolTable[elem.addrPtr].name.c_str());
				exit(0);
			}
			return TypeList[SymbolTable[elem.addrPtr].typePtr].typeName;
		case Temp:
			return tempMap[elem.addrPtr];//第addrPtr个临时变量的类型
		case aTemp:
			return aTempMap[elem.addrPtr];
		case nT://整型常数
			return  INT;
		case fT://浮点常数
			return FLOAT;
		case cT://常数字符
			return CHAR;
		case sT://常字符串
			return STRING;
		}return DefaultT;
	}
	void FillTemp(const Elem & elem, EnumTYPE tp)
	{//将临时变量填表,tp是临时变量的类型也是其在类型表中的位置,对应符号表的listPtr项
		static int tempN = 0;//临时变量的数目
		static int idN = 0;//用于记录数组内存地址的变量
		if (tp > VOID) { printf("temp variable is invalid!\n"); exit(0); }
		stringstream ss;
		if (elem.kind == Temp)//临时变量的填写
		{
			tempMap[elem.addrPtr] = tp;//临时变量在符号表中的位置与类型的映射
			ss << "t" << tempN++;//数字字符转化
			SymbolTable.push_back(TableNode(ss.str(), tp, TEMP, OffSet));//临时变量加入符号表
		}
		else if (elem.kind == aTemp)//记录数组下标的临时变量
		{
			aTempMap[elem.addrPtr] = tp;//临时变量在符号表中的位置与类型的映射
			ss << "i" << idN++;
			SymbolTable.push_back(TableNode(ss.str(), tp, VAR, OffSet));//数组下标定义为临时变量
		}
		OffSet += TypeList[tp].listPtr;//修改偏移值
		ss.str("");
	}
	void ShowSymbolTable();
	void typeCmp(const Elem &opl, const Elem &opr)//操作数的类型比较
	{
		if (TypeOf(opl)!=TypeOf(opr))//操作数的类型检查
		{
			ShowSymbolTable();
			printf("[opl,%s,%d][opr,%s,%d]\n", WordMap[opl.kind].c_str(),opl.addrPtr,
				WordMap[opr.kind].c_str(), opr.addrPtr);
			printf("can't calculate two number with diffrent kind!\n");
			exit(0);
		}
	}
	//送入四元式
	void Send(Elem op)//暂时不支持两个不同类型的操作数参与运算
	{
		if (op.kind != opK && op.kind!=pT) //op.kind只能是pT或者opK 
		{
			printf("operator's kind error\n"); exit(0);
		}
		Elem opr = SEM.top();//栈顶为右操作数
		SEM.pop();
		typeCmp(SEM.top(), opr);//操作数的类型检查
		Elem tempData(Temp, SymbolTable.size());//申请临时变量
		//需要将临时变量加入符号表
		FillTemp(tempData,TypeOf(opr));//类型与右操作数相同
		Quarts.push_back(Quarter(op, SEM.top(), opr, tempData));
		SEM.pop();
		SEM.push(tempData);
	}
	void SendAQ()//产生数组的四元式
	{
		Elem aIT = iTVec.back();//栈顶元素为iT,该数组的标识符
		int i = SymbolTable[aIT.addrPtr].typePtr;//获取标识符类型指针
		int j;
		if (TypeList[i].typeName == ARRAY)
		{
			if (SEM.top().kind == nT)//SEM.top是Expr的索引，即判断下标类型
			{
				j = IntList[SEM.top().addrPtr];//常数值，仅对常数下标做越界检查
				if (!(j >= ArrayList[TypeList[i].listPtr].low 
					&& j <= ArrayList[TypeList[i].listPtr].high))//下标范围正常
				{//常数的话可在编译阶段直接根据类型找到偏移量
					printf("%d is out of range of array %s ",j, SymbolTable[aIT.addrPtr].name.c_str() );
					exit(0);
				}
			}
			j = FindList(4, IntList);//j为常数表的索引，这里4是int类型的值单元数
			if (j == -1)
			{
				j = IntList.size();
				IntList.push_back(4);
			}//让j指向常数表指针
			SEM.push(Elem(nT, j));
			//非常量类型无法在编译时刻做类型检查（temp或iT类型）
			Send(Elem(pT, FindList("*", PList)));//计算出内存地址
			///注意Send之后SEM.top()发生改变
			Elem aTemp(aTemp, SymbolTable.size());
			if (TypeList[SymbolTable[SEM.top().addrPtr].typePtr].typeName == INT)//下表是int类型？
				FillTemp(aTemp, INT);//数组下标默认是int类型
			else
			{
				printf("array's index must be Int type.\n");
				exit(0);
			}
			Quarts.push_back(Quarter(Elem(opK, FindList("of", OpList)), Elem(iTVec.back()), SEM.top(), aTemp));
			SEM.pop(); SEM.push(aTemp);//将数组标号压入语义栈
		}//标识符数组类型检查
		else 
		{
			printf("\"%s\" is not a array.\n", SymbolTable[aIT.addrPtr].name.c_str()); 
			exit(0);
		}
	}
	template<class ListType>
	//针对stringList输出
	void ShowWordList(const ListType &list, int size = 0)
	{
		for (int i = 0, j = 0; i < size; i++)//输出文法
		{
			j++;
			printf("[%3d,%-10s]", i, list.at(i).c_str());
			if (j == 5) { printf("\n"); j = 0; }
		}printf("\n");
	}
	//显示四元式序列
	void ShowQuarts(int s,int d, const vector<Quarter> &Quad)
	{
		for (int i = s; i <= d; i++)
		{/*
			printf("[%2d][(%5s,%2s)(%5s,%d)(%5s,%d)(%5s,%d)]\n", i,
				WordMap[Quarts.at(i).op.kind].c_str(), PList.at(Quarts.at(i).op.addrPtr).c_str(),
				WordMap[Quarts.at(i).opL.kind].c_str(), Quarts.at(i).opL.addrPtr,
				WordMap[Quarts.at(i).opR.kind].c_str(), Quarts.at(i).opR.addrPtr,
				WordMap[Quarts.at(i).des.kind].c_str(), Quarts.at(i).des.addrPtr);*/
			printf("[%2d][", i);
			///操作符字段
			if (Quad.at(i).op.kind == pT)printf("%-6s,", PList.at(Quad.at(i).op.addrPtr).c_str());
			else if(Quad.at(i).op.kind == opK) printf("%-6s,", OpList.at(Quad.at(i).op.addrPtr).c_str());
			else { printf("wrong operator!\n"); }
			///左操作字段
			if (Quad.at(i).opl.kind == iT)//iT
				printf("%-6s(%2d),", SymbolTable.at(Quad.at(i).opl.addrPtr).name.c_str()
					               , Quad.at(i).leftL);
			else if (Quad.at(i).opl.kind == nT)//nT
				printf("%-10d,", IntList.at(Quad.at(i).opl.addrPtr));
			else if (Quad.at(i).opl.kind == fT)//fT
				printf("%-10.2f,", FloatList.at(Quad.at(i).opl.addrPtr));
			else if (Quad.at(i).opl.kind == Temp || Quad.at(i).opl.kind == aTemp)//Temp
				printf("%-6s(%2d),", SymbolTable[Quad.at(i).opl.addrPtr].name.c_str()
					               , Quad.at(i).leftL);
			else if (Quad.at(i).opl.kind == cT)//cT
				printf("%-10c,", CharList.at(Quad.at(i).opl.addrPtr));
			else printf("%-10s,", "_");//Default
			///右操作字段
			if (Quad.at(i).opr.kind == iT)
				printf("%-6s(%2d),", SymbolTable.at(Quad.at(i).opr.addrPtr).name.c_str()
								   , Quad.at(i).rightL);//iT
			else if (Quad.at(i).opr.kind == nT)
				printf("%-10d,", IntList.at(Quad.at(i).opr.addrPtr));//nT
			else if (Quad.at(i).opr.kind == fT)
				printf("%-10.2f,", FloatList.at(Quad.at(i).opr.addrPtr));//fT
			else if (Quad.at(i).opr.kind == Temp || Quad.at(i).opr.kind == aTemp)//Temp
				printf("%-6s(%2d),", SymbolTable[Quad.at(i).opr.addrPtr].name.c_str()
								   , Quad.at(i).rightL);
			else if (Quad.at(i).opr.kind == cT)
				printf("%-10c,", CharList.at(Quad.at(i).opr.addrPtr));//cT
			else printf("%-10s,", "_");
			///目的字段
			if (Quad.at(i).des.kind == Temp||Quad.at(i).des.kind == iT || Quad.at(i).des.kind == aTemp)//Temp
				printf("%-6s(%2d)", SymbolTable[Quad.at(i).des.addrPtr].name.c_str()
								  , Quad.at(i).desL);//iT
			else if(Quad.at(i).des.kind == kT)
				printf("%-6s(%2d)", KeyList[Quad.at(i).des.addrPtr].c_str()
					, Quad.at(i).desL);//iT
			else
				printf("%-10s", "_");
			printf("]\n");
		}
	}
	//显示与符号表相关信息
	void ShowSymbolTable()
	{
		///符号表
		printf("+-----------------------------------------------+\n");
		printf("|                   符号表:%3d                  |\n", SymbolTable.size());
		printf("|-----------------------------------------------|\n");
		printf("| i  |     name   |typePtr |  cat | addr|activeL|\n");
		printf("+----+------------+--------+------+-----+-------+\n");
		for (int i = 0; i<int(SymbolTable.size()); i++)//符号表主表
		{
			printf("|%3d |%10s  | %5d  |%5s |%4d |%5d  |\n", i, 
				SymbolTable.at(i).name.c_str(),
				SymbolTable.at(i).typePtr,
				CatMap[SymbolTable.at(i).cat].c_str(), 
				SymbolTable.at(i).addr,
				SymbolTable.at(i).activeL);
			printf("+----+------------+--------+------+-----+-------+\n");
		
}
		///函数表
		printf("+-----------------------------------------+\n");
		printf("|                函数表:%3d               |\n", FuncList.size());
		printf("|-----------------------------------------|\n");
		printf("| i  |level |  off   | PrmN |entry|prmPtr |\n");
		printf("+----+------+--------+------+-----+-------+\n");
		for (int i = 0; i<int(FuncList.size()); i++)//符号表主表
		{
			printf("|%3d |%4d  | %5d  |%5d |%4d |%5d  |\n", i,
				FuncList[i].level,FuncList[i].off,FuncList[i].paramNum,
				FuncList[i].entry,FuncList[i].paramPtr);
			printf("+----+------+--------+------+-----+-------+\n");
		}
		///形参表
		printf("+-----------------------------------------------+\n");
		printf("|                   形参表:%3d                  |\n", PrmList.size());
		printf("|-----------------------------------------------|\n");
		printf("| i  |     name   |typePtr |  cat | addr|activeL|\n");
		printf("+----+------------+--------+------+-----+-------+\n");
		for (int i = 0; i<int(PrmList.size()); i++)//符号表主表
		{
			printf("|%3d |%10s  | %5d  |%5s |%4d |%5d  |\n", i,
				PrmList[i].name.c_str(),
				PrmList[i].typePtr,
				CatMap[PrmList[i].cat].c_str(),
				PrmList[i].addr,
				PrmList[i].activeL);
			printf("+----+------------+--------+------+-----+-------+\n");

		}
		///类型表
		printf("+---------------------+\n");
		printf("|      类型表:%3d     |\n", TypeList.size());
		printf("+---------------------+\n");
		printf("| i  |  Type  |listPtr|\n");
		printf("+----+--------+-------+\n");
		for (int i = 0; i<int(TypeList.size()); i++)//数组表
		{
			printf("|%3d | %6s |%5d  |\n", i, 
				TypeMap[TypeList[i].typeName].c_str(), TypeList[i].listPtr);
			printf("+----+--------+-------+\n");
		}printf("\n\n");
		///长度表
		printf("+--------------+\n");
		printf("|  长度表:%3d  |\n", LengthList.size());
		printf("+--------------+\n");
		printf("| i  |  value  |\n");
		printf("+----+---------+\n");
		for (int i = 0; i<int(LengthList.size()); i++)//数组表
		{
			printf("|%3d |  %5d  |\n", i, LengthList[i]);
			printf("+----+---------+\n");
		}printf("\n\n");
		///数组表
		printf("+-----------------------------+\n");
		printf("|           数组表:%3d        |\n", ArrayList.size());
		printf("+-----------------------------+\n");
		printf("| i  |low| high|typePtr | Unit|\n");
		printf("+----+---+-----+--------+-----+\n");
		for (int i = 0; i<int(ArrayList.size()); i++)//数组表
		{
			printf("|%3d | %d |%3d  | %5d  |%3d  |\n", i,
				ArrayList[i].low, ArrayList[i].high, 
				ArrayList[i].typePtr, ArrayList[i].valNum);
			printf("+----+---+-----+--------+-----+\n");
		}printf("\n\n");
		///结构表
		printf("+-------------------------------+\n");//符号表主表
		printf("|           结构表:%3d          |\n",StructList.size());
		printf("+-------------------------------+\n");
		printf("| i  |   name  |  off  |typePtr |\n");
		printf("+----+---------+-------+--------+\n");
		for (int i = 0; i<int(StructList.size()); i++)//符号表主表
		{
			printf("|%3d | %7s | %4d  | %5d  |\n", i,
				StructList[i].name.c_str(), StructList[i].off,
				StructList[i].typePtr);
			printf("+----+---------+-------+--------+\n");
		}printf("\n");
	}
	void ShowList()//显示各个符号表内容
	{
		printf("---------------------kT---------------------\n");
		ShowWordList(KeyList, KeyList.size());
		printf("---------------------pT---------------------\n");
		ShowWordList(PList, PList.size());
		printf("---------------------nT---------------------\n");
		for (int i = 0; i<int(IntList.size()); i++)
			printf("[%-3d,%d]  ", i, IntList.at(i));
		printf("\n");

		printf("---------------------fT---------------------\n");
		for (int i = 0; i<int(FloatList.size()); i++)
			printf("[%-3d,%f]\t", i, FloatList.at(i));
		printf("\n");

		printf("---------------------cT---------------------\n");
		for (int i = 0; i<int(CharList.size()); i++)
			printf("[%-3d,%c]", i, CharList.at(i));

		printf("\n");
		printf("---------------------sT---------------------\n");
		ShowWordList(StrList, StrList.size());
	}
	void ShowQuadBlock(const vector<int> &seg, const vector<Quarter> &quad)
	{
		for (size_t i = 0; i < seg.size(); i += 2)//显示源四元式基本信息
		{
			ShowQuarts(seg[i], seg[i + 1], quad);
			printf("\n");
		}
	}
#endif // !SYMBOLTABLE_H