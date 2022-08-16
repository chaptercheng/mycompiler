#include <iostream>
#include "LRAnalyzer.h"
#include "Scanner.h"
#include "SLRGen.h"
using namespace std;

int main()
{
    ///�ȴ��ļ���ȡ���ʽ
    Scanner scan;//���嵥��ɨ��������
    ifstream in;
    in.open("text.txt");//�򿪴�ʶ�𵥴�Դ�ļ�
    if(in.is_open())
    {

        string line,total="";
        while(getline(in,line))//���ж�ȡ�ļ�����
        {
            total.append(line);
        }
        total.append("#");
        scan.recogniseLex(total);//���дʷ�����
        in.close();
    }
    else//��Դ�����ļ�ʧ��
    {
        cout<<"can't open file text.txt!"<<endl;
        exit(0);
    }
    scan.showToken();//��ʾtoken����

    getGrammer();/*
    SLRAnalyzer lr(scan);///LR������
    if(lr.initChart("lrChart.txt"))//��ȡLR������
    {
        if(lr.analyze())//LR�﷨����
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
