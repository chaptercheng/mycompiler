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
    string str;//用于动态字符拼接
	string text;//要识别的文本提前加载到内存
	bool isOneScan;//判断是否是一遍扫描
	int curChar;//当前词法分析器正在识别的字母
    void addToken(EnumWord id,int pos)
    {
        Elem t;
        t.kind=id;
        t.addrPtr=pos;
        token.push_back(t);
    }
public:
    vector<Elem> token;//token序列向量
    Scanner(string filename){
		if (!cm.readMachine("constMachine.txt"))//初始化常数处理机
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
	}//初始化扫描器的DFA

    void transfer(int &state,char ch)//在状态state下扫一个字符ch
    {///先在不同状态下根据字符ch的值转化为字母表里的符号表示,再转换
        switch(state)
        {
        case 0://b=空格，d=数字0-9，c=字母，p=符号>、<、=，r表示其他界符
            if(ch==' '||ch=='\t'||ch=='\n')//空白字符处理
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
                    ch='r';//用查找界符表，判断是否为其他界符
            }
            break;
        case 1:
            if((ch>='0' && ch<='9')||ch=='.'||ch=='e'||ch=='+'||ch=='-')
                ch='f';//在状态1下判断是否为常数处理机要处理的字母
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
            ch='`';//在6、8状态下将当前单词为后继符
        if(state==3 || state==11 || state==13 )
        {//在3、11、13状态下查看当前单词是否为合法标识符的内容
            if((ch>='a' && ch<='z')||(ch>='A' && ch<='Z')|| ch=='_'||(ch>='0' && ch<='9'))
                ch='c';
            else if(state==3) ch='`';
        }
        state=df.transfer(state,ch);//状态转移
    }

	bool readText(string filename)
	{
		bool flag = false;
		text = "";
		ifstream inf;//打开要识别的文件
		inf.open(filename);
		if (inf.is_open())
		{
			flag = true;
			string temp;
			while (getline(inf, temp))
				text.append(temp.substr(0, temp.find("//")));//读取文件,并去掉注释
			text.append("#");
		}// if open
		return flag;
	}

	bool isFinal(int state)//判断状态是否为结束状态
	{
		vector<int>::iterator iter = find(df.finl.begin(), df.finl.end(), state);
		return (iter != df.finl.end());
	}
    bool nextToken()//识别单词的一个字母,同时调用stateFun处理
    {
			int state = df.start;//从开始状态出发
			char ch;//当前字母
			while (curChar<int(text.size()) && !isFinal(state))
			{
				ch = text.at(curChar);//获取当前字符
				//printf("%c", ch);
				transfer(state, ch);//查找状态转移表，到达下一个状态
				if(!isFinal(state))
					curChar++;//非结束符则转向下一个字母
				if (state > -1)
					stateFun(ch, state);//扫描一个ch进入状态state的操作
				else
				{
					printf("the scanner can't recognise word,enter state %d\n",state);
					exit(0);
				}
			}//while
			return (state != 10);//nextToken未结束
    }
    void stateFun(char ch,int &state)//动作函数
	{
        int k;
        switch(state)
        {
        case 1://常数串拼接
            str.push_back(ch);
            break;
        case 2://常数处理结束状态
            if((ch>='a' && ch<='z')||(ch>='A' && ch<='Z')|| ch=='_')
            {//遇到错误的后继符错误处理
                cout<<"error when meeting \""<<ch<<"\" in state "<<state<<endl;
                exit(0);
            }
            str.append(";");
            cm.readline(str);//用常数处理机识别该数字
            if(!cm.dou.empty())
            {
                float f=cm.dou.at(0);
                k=FindList(f,FloatList);//查找浮点常数表
                if(k<0)//未找到则插入浮点常数表
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
                k=FindList(it,IntList);//查找整型常数表
                if(k<0)//未找到则插入整型常数表
                {
                    k=IntList.size();
                    IntList.push_back(it);
                }
                addToken(nT,k);
                cm.con.clear();
            }
            str.clear();//str清空
            break;
        case 3://字符串拼接
            str.push_back(ch);
            break;
        case 4://识别标识符结束状态
            k=FindList(str,KeyList);//先查找关键字表
            if(k<0)//没有找到
            {
                k=FindList(TableNode(str),SymbolTable);//查找标识符表
                if(k<0)
                {
                    k=SymbolTable.size();
                    SymbolTable.push_back(TableNode(str));
                }
                addToken(iT,k);
            }
            else//是关键字
            {
                addToken(kT,k);//关键字的token
            }
            str.clear();
            break;
        case 5://> = <
        case 6://=
            str.push_back(ch);
            break;
        case 7://关系符号识别结束状态
            k=FindList(str,PList);
            addToken(pT,k); 
            str.clear();
            break;
        case 8://其他界符
            str.push_back(ch);
            break;
        case 9://其他界符识别结束状态
            k=FindList(str,PList);//查找界符表
            addToken(pT,k);//添加token
            str.clear();
            break;
        case 10://遇见#，退出状态
			curChar++;
			str.push_back('#');
			addToken(pT,FindList(str,PList));
			str.clear();
            break;
        case 11://开始接受字符串
            if(ch!='\"')
                str.push_back(ch);
            break;
        case 12://字符串识别结束
            k=FindList(str,StrList);//查找字符串
            if(k==-1)//没找到则插入字符串表
            {
                k=StrList.size();
                StrList.push_back(str);
            }
            str.clear();
            addToken(sT,k);
            break;
        case 13:
            break;
        case 14://接受字符
            if((ch>='a' && ch<='z')||(ch>='A' && ch<='Z')|| ch=='_')
                str.push_back(ch);
            else {cout<<"字符错误"<<endl;exit(0);}
            break;
        case 15://字符识别结束状态
            k=FindList(str[0],CharList);
            if(k==-1)//没找到
            {
                k=CharList.size();
                CharList.push_back(str[0]);
            }
            str.clear();
            addToken(cT,k);
            break;
        case 16://错误处理状态
            cout<<"扫描字符"<<ch<<"错误"<<endl;
            exit(0);
		case 17:case 18:case 19:case 20:
			str.push_back(ch);
			break;
        }//switch
    }//stateFun

    void showToken()
    {//输出Token序列
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
b空格(回车)  d常数   f(d . e + -)   `(后继符)   p(>=<) = r(其他p) c字母

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