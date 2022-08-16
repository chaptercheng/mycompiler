#ifndef CONST_MACHINE_H
#define CONST_MACHINE_H
#include "DFA.h"
#include <cstdlib>
#include <math.h>
#include <vector>

using namespace std;

///ʵ�ֳ�����ʶ�𣬰������ͳ����͸����ͳ���

class ConstMachine
{
private:
	bool isInit;
	DFA df;//����ʶ���Զ���
	int n, p, m, sn, sp, t;//������ֵn,ָ����ֵp,С��λ��m����ֵ����sn��ָ������sp�������ж�t(0Ϊint,1Ϊfloat)
public:
	vector<int> con;//���ͳ�����
	vector<float> dou;//���㳣����
	ConstMachine() { isInit = false; }
	int value(char ch)
	{
		return (ch - '0');
	}
	void stateConFun(char ch, int state) //״̬��������
	{
		switch (state)//state��0��ʼ��
		{
		case 0:
			n = p = m = t = 0;//Ĭ������
			sp = sn = 1;//Ĭ��Ϊ����
			break;
		case 1:
			if (ch == '-')//�Ǹ���
				sn = -1;
			else
				sn = 1;//������
			break;
		case 2:
			n = n * 10 + value(ch);//�������
			break;
		case 3:
			t = 1;//С��
			break;
		case 4:
			m++;//��¼С����λ��
			n = n * 10 + value(ch);
			break;
		case 5:
			t = 1;//��ָ����Ϊ������
			break;
		case 6://ָ��������־
			if (ch == '-')
				sp = -1;
			else
				sp = 1;
			break;
		case 7://����ָ����ֵ
			p = p * 10 + value(ch);
			break;
		default:
			if (t == 0)//����
				con.push_back(sn*n);
			else//���������
			{
				float num = 1.0;
				for (int i = 0; i < abs(sp*p - m); i++)
				{
					if ((sp*p - m) > 0)
						num *= 10;
					else
						num /= 10;
				}
				num = sn * n*num;//�������ĸ�������ֵ
				dou.push_back(num);
			}
			break;
		}//switch
	}
	bool readMachine(string filename)//��ȡ���㳣���������DFA
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
		else//��ĸ���ڵ�
			stateConFun(ch, state);
	}
	void showCon()//���ͳ�������ʾ
	{
		cout << "constant:";
		for (int i = 0; i<int(con.size()); i++)
			cout << con.at(i) << " ";
		cout << endl;
	}
	void readline(string str)//ʶ���ʶ��������ַ���
	{
		int state = df.start;
		stateConFun(' ', state);//���뿪ʼ״̬
		for (int i = 0; i<int(str.size()); i++)
			recogniseCon(str.at(i), state);
	}
	void showDou()//��ʾ���㳣����
	{
		cout << "double:";
		for (int i = 0; i<int(dou.size()); i++)
			cout << dou.at(i) << " ";
		cout << endl;
	}
};
#endif // CONST_MACHINE_H