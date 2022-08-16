#ifndef SCANNER_H
#define SCANNER_H
#include "ConstMachine.h"
#include <iomanip>
#include "SymbolTable.h"

using namespace std;

class Scanner
{
public:
    DFA df;
    ConstMachine cm;
    string str;//���ڶ�̬�ַ�ƴ��
	string text;//Ҫʶ����ı���ǰ���ص��ڴ�
	bool isOneScan;//�ж��Ƿ���һ��ɨ��
	int curChar;//��ǰ�ʷ�����������ʶ�����ĸ
    void addToken(EnumWord id,int pos)
    {
        Elem t;
        t.kind=id;
        t.addrPtr=pos;
        token.push_back(t);
    }
public:
    vector<Elem> token;//token��������
    Scanner(string filename){
		if (!cm.readMachine("constMachine.txt"))//��ʼ�����������
		{
			printf("can't open constMachine.txt\n");
			exit(0);
		}
		if (!df.readDFA("scanner.txt"))
		{
			printf("can't open scanner.txt\n");
			exit(0);
		}
		if(!readText(filename))
		{
			printf("can't open %s.txt\n",filename.c_str());
			exit(0);
		}
		curChar = 0;
	}//��ʼ��ɨ������DFA

    void transfer(int &state,char ch)//��״̬state��ɨһ���ַ�ch
    {///���ڲ�ͬ״̬�¸����ַ�ch��ֵת��Ϊ��ĸ����ķ��ű�ʾ,��ת��
        switch(state)
        {
        case 0://b=�ո�d=����0-9��c=��ĸ��p=����>��<��=��r��ʾ�������
            if(ch==' '||ch=='\t'||ch=='\n')//�հ��ַ�����
                ch='b';
            else if(ch>='0' &&ch<='9')
                ch='d';
            else if((ch>='a' && ch<='z')||(ch>='A' && ch<='Z')|| ch=='_')
                ch='c';
            else if(ch=='>'||ch=='<'||ch=='='||ch=='!')
                ch='p';
			else if (ch == '#' || ch == '&' || ch == '|');
            else
            {
                string temp;
                temp.push_back(ch);
                if(FindList(temp,PList)!=-1)
                    ch='r';//�ò��ҽ�����ж��Ƿ�Ϊ�������
            }
            break;
        case 1:
            if((ch>='0' && ch<='9')||ch=='.'||ch=='e'||ch=='+'||ch=='-')
                ch='f';//��״̬1���ж��Ƿ�Ϊ���������Ҫ�������ĸ
            else ch='`';
            break;
        case 5:
            if(ch!='=')
                ch='`';
			break;
		case 11:
			if (ch != '\"')
				ch = '`';
			break;
		case 14:
			if (ch != '\'')
				ch = '`';
			break;
		case 17:
			if (ch != '&')
				ch = '`';
			break;
		case 19:
			if (ch != '|')
				ch = '`';
			break;
        }//switch
        if(state==6 || state==8 || state == 18 || state == 20)
            ch='`';//��6��8״̬�½���ǰ����Ϊ��̷�
        if(state==3 || state==11 || state==13 )
        {//��3��11��13״̬�²鿴��ǰ�����Ƿ�Ϊ�Ϸ���ʶ��������
            if((ch>='a' && ch<='z')||(ch>='A' && ch<='Z')|| ch=='_'||(ch>='0' && ch<='9'))
                ch='c';
            else if(state==3) ch='`';
        }
        state=df.transfer(state,ch);//״̬ת��
    }

	bool readText(string filename)
	{
		bool flag = false;
		text = "";
		ifstream inf;//��Ҫʶ����ļ�
		inf.open(filename);
		if (inf.is_open())
		{
			flag = true;
			string temp;
			while (getline(inf, temp))
				text.append(temp.substr(0, temp.find("//")));//��ȡ�ļ�,��ȥ��ע��
			text.append("#");
		}// if open
		return flag;
	}

