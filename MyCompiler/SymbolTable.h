#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "Grammer.h"
#include <sstream>
enum EnumWord { nT, fT, cT, kT, pT, sT, iT, Temp, aTemp, Entry, opK, DefaultW };//������������
const string WordMap[] = { "nT","fT","cT", "kT","pT","sT","iT","Temp", "aT", "Entry","opK","DefaultW" };
enum EnumTYPE { INT, FLOAT, CHAR, BOOL, VOID, STRING, ARRAY, STRUCT, DefaultT };//���ͣ�����int,float,char,bool��
const string TypeMap[] = { "INT", "FLOAT", "CHAR", "BOOL", "VOID", "STRING", "ARRAY", "STRUCT" };
enum EnumCAT { FUNC, CONST, TYPE, DOMIN, VAR, VARN, VARF , TEMP, DEFAULTC};//����
const string CatMap[] = { "FUNC","CONST","TYPE","DOMIN","VAR","VARN","VARF","TEMP", "DEFAULTC" };
enum EnumADDR { PFINFL, LENL, CONSL, VALL };//��ַ���������������¼��
enum Register { AX, BX, CX, DX, BP, SI, DI };
vector <int> nTVec;//��¼����ĳ�ʼ��ֵ
const int INIOff = 0;//������ʼ������
int OffSet = INIOff;//main�����ڣ��������ڴ�ƫ����
int Seg = 1;//�����λ�ã�������ת�ı��
int PrmN;//��������
//�����2
const vector<string> PList = { "+","-","*","/", "{","[","]","}","(",")",
							   ",",".","<",">", ":",";","&","|","!","\\",
							  "!=","=","%","&","==",">=","<=","&&","||","#" };

const vector<string> OpList = { "wh","do","we","if","el","ie" ,"rt","pf","sf","bg","ed","of" };
vector<int> IntList;//���ͳ�����3
vector<float> FloatList;//���㳣����4

vector<char> CharList;//���ַ���6
vector<string> StrList;//���ַ�����7

