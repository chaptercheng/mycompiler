#ifndef CONST_MACHINE_H
#define CONST_MACHINE_H
#include "DFA.h"
#include <cstdlib>
#include <math.h>
#include <vector>

using namespace std;

///实现常数的识别，包括整型常数和浮点型常数

class ConstMachine
{
private:
	bool isInit;
	DFA df;//常数识别自动机
	int n, p, m, sn, sp, t;//底数数值n,指数数值p,小数位数m，数值正负sn，指数正负sp，类型判断t(0为int,1为float)
public:
	vector<int> con;//整型长量表
	vector<float> dou;//浮点常数表
	ConstMachine() { isInit = false; }
	int value(char ch)
	{
		return (ch - '0');
	}
	void stateConFun(char ch, int state) //状态动作函数
	{
		switch (state)//state从0开始算
		{
		case 0:
			n = p = m = t = 0;//默认整型
			sp = sn = 1;//默认为正数
			break;
		case 1:
			if (ch == '-')//是负数
				sn = -1;
			else
				sn = 1;//是正数
			break;
		case 2:
			n = n * 10 + value(ch);//计算底数
			break;
		case 3:
			t = 1;//小数
			break;
		case 4:
			m++;//记录小数点位数
			n = n * 10 + value(ch);
			break;
		case 5:
			t = 1;//有指数即为浮点数
			break;
		case 6://指数正负标志
			if (ch == '-')
				sp = -1;
			else
				sp = 1;
			break;
		case 7://计算指数的值
			p = p * 10 + value(ch);
			break;
		default:
			if (t == 0)//整数
				con.push_back(sn*n);
			else//算出浮点数
			{
				float num = 1.0;
				for (int i = 0; i < abs(sp*p - m); i++)
				{
					if ((sp*p - m) > 0)
						num *= 10;
					else
						num /= 10;
				}
				num = sn * n*num;//计算最后的浮点数的值
				dou.push_back(num);
			}
			break;
		}//switch
	}
	bool readMachine(string filename)//读取浮点常数处理机的DFA
	{
		isInit = true;
		return df.readDFA(filename);
	}
	void recogniseCon(char &ch, int &state)
	{
		state = df.transfer(state, ch);
		if (state < 0)
		{
			stateConFun('`', state);
			state = df.start;
			stateConFun(' ', state);
		}
		else//字母表内的
			stateConFun(ch, state);
	}
	void showCon()//整型常数表显示
	{
		cout << "constant:";
		for (int i = 0; i<int(con.size()); i++)
			cout << con.at(i) << " ";
		cout << endl;
	}
	void readline(string str)//识别待识别的数字字符串
	{
		int state = df.start;
		stateConFun(' ', state);//进入开始状态
		for (int i = 0; i<int(str.size()); i++)
			recogniseCon(str.at(i), state);
	}
	void showDou()//显示浮点常数表
	{
		cout << "double:";
		for (int i = 0; i<int(dou.size()); i++)
			cout << dou.at(i) << " ";
		cout << endl;
	}
};
#endif // CONST_MACHINE_H