#ifndef SLRANALYZER_H
#define SLRANALYZER_H
#include "ListGenerator.h"
/**
*@author 程章
*LR(0) 分析表需要自己设计写入lrChart文件中
*根据分析方法设立两个站stateStk,alphaStk;
*在LR分析表中若为0表示空，大于1000则为规约状态，
*并对1000取余后为规约是对应的文法表达式
*注意文法的读取要求，文法的分析表中指出的规约项目
*应当是按照文法定义的出现顺序编号
*/

class SLRAnalyzer
{
public:
	Scanner *scan;//词法分析器
	ListGenerator *lg;//分析表产生器
	int stateNum, alphaNum;//状态数及字母表数（分析表的行与列）
	int cursor;//Token序列的游标
	stack<string> alphaStk;//符号栈
	stack<SLRNode> stateStk;//状态栈
	stack<Elem> opStk;//算符栈
	stack<int> MemOff;//结构成员区距
	stack<int> LenStk;//结构体长度信息栈
	stack<int> AnStk;//既是数组下标栈，也是结构体的索引栈
	bool isStruct;//在定义变量时判断是否为结构体
	SLRAnalyzer(Scanner *SCAN, ListGenerator *LG) :scan(SCAN), lg(LG)
	{
		isStruct = false;
		if (!lg->chart.empty())
		{
			stateNum = lg->chart.size();
			alphaNum = lg->chart.at(0).size();
		}
		cursor = -1;//游标初始化
		///初始化类型表(int float char void)
		///基本类型的listPtr规定存储基本类型值单元
		TypeList.push_back(TYPEL(INT,sizeof(int)));
		TypeList.push_back(TYPEL(FLOAT, sizeof(float)));
		TypeList.push_back(TYPEL(CHAR, sizeof(char)));
		TypeList.push_back(TYPEL(BOOL, sizeof(bool)));
		TypeList.push_back(TYPEL(VOID, 0));//注意枚举类型的值与其在typelist中的位置相同
		Quarts.push_back(Quarter(Elem(opK, FindList("bg", OpList)), Elem(DefaultW, -1), //四元式开始
						         Elem(DefaultW, -1), Elem(kT, FindList("main", KeyList))));
	}
	///转移函数
	SLRNode trans(int src, string str)
	{
		SLRNode rn(-1, 0);
		if (!(str == "#" || lg->gram->isNon(str) || lg->gram->isTermin(str)))
		{//字符出错检测
			cout << str << " can't be recognised!" << endl; exit(0);
		}
		if (src < stateNum && src > -1)
		{
			rn = lg->chart.at(src).at(lg->gram->symPos[str]);
		}
		return rn;
	}
	///下一个字符根据当前游标返回字符
	void getNext(string &str)//注意在next中完成对符号所代表的意思的转换
	{
		cursor++;
		if (cursor == int(scan->token.size()))//出界检查
		{
			cerr << "wrong current token pos overflow!" << endl; exit(0);
		}
		switch (scan->token[cursor].kind)//转义
		{
		case kT://关键字
			str = KeyList.at(scan->token[cursor].addrPtr);
			break;
		case pT://界符(支持+- * /（）> < == <= >=)
			str = PList.at(scan->token[cursor].addrPtr);
			break;
		case nT://整型常数
			str = "nT";
			break;
		case sT://字符串
			str = "sT";
			break;
		case cT://字符
			str = "cT";
			break;
		case fT://浮点常数
			str = "fT";
			break;
			break;
		case iT://标识符
			str = "iT";
			break;
		default:
			cout << "find the next wrong" << endl; exit(0);
		}
	}
	///规约状态处理函数
	string reduce(int state)//state指出规约式在原文法中的位置
	{
		if (state >= (int)lg->gram->genList.size())
		{
			return " ";//越界返回空
		}
		translate(state);
		string str = lg->gram->genList[state].genL;//规约式首部
		int num = lg->gram->genList[state].genR.size();//规约式长度，即弹栈次数
		if (num > 0 && lg->gram->genList[state].genR[0] == "$") num = 0;//遇空串
		for (int i = 0; i < num && !stateStk.empty(); i++)//出栈归约
		{
			stateStk.pop();
			alphaStk.pop();
		}if (stateStk.empty()) { cout << "state stack empty" << endl; exit(0); }
		return str;
	}
	///分析函数
	bool analyze()
	{
		bool flag = false;
		string curStr;//当前符号
		scan->nextToken();//读下一个单词
		getNext(curStr);//取单词
		SLRNode tempState;//临时状态，判断下一步动作时使用
		alphaStk.push("#");
		stateStk.push(SLRNode(0, false));
		while (cursor<int(scan->token.size()))
		{
			tempState = trans(stateStk.top().state, curStr);
			if (tempState == SLRNode(0, 1))//结束状态
			{
				flag = true;
				break;
			}//匹配成功
			else if (tempState.isReduce)//如果是可归约状态则归约
			{
				curStr = reduce(tempState.state);//栈顶状态归约，当前字符变为规约后的左部
				alphaStk.push(curStr);//规约后的左部入栈
				tempState = trans(stateStk.top().state, curStr);//临时状态变为当前状态栈顶扫刚入栈的左部后的状态
				stateStk.push(tempState);//转换后的状态入栈
				cursor--; getNext(curStr);//重新回到当前字符
			}
			else if (!tempState.isReduce && tempState.state > -1)//移进状态
			{
				alphaStk.push(curStr);//当前当前字符入栈
				stateStk.push(tempState);//当前状态入栈
				scan->nextToken();//读下一单词
				getNext(curStr);//更新当前字符
			}
			else 
			{
				printf("In %dth token wrong, entered a invalid state when meeting %s",cursor,curStr.c_str()); 
				exit(0);
			}
		}//while
		return flag;
	}

