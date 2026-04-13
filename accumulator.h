#pragma once
//#include<iostream>
#include<Qstring>
#include<cstring>
#include"date.h"
using namespace std;

class Accumulator
{
public:
	Accumulator(Date date, double value);
	const double getSum(Date date, bool skip);
	void change(Date date, double value);
	void reset(Date date, double value);

private:
	Date LastDate;//上一次更新的日期
	double value;//本金或者是欠款
	double sum;//已累计的金额*天数
};
