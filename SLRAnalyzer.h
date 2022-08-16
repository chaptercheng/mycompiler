#ifndef SLRANALYZER_H
#define SLRANALYZER_H
#include "ListGenerator.h"
/**
*@author ����
*LR(0) ��������Ҫ�Լ����д��lrChart�ļ���
*���ݷ���������������վstateStk,alphaStk;
*��LR����������Ϊ0��ʾ�գ�����1000��Ϊ��Լ״̬��
*����1000ȡ���Ϊ��Լ�Ƕ�Ӧ���ķ����ʽ
*ע���ķ��Ķ�ȡҪ���ķ��ķ�������ָ���Ĺ�Լ��Ŀ
*Ӧ���ǰ����ķ�����ĳ���˳����
*/

class SLRAnalyzer
{
public:
	Scanner *scan;//�ʷ�������
	ListGenerator *lg;//�����������
	int stateNum, alphaNum;//״̬������ĸ������������������У�
	int cursor;//Token���е��α�
	stack<string> alphaStk;//����ջ
	stack<SLRNode> stateStk;//״̬ջ
	stack<Elem> opStk;//���ջ
	stack<int> MemOff;//�ṹ��Ա����
	stack<int> LenStk;//�ṹ�峤����Ϣջ
	stack<int> AnStk;//���������±�ջ��Ҳ�ǽṹ�������ջ
	bool isStruct;//�ڶ������ʱ�ж��Ƿ�Ϊ�ṹ��
	SLRAnalyzer(Scanner *SCAN, ListGenerator *LG) :scan(SCAN), lg(LG)
	{
		isStruct = false;
		if (!lg->chart.empty())
		{
			stateNum = lg->chart.size();
			alphaNum = lg->chart.at(0).size();
		}
		cursor = -1;//�α��ʼ��
		///��ʼ�����ͱ�(int float char void)
		///�������͵�listPtr�涨�洢��������ֵ��Ԫ
		TypeList.push_back(TYPEL(INT,sizeof(int)));
		TypeList.push_back(TYPEL(FLOAT, sizeof(float)));
		TypeList.push_back(TYPEL(CHAR, sizeof(char)));
		TypeList.push_back(TYPEL(BOOL, sizeof(bool)));
		TypeList.push_back(TYPEL(VOID, 0));//ע��ö�����͵�ֵ������typelist�е�λ����ͬ
		Quarts.push_back(Quarter(Elem(opK, FindList("bg", OpList)), Elem(DefaultW, -1), //��Ԫʽ��ʼ
						         Elem(DefaultW, -1), Elem(kT, FindList("main", KeyList))));
	}
	///ת�ƺ���
	SLRNode trans(int src, string str)
	{
		SLRNode rn(-1, 0);
		if (!(str == "#" || lg->gram->isNon(str) || lg->gram->isTermin(str)))
		{//�ַ�������
			cout << str << " can't be recognised!" << endl; exit(0);
		}
		if (src < stateNum && src > -1)
		{
			rn = lg->chart.at(src).at(lg->gram->symPos[str]);
		}
		return rn;
	}
	///��һ���ַ����ݵ�ǰ�α귵���ַ�
	void getNext(string &str)//ע����next����ɶԷ������������˼��ת��
	{
		cursor++;
		if (cursor == int(scan->token.size()))//������
		{
			cerr << "wrong current token pos overflow!" << endl; exit(0);
		}
		switch (scan->token[cursor].kind)//ת��
		{
		case kT://�ؼ���
			str = KeyList.at(scan->token[cursor].addrPtr);
			break;
		case pT://���(֧��+- * /����> < == <= >=)
			str = PList.at(scan->token[cursor].addrPtr);
			break;
		case nT://���ͳ���
			str = "nT";
			break;
		case sT://�ַ���
			str = "sT";
			break;
		case cT://�ַ�
			str = "cT";
			break;
		case fT://���㳣��
			str = "fT";
			break;
			break;
		case iT://��ʶ��
			str = "iT";
			break;
		default:
			cout << "find the next wrong" << endl; exit(0);
		}
	}
	///��Լ״̬������
	string reduce(int state)//stateָ����Լʽ��ԭ�ķ��е�λ��
	{
		if (state >= (int)lg->gram->genList.size())
		{
			return " ";//Խ�緵�ؿ�
		}
		translate(state);
		string str = lg->gram->genList[state].genL;//��Լʽ�ײ�
		int num = lg->gram->genList[state].genR.size();//��Լʽ���ȣ�����ջ����
		if (num > 0 && lg->gram->genList[state].genR[0] == "$") num = 0;//���մ�
		for (int i = 0; i < num && !stateStk.empty(); i++)//��ջ��Լ
		{
			stateStk.pop();
			alphaStk.pop();
		}if (stateStk.empty()) { cout << "state stack empty" << endl; exit(0); }
		return str;
	}
	///��������
	bool analyze()
	{
		bool flag = false;
		string curStr;//��ǰ����
		scan->nextToken();//����һ������
		getNext(curStr);//ȡ����
		SLRNode tempState;//��ʱ״̬���ж���һ������ʱʹ��
		alphaStk.push("#");
		stateStk.push(SLRNode(0, false));
		while (cursor<int(scan->token.size()))
		{
			tempState = trans(stateStk.top().state, curStr);
			if (tempState == SLRNode(0, 1))//����״̬
			{
				flag = true;
				break;
			}//ƥ��ɹ�
			else if (tempState.isReduce)//����ǿɹ�Լ״̬���Լ
			{
				curStr = reduce(tempState.state);//ջ��״̬��Լ����ǰ�ַ���Ϊ��Լ�����
				alphaStk.push(curStr);//��Լ�������ջ
				tempState = trans(stateStk.top().state, curStr);//��ʱ״̬��Ϊ��ǰ״̬ջ��ɨ����ջ���󲿺��״̬
				stateStk.push(tempState);//ת�����״̬��ջ
				cursor--; getNext(curStr);//���»ص���ǰ�ַ�
			}
			else if (!tempState.isReduce && tempState.state > -1)//�ƽ�״̬
			{
				alphaStk.push(curStr);//��ǰ��ǰ�ַ���ջ
				stateStk.push(tempState);//��ǰ״̬��ջ
				scan->nextToken();//����һ����
				getNext(curStr);//���µ�ǰ�ַ�
			}
			else 
			{
				printf("In %dth token wrong, entered a invalid state when meeting %s",cursor,curStr.c_str()); 
				exit(0);
			}
		}//while
		return flag;
	}