	bool translate(int state)//语义分析函数
	{//在规约状态插入动作函数
		//printf("%d\n",state);
		static int N = 0;
		if (state >64 && state < 74)
		{//将各种算符压入符号栈
			opStk.push(Elem(scan->token[cursor - 1].kind, scan->token[cursor - 1].addrPtr));
		}
		if (state  > 21 && state  < 25)
		{//将操作数语义栈
			int i = scan->token[cursor - 1].addrPtr;
			if (scan->token[cursor - 1].kind == iT && SymbolTable[i].typePtr == -1)//检查是否定义3
			{
				printf("\"%s\" is not declared!\n", SymbolTable[i].name.c_str());
				exit(0);
			}
			SEM.push(Elem(scan->token[cursor-1].kind, scan->token[cursor - 1].addrPtr));
		}
		Elem tempTop;
		int i; int j = VARF;//一些case中会用到的临时变量
		switch (state)
		{
		case 3://Def-> int main ( PrmDclr ) { FunBdy }
			Quarts.push_back(Quarter(Elem(opK, FindList("ed", OpList)), Elem(DefaultW, -1), //四元式开始
				Elem(DefaultW, -1), Elem(kT, FindList("main", KeyList))));//四元式结束
			break;
		case 5://Type-> int压栈
			TypeStk.push(Type(INT,sizeof(int)));//压入类型栈
			break;
		case 6://Type-> float
			TypeStk.push(Type(FLOAT,sizeof(float)));
			break;
		case 7://Type-> char
			TypeStk.push(Type(CHAR,sizeof(char)));
			break;
		case 8://Type-> void
			TypeStk.push(Type(VOID,0));
			break;
		case 9://Ptr-> *
			j = VARN;
		case 10://Ptr-> $
			i = scan->token[cursor].addrPtr;//指向符号表
			if (SymbolTable[i].typePtr != -1)//该符号在其他地方定义
			{
				if (FindList(TableNode(SymbolTable[i].name), PrmList) == -1)//参数没有重复
				{
					///添加新的参数到符号表
					SymbolTable.push_back(TableNode(SymbolTable[i].name, TypeStk.top().type, (EnumCAT)j, OffSet));
					i = SymbolTable.size() -1;//指向刚加入的地方
				}
				else { printf("params %s is redeclared.\n",SymbolTable[i].name.c_str()); exit(0); }
			}
			if (TypeStk.top().type <= VOID)//属于基本类型
			{
				PrmList.push_back(TableNode(SymbolTable[i].name, TypeStk.top().type, (EnumCAT)j, OffSet));//添加到形参表
				PrmN++;//记录参数个数
				SymbolTable[i].typePtr = TypeStk.top().type;//填写符号表的type项
				SymbolTable[i].cat = (EnumCAT)j;
				SymbolTable[i].addr = OffSet;//相对偏移量
				OffSet += TypeStk.top().valNum;
			}
			TypeStk.pop();
			break;
		case 11://Struct-> Init typedef struct { StrcBdy } IT ;//填写结构类型IT的信息
			if(MemOff.empty())
				isStruct = false;
			break;
		case 15://Member-> CatD VarD
			break;
		case 21://Fctor-> ViT		
			//iTVec.pop_back();
			break;
		case 25://PrmDclr-> Prm Prms
			i = SEM.top().addrPtr;//从语义栈中读出函数的指针信息，指向符号表
			SEM.pop();
			SymbolTable[i].addr = FuncList.size();//指向函数表
			FuncList.push_back(PINFL(PrmN, -1, PrmList.size() - PrmN));//填写函数表
			PrmN = 0;//参数个数清零
			///填写符号表中函数的信息
			break;
		case 27://Prm-> Type Ptr iT
			//先判断变量是否已经定义过
			break;
		case 30://FunBdy-> VarDefs FunBdys
			while (!SEM.empty())//函数体结束，语义栈清空
				SEM.pop();
			break;
		case 31://VarDefs-> VarD VarDefs
			break;
		case 32://VarDefs-> $
			break;
		case 33://VarD-> VarDef ;
			break;
		case 34://VarDef->Type DiT ArrAss Vars
			//填写符号表的地址项Addr
			if (isStruct)//结构体域成分信息
			{
				for (size_t i = 0; i < iTVec.size(); i++)
				{
					SymbolTable[iTVec[i].addrPtr].cat = DOMIN;//结构体域名种类
					SymbolTable[iTVec[i].addrPtr].addr = LengthList.size();
					if (TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].typeName <= VOID)//基本类型
					{
						LenStk.top() += TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr;//计算总长度
						LengthList.push_back(TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr);//长度表
						StructList.push_back(RINFL(SymbolTable[iTVec[i].addrPtr].name,
							       MemOff.top(), SymbolTable[iTVec[i].addrPtr].typePtr));//结构表
						MemOff.top() += TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr;//区距
					}
					else if (TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].typeName == ARRAY)//数组类型
					{//加上数组占用的值单元
						LengthList.push_back(ArrayList[TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr].valNum);//长度为数组单元数
						LenStk.top() += ArrayList[TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr].valNum;//计算总长度
						StructList.push_back(RINFL(SymbolTable[iTVec[i].addrPtr].name,
							       MemOff.top(), SymbolTable[iTVec[i].addrPtr].typePtr));//结构表填写
						MemOff.top() += ArrayList[TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr].valNum;//改变区距
					}
				}//for
			}//if
			else //函数内的定义信息
			{
				for (size_t i = 0; i < iTVec.size(); i++)
				{
					SymbolTable[iTVec[i].addrPtr].addr = OffSet;
					if (TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].typeName <= VOID)//基本类型
					{
						OffSet += TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr;
					}
					else if (TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].typeName == ARRAY)//数组类型
					{//加上数组占用的值单元
						OffSet += ArrayList[TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr].valNum;
					}
				}//for
			}
			iTVec.clear();
			TypeStk.pop();
			break;
		case 35://ArrAss-> Array
			i = iTVec.size() - 1;//指向定义性标识符最后一个元素
			SymbolTable[iTVec[i].addrPtr].typePtr = TypeList.size() - 1;
			break;
		case 36://ArrAss->Assign
			break;
		case 37://Vars->, DiT ArrAss Vars
			break;
		case 38://Vars-> $
			break;
		case 39://[ DnT ] Arr Array
			i = TypeList.size() - 1;
			j = AnStk.top();//存放数组下标的栈顶
			ArrayList.push_back(AINFL(j-1, i, ArrayList[TypeList[i].listPtr].valNum*j));//数组的高维
			TypeList.push_back(TYPEL(ARRAY, ArrayList.size() - 1));//类型表指向数组表
			AnStk.pop();//下标出栈			
			break;
		case 40://Array-> [ DnT ]
			//先填写数组表
			ArrayList.push_back(AINFL(AnStk.top()-1, TypeStk.top().type, TypeStk.top().valNum*AnStk.top()));
			TypeList.push_back(TYPEL(ARRAY, ArrayList.size() - 1));//类型表指向数组表
			AnStk.pop();//下标出栈
			//ArrNumStk.push(ArrayList.size() - 1);//数组表位置压栈
			break; 
		case 41://DiT->iT
			i = SymbolTable[scan->token[cursor - 1].addrPtr].typePtr;
			if (i > -1)//重定义检查
			{
				printf("%s has been declared!\n", SymbolTable[scan->token[cursor - 1].addrPtr].name.c_str());
				exit(0);
			}
			if (isStruct && TypeStk.empty())//结构体名字
			{
				SymbolTable[scan->token[cursor - 1].addrPtr].cat = TYPE;//结构体种类
				///填写结构体长度
				SymbolTable[scan->token[cursor - 1].addrPtr].addr = LengthList.size();
				LengthList.push_back(LenStk.top());//结构体的长度信息
				SymbolTable[scan->token[cursor - 1].addrPtr].typePtr = TypeList.size();//结构体类型
				TypeList.push_back(TYPEL(STRUCT,AnStk.top()));//结构体的类型
				LenStk.pop();
				AnStk.pop();
				MemOff.pop();//结构体域成分的区距
			}
			else//变量定义
			{
				SymbolTable[scan->token[cursor - 1].addrPtr].typePtr = TypeStk.top().type;
				//压入定义性标识符向量以备用
				iTVec.push_back(Elem(scan->token[cursor-1].kind, scan->token[cursor - 1].addrPtr));
			}
			//基本类型枚举值=在Typelist中的位置
			break;
		case 42://DnT-> nT
			AnStk.push(IntList.at(scan->token[cursor-1].addrPtr));
			break;
		case 43://Assign-> Ass RightV
			tempTop = SEM.top();//表达式值在栈顶，被赋值元素在次栈顶
			SEM.pop();
			Quarts.push_back(Quarter(opStk.top(), tempTop, Elem(DefaultW, -1), SEM.top()));
			opStk.pop(); SEM.pop();
			break;
		case 52:case 53:// ClAs->ViT Assign;
			//SEM.pop();
			break;
		case 63://While-> while WH ( Logic ) DO { FunBdy }
			Quarts.push_back(Quarter(Elem(opK, FindList("we", OpList))));
			break;
		case 16://+ -
		case 18://* /
		case 64://比较运算Logic-> Expr Cmp Expr
			Send(opStk.top());
			opStk.pop();
			break;
		case 65://Ass-> =//被赋值符号入栈
			//SEM.push(Elem(scan->token[cursor-2].kind,scan->token[cursor-2].addrPtr));
			break;
		case 74://If-> if ( Logic ) IF { FunBdy } Else
			Quarts.push_back(Quarter(Elem(opK, FindList("ie", OpList))));
			break;
		case 77://Retrn-> return Expr ;
			Quarts.push_back(Quarter(Elem(opK, FindList("rt", OpList)), SEM.top(), 
									 Elem(DefaultW, -1), Elem(DefaultW, -1)));
			SEM.pop();
			break;
		case 78://Print-> printf ( ViT ) ;
			Quarts.push_back(Quarter(Elem(opK, FindList("pf", OpList)), SEM.top(),
				Elem(DefaultW, -1), Elem(DefaultW, -1)));
			SEM.pop();
			break;
		case 79://Scanf-> scanf ( ViT ) ;
			Quarts.push_back(Quarter(Elem(opK, FindList("sf", OpList)), SEM.top(),
				Elem(DefaultW, -1), Elem(DefaultW, -1)));
			SEM.pop();
			break;
		case 80://IF
			Quarts.push_back(Quarter(Elem(opK, FindList("if", OpList)), SEM.top(),
									 Elem(DefaultW,-1),Elem(DefaultW,-1)));
			SEM.pop();
			break;
		case 81://EL
			Quarts.push_back(Quarter(Elem(opK, FindList("el", OpList))));
			break;
		case 82://| 66 :WH-> $
			Quarts.push_back(Quarter(Elem(opK, FindList("wh", OpList))));
			break;
		case 83://| 67 : DO->$
			Quarts.push_back(Quarter(Elem(opK, FindList("do", OpList)), SEM.top(),
									 Elem(DefaultW, -1), Elem(DefaultW, -1)));
			SEM.pop();
			break;
		case 84://CatD-> $
			isStruct = true;
			break;
		case 85://ViT-> AiT AT
			if(N==0)
				SEM.push(iTVec.back());
			else N = 0;
			iTVec.pop_back();
			break;
		case 86://Init-> $
			LenStk.push(0);
			AnStk.push(StructList.size());//结构体表的索引栈
			MemOff.push(0);
			break;
		case 87://Main-> main
			SEM.push(Elem(iT,SymbolTable.size()));//函数待回填的信息压入语义栈
			SymbolTable.push_back(TableNode(KeyList[scan->token[cursor-1].addrPtr], INT, FUNC, 0));
			break;
		case 90://Elem->nT
		case 91://Elem->Elem, nT
			nTVec.push_back(IntList[scan->token[cursor-1].addrPtr]);
			break;
		case 92://AT->[Expr] AT,对一个数组类型需要做越界检查(目前先针对一维数组)
			N++;//层数记录
			if (N > 1)
			{
				printf("sorry, we only surpport one demensional array.\n");
				exit(0);
			}
			SendAQ();
			break;
		case 93://AT-> $
			break;
		case 94://AiT-> iT
			i = scan->token[cursor - 1].addrPtr;
			if (scan->token[cursor - 1].kind == iT && SymbolTable[i].typePtr == -1)//检查是否定义
			{
				printf("\"%s\" is not declared!\n", SymbolTable[i].name.c_str());
				exit(0);
			}
			iTVec.push_back(Elem(scan->token[cursor - 1].kind, scan->token[cursor - 1].addrPtr));
			break;
		case 95:
			SEM.push(iTVec.back());
			break;
		}
		return true;
	}
};
#endif // SLRANALYZER_H