/// *************���ֱ��Ľڵ�Ԫ��*************** ///
	//��������ջ�Ľڵ�
	struct Type
	{
		EnumTYPE type;
		int valNum;
		Type(EnumTYPE typ,int VU) :type(typ),valNum(VU) {}
	};

	//�����ڵ�
	struct AINFL
	{
		int low;//�Ͻ�
		int high;//�½�
		int typePtr;//�ɷ�����ָ��
		int valNum;//ֵ��Ԫ����
		AINFL(int high, int ptr, int valNum) :
		high(high),typePtr(ptr),valNum(valNum){low = 0; }
	};

	//�ṹ���ڵ�
	struct RINFL
	{
		string name;//����
		int off;//�����ƫ��ֵ
		int typePtr;//�ɷ�����ָ��
		RINFL(string name, int off, int typeptr) :
			name(name), off(off), typePtr(typeptr) {}
	};

	//���ͱ�ڵ�
	struct TYPEL
	{
		EnumTYPE typeName;//����
		int listPtr;//ָ��ṹ��������ָ��
		TYPEL(EnumTYPE TN,int LP) :typeName(TN),listPtr(LP) {}
	};

	//������ڵ�
	struct PINFL
	{
		int level;//���������κ�
		int off;//������ƫ����
		int paramNum;//��������
		int entry;//��ڵ�ַ
		int paramPtr;//�β�ָ��
		PINFL() {}
		PINFL(int paramNum,int entry,int paramPtr):
			paramNum(paramNum), entry(entry), paramPtr(paramPtr) {
			level = 0; off = 0;
		}
	};

	//��ʶ���б�ڵ�
	struct TableNode
	{
		string name;//��ʶ����
		int typePtr;//��ʶ������ָ��
		EnumCAT cat;//��ʶ������
		int addr;//��ַ��
		int activeL;//���ű�Ļ�Ծ������չ(-5��ʾ����أ�-2��ʾ����Ծn��-1��ʾ��Ծy������0��ʾΪ����������0��ʾ����Ӧ����Ԫʽ����Ծ)
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
		}//������ƥ��
		void initActive()
		{
			activeL = -5;//Ĭ�ϲ����,��������
			if (cat == VAR || cat == VARF || cat == VARN)//�û�������Ծ
			{
				activeL = -1;
			}
			else if (cat == TEMP)//��ʱ��������Ծ
				activeL = -2;
		}
	};

	struct Elem//��ԪʽԪ�ؽڵ�,��token��������
	{/*˵����Elemʵ������ָ����ֱ��ָ��
	 ��kind=iT��Tempʱָ����ű�
	 ��kind=nT��fT��cT��sTʱָ����Ӧ������
	 ��kind=opKʱ��ָ���Զ����������OpList
	 ��֮����kind������ָ��ı�����Ͳ�һ��
	 **/
		EnumWord kind;//Ԫ��ָ������
		int addrPtr;//Ԫ�ص�ַָ��,ָ����ű������
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
		Elem op;//Ԫ��1�����ָ��
		Elem opl;
		Elem opr;
		Elem des;//Ԫ��4,Ŀ�Ĳ�����
		int leftL, rightL, desL;//�����������Ҳ������Ļ�Ծ��Ϣ,-2����Ծ��-1��Ծ������0�ǳ���������Ϊ����Ԫʽq����Ծ
		Quarter() {}
		Quarter(Elem OP) :op(OP) { opl = opr = des = Elem(DefaultW, -1); initL(); }//ֻ������λ����Ԫʽ
		Quarter(Elem OP, Elem OPL, Elem OPR, Elem DES) :op(OP), opl(OPL), opr(OPR), des(DES) { initL(); }
	private:void initL() { leftL = rightL = desL = -5; }//��ʼ��Ϊ�����
	};

	/// **********�����嶯����ص�ջ********** ///
	stack<Elem> SEM;//����ջ
	map<int, EnumTYPE> tempMap, aTempMap;//��ʱ���������ͱ��ϵĵ�ַӳ��
	vector<Elem> iTVec;//���涨���Ա�ʶ������
	stack<Type> TypeStk;//���ͺ�����ջ
	vector<Quarter> Quarts;//��Ԫʽ��
	/// *****************��*********************** ///
	vector<TableNode> SymbolTable;//���ű��5

	vector<int> LengthList;//���ȱ�
	vector<TYPEL> TypeList;//���ͱ�
	vector<AINFL> ArrayList;//�����
	vector<RINFL> StructList;//�ṹ��
	vector<PINFL> FuncList;//������
	vector<TableNode> PrmList;//�βα�
	//�ؼ��ֱ�1
	const vector<string> KeyList = { "double","int","struct","float","void","break",
							"case","main","char","return","string","printf","if",
							"continue","do","const","long","switch","else",
							"for","default","while", "static", "typedef" };

	template<class T, class P>
	//ֻҪ�����˵��ڷ��ŽԿ����ô˺�������
	int FindList(T elem, const P &SomeList)//�б���Һ���
	{
		int flag = SomeList.size() - 1;
		while (flag > -1 && !(SomeList[flag] == elem))
			flag--;
		return flag;
	}
	void ResetL()//�����ű�Ļ�Ծ��Ϣ��λ
	{
		for (size_t i = 0; i < SymbolTable.size(); i++)
			SymbolTable[i].initActive();
	}
	EnumTYPE TypeOf(const Elem &elem)//����token����Ԫ������
	{
		switch (elem.kind)
		{
		case iT://��ʶ��
			if (SymbolTable[elem.addrPtr].typePtr == -1)
			{
				printf("\"%s\" is not declared!\n", SymbolTable[elem.addrPtr].name.c_str());
				exit(0);
			}
			return TypeList[SymbolTable[elem.addrPtr].typePtr].typeName;
		case Temp:
			return tempMap[elem.addrPtr];//��addrPtr����ʱ����������
		case aTemp:
			return aTempMap[elem.addrPtr];
		case nT://���ͳ���
			return  INT;
		case fT://���㳣��
			return FLOAT;
		case cT://�����ַ�
			return CHAR;
		case sT://���ַ���
			return STRING;
		}return DefaultT;
	}
	void FillTemp(const Elem & elem, EnumTYPE tp)
	{//����ʱ�������,tp����ʱ����������Ҳ���������ͱ��е�λ��,��Ӧ���ű��listPtr��
		static int tempN = 0;//��ʱ��������Ŀ
		static int idN = 0;//���ڼ�¼�����ڴ��ַ�ı���
		if (tp > VOID) { printf("temp variable is invalid!\n"); exit(0); }
		stringstream ss;
		if (elem.kind == Temp)//��ʱ��������д
		{
			tempMap[elem.addrPtr] = tp;//��ʱ�����ڷ��ű��е�λ�������͵�ӳ��
			ss << "t" << tempN++;//�����ַ�ת��
			SymbolTable.push_back(TableNode(ss.str(), tp, TEMP, OffSet));//��ʱ����������ű�
		}
		else if (elem.kind == aTemp)//��¼�����±����ʱ����
		{
			aTempMap[elem.addrPtr] = tp;//��ʱ�����ڷ��ű��е�λ�������͵�ӳ��
			ss << "i" << idN++;
			SymbolTable.push_back(TableNode(ss.str(), tp, VAR, OffSet));//�����±궨��Ϊ��ʱ����
		}
		OffSet += TypeList[tp].listPtr;//�޸�ƫ��ֵ
		ss.str("");
	}
	void ShowSymbolTable();
	void typeCmp(const Elem &opl, const Elem &opr)//�����������ͱȽ�
	{
		if (TypeOf(opl)!=TypeOf(opr))//�����������ͼ��
		{
			ShowSymbolTable();
			printf("[opl,%s,%d][opr,%s,%d]\n", WordMap[opl.kind].c_str(),opl.addrPtr,
				WordMap[opr.kind].c_str(), opr.addrPtr);
			printf("can't calculate two number with diffrent kind!\n");
			exit(0);
		}
	}
	//������Ԫʽ
	void Send(Elem op)//��ʱ��֧��������ͬ���͵Ĳ�������������
	{
		if (op.kind != opK && op.kind!=pT) //op.kindֻ����pT����opK 
		{
			printf("operator's kind error\n"); exit(0);
		}
		Elem opr = SEM.top();//ջ��Ϊ�Ҳ�����
		SEM.pop();
		typeCmp(SEM.top(), opr);//�����������ͼ��
		Elem tempData(Temp, SymbolTable.size());//������ʱ����
		//��Ҫ����ʱ����������ű�
		FillTemp(tempData,TypeOf(opr));//�������Ҳ�������ͬ
		Quarts.push_back(Quarter(op, SEM.top(), opr, tempData));
		SEM.pop();
		SEM.push(tempData);
	}
	void SendAQ()//�����������Ԫʽ
	{
		Elem aIT = iTVec.back();//ջ��Ԫ��ΪiT,������ı�ʶ��
		int i = SymbolTable[aIT.addrPtr].typePtr;//��ȡ��ʶ������ָ��
		int j;
		if (TypeList[i].typeName == ARRAY)
		{
			if (SEM.top().kind == nT)//SEM.top��Expr�����������ж��±�����
			{
				j = IntList[SEM.top().addrPtr];//����ֵ�����Գ����±���Խ����
				if (!(j >= ArrayList[TypeList[i].listPtr].low 
					&& j <= ArrayList[TypeList[i].listPtr].high))//�±귶Χ����
				{//�����Ļ����ڱ���׶�ֱ�Ӹ��������ҵ�ƫ����
					printf("%d is out of range of array %s ",j, SymbolTable[aIT.addrPtr].name.c_str() );
					exit(0);
				}
			}
			j = FindList(4, IntList);//jΪ�����������������4��int���͵�ֵ��Ԫ��
			if (j == -1)
			{
				j = IntList.size();
				IntList.push_back(4);
			}//��jָ������ָ��
			SEM.push(Elem(nT, j));
			//�ǳ��������޷��ڱ���ʱ�������ͼ�飨temp��iT���ͣ�
			Send(Elem(pT, FindList("*", PList)));//������ڴ��ַ
			///ע��Send֮��SEM.top()�����ı�
			Elem aTemp(aTemp, SymbolTable.size());
			if (TypeList[SymbolTable[SEM.top().addrPtr].typePtr].typeName == INT)//�±���int���ͣ�
				FillTemp(aTemp, INT);//�����±�Ĭ����int����
			else
			{
				printf("array's index must be Int type.\n");
				exit(0);
			}
			Quarts.push_back(Quarter(Elem(opK, FindList("of", OpList)), Elem(iTVec.back()), SEM.top(), aTemp));
			SEM.pop(); SEM.push(aTemp);//��������ѹ������ջ
		}//��ʶ���������ͼ��
		else 
		{
			printf("\"%s\" is not a array.\n", SymbolTable[aIT.addrPtr].name.c_str()); 
			exit(0);
		}
	}
	template<class ListType>
	//���stringList���
	void ShowWordList(const ListType &list, int size = 0)
	{
		for (int i = 0, j = 0; i < size; i++)//����ķ�
		{
			j++;
			printf("[%3d,%-10s]", i, list.at(i).c_str());
			if (j == 5) { printf("\n"); j = 0; }
		}printf("\n");
	}
	//��ʾ��Ԫʽ����
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
			///�������ֶ�
			if (Quad.at(i).op.kind == pT)printf("%-6s,", PList.at(Quad.at(i).op.addrPtr).c_str());
			else if(Quad.at(i).op.kind == opK) printf("%-6s,", OpList.at(Quad.at(i).op.addrPtr).c_str());
			else { printf("wrong operator!\n"); }
			///������ֶ�
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
			///�Ҳ����ֶ�
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
			///Ŀ���ֶ�
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
	//��ʾ����ű������Ϣ
	void ShowSymbolTable()
	{
		///���ű�
		printf("+-----------------------------------------------+\n");
		printf("|                   ���ű�:%3d                  |\n", SymbolTable.size());
		printf("|-----------------------------------------------|\n");
		printf("| i  |     name   |typePtr |  cat | addr|activeL|\n");
		printf("+----+------------+--------+------+-----+-------+\n");
		for (int i = 0; i<int(SymbolTable.size()); i++)//���ű�����
		{
			printf("|%3d |%10s  | %5d  |%5s |%4d |%5d  |\n", i, 
				SymbolTable.at(i).name.c_str(),
				SymbolTable.at(i).typePtr,
				CatMap[SymbolTable.at(i).cat].c_str(), 
				SymbolTable.at(i).addr,
				SymbolTable.at(i).activeL);
			printf("+----+------------+--------+------+-----+-------+\n");
		
}
		///������
		printf("+-----------------------------------------+\n");
		printf("|                ������:%3d               |\n", FuncList.size());
		printf("|-----------------------------------------|\n");
		printf("| i  |level |  off   | PrmN |entry|prmPtr |\n");
		printf("+----+------+--------+------+-----+-------+\n");
		for (int i = 0; i<int(FuncList.size()); i++)//���ű�����
		{
			printf("|%3d |%4d  | %5d  |%5d |%4d |%5d  |\n", i,
				FuncList[i].level,FuncList[i].off,FuncList[i].paramNum,
				FuncList[i].entry,FuncList[i].paramPtr);
			printf("+----+------+--------+------+-----+-------+\n");
		}
		///�βα�
		printf("+-----------------------------------------------+\n");
		printf("|                   �βα�:%3d                  |\n", PrmList.size());
		printf("|-----------------------------------------------|\n");
		printf("| i  |     name   |typePtr |  cat | addr|activeL|\n");
		printf("+----+------------+--------+------+-----+-------+\n");
		for (int i = 0; i<int(PrmList.size()); i++)//���ű�����
		{
			printf("|%3d |%10s  | %5d  |%5s |%4d |%5d  |\n", i,
				PrmList[i].name.c_str(),
				PrmList[i].typePtr,
				CatMap[PrmList[i].cat].c_str(),
				PrmList[i].addr,
				PrmList[i].activeL);
			printf("+----+------------+--------+------+-----+-------+\n");

		}
		///���ͱ�
		printf("+---------------------+\n");
		printf("|      ���ͱ�:%3d     |\n", TypeList.size());
		printf("+---------------------+\n");
		printf("| i  |  Type  |listPtr|\n");
		printf("+----+--------+-------+\n");
		for (int i = 0; i<int(TypeList.size()); i++)//�����
		{
			printf("|%3d | %6s |%5d  |\n", i, 
				TypeMap[TypeList[i].typeName].c_str(), TypeList[i].listPtr);
			printf("+----+--------+-------+\n");
		}printf("\n\n");
		///���ȱ�
		printf("+--------------+\n");
		printf("|  ���ȱ�:%3d  |\n", LengthList.size());
		printf("+--------------+\n");
		printf("| i  |  value  |\n");
		printf("+----+---------+\n");
		for (int i = 0; i<int(LengthList.size()); i++)//�����
		{
			printf("|%3d |  %5d  |\n", i, LengthList[i]);
			printf("+----+---------+\n");
		}printf("\n\n");
		///�����
		printf("+-----------------------------+\n");
		printf("|           �����:%3d        |\n", ArrayList.size());
		printf("+-----------------------------+\n");
		printf("| i  |low| high|typePtr | Unit|\n");
		printf("+----+---+-----+--------+-----+\n");
		for (int i = 0; i<int(ArrayList.size()); i++)//�����
		{
			printf("|%3d | %d |%3d  | %5d  |%3d  |\n", i,
				ArrayList[i].low, ArrayList[i].high, 
				ArrayList[i].typePtr, ArrayList[i].valNum);
			printf("+----+---+-----+--------+-----+\n");
		}printf("\n\n");
		///�ṹ��
		printf("+-------------------------------+\n");//���ű�����
		printf("|           �ṹ��:%3d          |\n",StructList.size());
		printf("+-------------------------------+\n");
		printf("| i  |   name  |  off  |typePtr |\n");
		printf("+----+---------+-------+--------+\n");
		for (int i = 0; i<int(StructList.size()); i++)//���ű�����
		{
			printf("|%3d | %7s | %4d  | %5d  |\n", i,
				StructList[i].name.c_str(), StructList[i].off,
				StructList[i].typePtr);
			printf("+----+---------+-------+--------+\n");
		}printf("\n");
	}
	void ShowList()//��ʾ�������ű�����
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
		for (size_t i = 0; i < seg.size(); i += 2)//��ʾԴ��Ԫʽ������Ϣ
		{
			ShowQuarts(seg[i], seg[i + 1], quad);
			printf("\n");
		}
	}
#endif // !SYMBOLTABLE_H