	bool translate(int state)//�����������
	{//�ڹ�Լ״̬���붯������
		//printf("%d\n",state);
		static int N = 0;
		if (state >64 && state < 74)
		{//���������ѹ�����ջ
			opStk.push(Elem(scan->token[cursor - 1].kind, scan->token[cursor - 1].addrPtr));
		}
		if (state  > 21 && state  < 25)
		{//������������ջ
			int i = scan->token[cursor - 1].addrPtr;
			if (scan->token[cursor - 1].kind == iT && SymbolTable[i].typePtr == -1)//����Ƿ���3
			{
				printf("\"%s\" is not declared!\n", SymbolTable[i].name.c_str());
				exit(0);
			}
			SEM.push(Elem(scan->token[cursor-1].kind, scan->token[cursor - 1].addrPtr));
		}
		Elem tempTop;
		int i; int j = VARF;//һЩcase�л��õ�����ʱ����
		switch (state)
		{
		case 3://Def-> int main ( PrmDclr ) { FunBdy }
			Quarts.push_back(Quarter(Elem(opK, FindList("ed", OpList)), Elem(DefaultW, -1), //��Ԫʽ��ʼ
				Elem(DefaultW, -1), Elem(kT, FindList("main", KeyList))));//��Ԫʽ����
			break;
		case 5://Type-> intѹջ
			TypeStk.push(Type(INT,sizeof(int)));//ѹ������ջ
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
			i = scan->token[cursor].addrPtr;//ָ����ű�
			if (SymbolTable[i].typePtr != -1)//�÷����������ط�����
			{
				if (FindList(TableNode(SymbolTable[i].name), PrmList) == -1)//����û���ظ�
				{
					///����µĲ��������ű�
					SymbolTable.push_back(TableNode(SymbolTable[i].name, TypeStk.top().type, (EnumCAT)j, OffSet));
					i = SymbolTable.size() -1;//ָ��ռ���ĵط�
				}
				else { printf("params %s is redeclared.\n",SymbolTable[i].name.c_str()); exit(0); }
			}
			if (TypeStk.top().type <= VOID)//���ڻ�������
			{
				PrmList.push_back(TableNode(SymbolTable[i].name, TypeStk.top().type, (EnumCAT)j, OffSet));//��ӵ��βα�
				PrmN++;//��¼��������
				SymbolTable[i].typePtr = TypeStk.top().type;//��д���ű��type��
				SymbolTable[i].cat = (EnumCAT)j;
				SymbolTable[i].addr = OffSet;//���ƫ����
				OffSet += TypeStk.top().valNum;
			}
			TypeStk.pop();
			break;
		case 11://Struct-> Init typedef struct { StrcBdy } IT ;//��д�ṹ����IT����Ϣ
			if(MemOff.empty())
				isStruct = false;
			break;
		case 15://Member-> CatD VarD
			break;
		case 21://Fctor-> ViT		
			//iTVec.pop_back();
			break;
		case 25://PrmDclr-> Prm Prms
			i = SEM.top().addrPtr;//������ջ�ж���������ָ����Ϣ��ָ����ű�
			SEM.pop();
			SymbolTable[i].addr = FuncList.size();//ָ������
			FuncList.push_back(PINFL(PrmN, -1, PrmList.size() - PrmN));//��д������
			PrmN = 0;//������������
			///��д���ű��к�������Ϣ
			break;
		case 27://Prm-> Type Ptr iT
			//���жϱ����Ƿ��Ѿ������
			break;
		case 30://FunBdy-> VarDefs FunBdys
			while (!SEM.empty())//���������������ջ���
				SEM.pop();
			break;
		case 31://VarDefs-> VarD VarDefs
			break;
		case 32://VarDefs-> $
			break;
		case 33://VarD-> VarDef ;
			break;
		case 34://VarDef->Type DiT ArrAss Vars
			//��д���ű�ĵ�ַ��Addr
			if (isStruct)//�ṹ����ɷ���Ϣ
			{
				for (size_t i = 0; i < iTVec.size(); i++)
				{
					SymbolTable[iTVec[i].addrPtr].cat = DOMIN;//�ṹ����������
					SymbolTable[iTVec[i].addrPtr].addr = LengthList.size();
					if (TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].typeName <= VOID)//��������
					{
						LenStk.top() += TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr;//�����ܳ���
						LengthList.push_back(TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr);//���ȱ�
						StructList.push_back(RINFL(SymbolTable[iTVec[i].addrPtr].name,
							       MemOff.top(), SymbolTable[iTVec[i].addrPtr].typePtr));//�ṹ��
						MemOff.top() += TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr;//����
					}
					else if (TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].typeName == ARRAY)//��������
					{//��������ռ�õ�ֵ��Ԫ
						LengthList.push_back(ArrayList[TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr].valNum);//����Ϊ���鵥Ԫ��
						LenStk.top() += ArrayList[TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr].valNum;//�����ܳ���
						StructList.push_back(RINFL(SymbolTable[iTVec[i].addrPtr].name,
							       MemOff.top(), SymbolTable[iTVec[i].addrPtr].typePtr));//�ṹ����д
						MemOff.top() += ArrayList[TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr].valNum;//�ı�����
					}
				}//for
			}//if
			else //�����ڵĶ�����Ϣ
			{
				for (size_t i = 0; i < iTVec.size(); i++)
				{
					SymbolTable[iTVec[i].addrPtr].addr = OffSet;
					if (TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].typeName <= VOID)//��������
					{
						OffSet += TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr;
					}
					else if (TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].typeName == ARRAY)//��������
					{//��������ռ�õ�ֵ��Ԫ
						OffSet += ArrayList[TypeList[SymbolTable[iTVec[i].addrPtr].typePtr].listPtr].valNum;
					}
				}//for
			}
			iTVec.clear();
			TypeStk.pop();
			break;
		case 35://ArrAss-> Array
			i = iTVec.size() - 1;//ָ�����Ա�ʶ�����һ��Ԫ��
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
			j = AnStk.top();//��������±��ջ��
			ArrayList.push_back(AINFL(j-1, i, ArrayList[TypeList[i].listPtr].valNum*j));//����ĸ�ά
			TypeList.push_back(TYPEL(ARRAY, ArrayList.size() - 1));//���ͱ�ָ�������
			AnStk.pop();//�±��ջ			
			break;
		case 40://Array-> [ DnT ]
			//����д�����
			ArrayList.push_back(AINFL(AnStk.top()-1, TypeStk.top().type, TypeStk.top().valNum*AnStk.top()));
			TypeList.push_back(TYPEL(ARRAY, ArrayList.size() - 1));//���ͱ�ָ�������
			AnStk.pop();//�±��ջ
			//ArrNumStk.push(ArrayList.size() - 1);//�����λ��ѹջ
			break; 
		case 41://DiT->iT
			i = SymbolTable[scan->token[cursor - 1].addrPtr].typePtr;
			if (i > -1)//�ض�����
			{
				printf("%s has been declared!\n", SymbolTable[scan->token[cursor - 1].addrPtr].name.c_str());
				exit(0);
			}
			if (isStruct && TypeStk.empty())//�ṹ������
			{
				SymbolTable[scan->token[cursor - 1].addrPtr].cat = TYPE;//�ṹ������
				///��д�ṹ�峤��
				SymbolTable[scan->token[cursor - 1].addrPtr].addr = LengthList.size();
				LengthList.push_back(LenStk.top());//�ṹ��ĳ�����Ϣ
				SymbolTable[scan->token[cursor - 1].addrPtr].typePtr = TypeList.size();//�ṹ������
				TypeList.push_back(TYPEL(STRUCT,AnStk.top()));//�ṹ�������
				LenStk.pop();
				AnStk.pop();
				MemOff.pop();//�ṹ����ɷֵ�����
			}
			else//��������
			{
				SymbolTable[scan->token[cursor - 1].addrPtr].typePtr = TypeStk.top().type;
				//ѹ�붨���Ա�ʶ�������Ա���
				iTVec.push_back(Elem(scan->token[cursor-1].kind, scan->token[cursor - 1].addrPtr));
			}
			//��������ö��ֵ=��Typelist�е�λ��
			break;
		case 42://DnT-> nT
			AnStk.push(IntList.at(scan->token[cursor-1].addrPtr));
			break;
		case 43://Assign-> Ass RightV
			tempTop = SEM.top();//���ʽֵ��ջ��������ֵԪ���ڴ�ջ��
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
		case 64://�Ƚ�����Logic-> Expr Cmp Expr
			Send(opStk.top());
			opStk.pop();
			break;
		case 65://Ass-> =//����ֵ������ջ
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
			AnStk.push(StructList.size());//�ṹ��������ջ
			MemOff.push(0);
			break;
		case 87://Main-> main
			SEM.push(Elem(iT,SymbolTable.size()));//�������������Ϣѹ������ջ
			SymbolTable.push_back(TableNode(KeyList[scan->token[cursor-1].addrPtr], INT, FUNC, 0));
			break;
		case 90://Elem->nT
		case 91://Elem->Elem, nT
			nTVec.push_back(IntList[scan->token[cursor-1].addrPtr]);
			break;
		case 92://AT->[Expr] AT,��һ������������Ҫ��Խ����(Ŀǰ�����һά����)
			N++;//������¼
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
			if (scan->token[cursor - 1].kind == iT && SymbolTable[i].typePtr == -1)//����Ƿ���
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
