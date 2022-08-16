#include <iostream>
#include "LRAnalyzer.h"
#include "Scanner.h"
#include "SLRGen.h"
using namespace std;

int main()
{
    ///先从文件读取表达式
    Scanner scan;//定义单词扫描器对象
    ifstream in;
    in.open("text.txt");//打开待识别单词源文件
    if(in.is_open())
    {

        string line,total="";
        while(getline(in,line))//按行读取文件内容
        {
            total.append(line);
        }
        total.append("#");
        scan.recogniseLex(total);//进行词法分析
        in.close();
    }
    else//打开源单词文件失败
    {
        cout<<"can't open file text.txt!"<<endl;
        exit(0);
    }
    scan.showToken();//显示token序列

    getGrammer();/*
    SLRAnalyzer lr(scan);///LR分析法
    if(lr.initChart("lrChart.txt"))//读取LR分析表
    {
        if(lr.analyze())//LR语法分析
            cout<<"right"<<endl;
        else cout<<"not acc"<<endl;
    }
    else cout<<"open Error";*/
    return 0;
}
/*
7
E->S
S->aA
S->bB
A->bB
A->a
B->aB
B->b
*/
