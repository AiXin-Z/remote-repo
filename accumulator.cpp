#include"accumulator.h"
#include"date.h"
#include"account.h"
//步骤2：
//添加Accumulator类，来描述两类账户在计算利息时的共性：都需要将某个数值（余额或欠款）按日累加
//Accumulator类提供计算按日累加和的基本操作
//两个派生类中包含Accumulator类型的数据成员，分别按两类账户的特点将该成员实例化并计算利息。
Accumulator::Accumulator(Date date, double value) :LastDate(date), value(value),sum(0)
{

}
//累加
const double Accumulator::getSum(Date date,bool skip)
{
	return sum + value * date.dateDif(LastDate,date,skip);//总累计金额=已累计的金额+本金*天数差
}

//change函数来更新最新的日期，相当于银行01中用的lastdate，还有累加总钱数
//当存款或者取款之后 结算利息时调用
void Accumulator::change(Date date, double value)
{
	sum = getSum(date,false);//累计金额到改变的前一天
	LastDate = date;//更新日期
	this->value = value;//更新金额
}

//重置，作用相当于银行01中的accumulation=0
void Accumulator::reset(Date date, double value)
{
	LastDate = date;//更新日期
	this->value = value;//新的金额
	sum = 0;//当利息结算之后重置sum=0
}

