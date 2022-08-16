#include <iostream>
#include "SLRAnalyzer.h"
#include "Scanner.h"
#include "ListGenerator.h"
#include "Grammer.h"
#include "DesCode.h"
#include "DagOptimal.h"
#include <sstream>

void test_scanner() {
	Scanner scan("text.txt");
	//scan.df.showDFA();
	while (scan.nextToken())
	{
		cout << scan.token.size() << endl;
	}
}

int main() {
	Grammer gram;
	Scanner scan("text.txt");
	if (gram.readGrammer("MyGram.txt"))
	{
		//gram.showGenList();//文法产生式
		ListGenerator lg(&gram);//通过给定文法生成SLR分析表
		//ShowList();
		lg.traverse();//项目集合的遍历，生成SLR分析表的关键函数
		//lg.showCluster();
		printf("cluster size:%d\n", lg.cluster.size());
		///单词识别、语法语义分析、四元式Quarts生成
		SLRAnalyzer slr(&scan, &lg);//给定扫描器和分析表做SLR分析
		if (slr.analyze())
		{
			scan.showToken();//分析完之后生成token序列
			cout << "sentence are accepted!" << endl;
		}
		else 
			cout << "wrong! this sentence not match the grammer." << endl;
		//lg.showItemSet();
	///语义及语法分析结果显示
	printf("\t\t语义分析产生的源四元\n");
	
	
	cout << "SEM:" << SEM.size()
		<< " OP:" << slr.opStk.size()
		<< " TypeStk:" << TypeStk.size() << endl;
	}
	else cout << "can't open the grammer file." << endl;
	ShowQuarts(0, Quarts.size() - 1, Quarts);//源四元式
	ShowSymbolTable();//符号表相关

	//四元式优化模块，注意优化之后segment的内容也需要相应改变
	vector<int> srcSeg;//源四元式的基本块的分块信息
	vector<int> optmSeg;//优化之后的基本块信息
	DesCoder dc(&Quarts);
	GetBlockSeg(Quarts, srcSeg);//源四元式划分基本块
	dc.labelL(srcSeg);
	dc.beginDC(srcSeg);//初始化并生成目标代码
	ShowQuadBlock(srcSeg, Quarts);

	printf("\t\t目标代码\n");
	
	dc.showDesCode();

	return 0;
}