	bool isFinal(int state)//�ж�״̬�Ƿ�Ϊ����״̬
	{
		vector<int>::iterator iter = find(df.finl.begin(), df.finl.end(), state);
		return (iter != df.finl.end());
	}
    bool nextToken()//ʶ�𵥴ʵ�һ����ĸ,ͬʱ����stateFun����
    {
			int state = df.start;//�ӿ�ʼ״̬����
			char ch;//��ǰ��ĸ
			while (curChar<int(text.size()) && !isFinal(state))
			{
				ch = text.at(curChar);//��ȡ��ǰ�ַ�
				//printf("%c", ch);
				transfer(state, ch);//����״̬ת�Ʊ�������һ��״̬
				if(!isFinal(state))
					curChar++;//�ǽ�������ת����һ����ĸ
				if (state > -1)
					stateFun(ch, state);//ɨ��һ��ch����״̬state�Ĳ���
				else
				{
					printf("the scanner can't recognise word,enter state %d\n",state);
					exit(0);
				}
			}//while
			return (state != 10);//nextTokenδ����
    }
    void stateFun(char ch,int &state)//��������
	{
        int k;
        switch(state)
        {
        case 1://������ƴ��
            str.push_back(ch);
            break;
        case 2://�����������״̬
            if((ch>='a' && ch<='z')||(ch>='A' && ch<='Z')|| ch=='_')
            {//��������ĺ�̷�������
                cout<<"error when meeting \""<<ch<<"\" in state "<<state<<endl;
                exit(0);
            }
            str.append(";");
            cm.readline(str);//�ó��������ʶ�������
            if(!cm.dou.empty())
            {
                float f=cm.dou.at(0);
                k=FindList(f,FloatList);//���Ҹ��㳣����
                if(k<0)//δ�ҵ�����븡�㳣����
                {
                    k=FloatList.size();
                    FloatList.push_back(f);
                }
                addToken(fT,k);
                cm.dou.clear();
            }
            if(!cm.con.empty())
            {
                int it=cm.con.at(0);
                k=FindList(it,IntList);//�������ͳ�����
                if(k<0)//δ�ҵ���������ͳ�����
                {
                    k=IntList.size();
                    IntList.push_back(it);
                }
                addToken(nT,k);
                cm.con.clear();
            }
            str.clear();//str���
            break;
        case 3://�ַ���ƴ��
            str.push_back(ch);
            break;
        case 4://ʶ���ʶ������״̬
            k=FindList(str,KeyList);//�Ȳ��ҹؼ��ֱ�
            if(k<0)//û���ҵ�
            {
                k=FindList(TableNode(str),SymbolTable);//���ұ�ʶ����
                if(k<0)
                {
                    k=SymbolTable.size();
                    SymbolTable.push_back(TableNode(str));
                }
                addToken(iT,k);
            }
            else//�ǹؼ���
            {
                addToken(kT,k);//�ؼ��ֵ�token
            }
            str.clear();
            break;
        case 5://> = <
        case 6://=
            str.push_back(ch);
            break;
        case 7://��ϵ����ʶ�����״̬
            k=FindList(str,PList);
            addToken(pT,k); 
            str.clear();
            break;
        case 8://�������
            str.push_back(ch);
            break;
        case 9://�������ʶ�����״̬
            k=FindList(str,PList);//���ҽ����
            addToken(pT,k);//���token
            str.clear();
            break;
        case 10://����#���˳�״̬
			curChar++;
			str.push_back('#');
			addToken(pT,FindList(str,PList));
			str.clear();
            break;
        case 11://��ʼ�����ַ���
            if(ch!='\"')
                str.push_back(ch);
            break;
        case 12://�ַ���ʶ�����
            k=FindList(str,StrList);//�����ַ���
            if(k==-1)//û�ҵ�������ַ�����
            {
                k=StrList.size();
                StrList.push_back(str);
            }
            str.clear();
            addToken(sT,k);
            break;
        case 13:
            break;
        case 14://�����ַ�
            if((ch>='a' && ch<='z')||(ch>='A' && ch<='Z')|| ch=='_')
                str.push_back(ch);
            else {cout<<"�ַ�����"<<endl;exit(0);}
            break;
        case 15://�ַ�ʶ�����״̬
            k=FindList(str[0],CharList);
            if(k==-1)//û�ҵ�
            {
                k=CharList.size();
                CharList.push_back(str[0]);
            }
            str.clear();
            addToken(cT,k);
            break;
        case 16://������״̬
            cout<<"ɨ���ַ�"<<ch<<"����"<<endl;
            exit(0);
		case 17:case 18:case 19:case 20:
			str.push_back(ch);
			break;
        }//switch
    }//stateFun

    void showToken()
    {//���Token����
        for(int i=0,j=0;i<int(token.size());i++)
        {
			j++;
			printf("%4d(%s,%3d) ", i, WordMap[token[i].kind].c_str(), token[i].addrPtr);
			if (j == 10) { printf("\n"); j = 0; }
        }
        cout<<endl;
        ShowList();
    }
};
#endif // SCANNER_H
/**
b�ո�(�س�)  d����   f(d . e + -)   `(��̷�)   p(>=<) = r(����p) c��ĸ

11 14 9 1 5
b c d f p r = ` #
0 b 0
0 d 1
0 # 10
0 c 3
0 p 5
0 r 8
1 f 1
1 ` 2
3 c 3
3 ` 4
5 = 6
5 ` 7
6 ` 7
8 ` 9
0
2 4 7 9 10
9 18 6 1 1
d . e ` + -
0 + 1
0 - 1
0 d 2
1 d 2
2 d 2
2 ` 8
2 . 3
2 e 5
3 d 4
4 d 4
4 ` 8
4 e 5
5 d 7
5 + 6
5 - 6
6 d 7
7 d 7
7 ` 8
0
8
*/