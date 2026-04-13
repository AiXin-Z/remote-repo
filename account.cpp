#include "account.h"
#include"date.h"
#include<cmath>
#include<cstring>
#include<iostream>

//（1）添加AccountRecord类描述账目
//（2）添加一个新容器recordMap来保存所有账目。
//将recordMap定义为multimap类型，其中键（key）是账目的日期，数据（value）是账目的详细内容。参考如下定义：
//在Account类中，将recordMap定义为静态数据成员
//（3）修改Account类中的record函数，每当Account的record函数被调用时，向recordMap中添加一条新账目
//（4）在Account类中定义新的函数query()，用于查询从day1到day2的历史账目。

AccountRecord::AccountRecord(Date date, Account* account, double amount, double balance, string use)
    :date(date), account(account), amount(amount), balance(balance), use(use)
{

}
void AccountRecord::show()
{
    date.show();
    cout<< "\t#" << account->id << "\t" << amount << "\t" << balance << "\t" << use << endl;//输出余额
}

RecordMap Account::recordMap;

//account构造函数
Account::Account(Date pdate, string pid):id(pid),balance(0)
{

}

//记录账户信息
void Account::record(Date date, double amount, string use)
{
    //抛出错误
    try
    {
        amount = floor(amount * 100 + 0.5) / 100;
        balance += amount;//余额加上新存的钱数
        total += amount;//总钱数加上新存的钱数
        AccountRecord record(date, this, amount, balance, use);//创建AccountRecord对象 表示交易记录
        recordMap.insert(make_pair(date, record));//按照日期排序记录所有的交易记录
        record.show();
    }

    catch (const std::exception& e)
    {
        throw AccoutException(this, "Record error" + string(e.what()));
    }

}

//如果出现错误
const string Account::error(string msg)
{
    return msg;
}

const string Account::getId()
{
    return id;
}

//获取余额 此时的余额是可正可负的
const double Account::getBalance()
{
    return balance;
}

//获取总钱数
double Account::getTotal()
{
    return total;
}

void Account::query(Date date1,Date date2)
{
    //利用lower_bound和upper_bound可以查询一段时间内的账目记录
    //lower_bound返回第一个>=指定元素的值，upper_bound是返回第一个>的
    //用RecordMap存储
    RecordMap::iterator it1 = recordMap.lower_bound(date1);
    RecordMap::iterator it2 = recordMap.upper_bound(date2);
    for (RecordMap::iterator it = it1; it != it2; ++it)
    {
        it->second.show();//输出AccountRecord的信息
    }

}

AccountType CreditAccount::getType(){ return AccountType::Credit; }

CreditAccount::CreditAccount(Date date, string id, double credit, double rate, double fee)
    :Account(date,id), credit(credit),rate(rate),fee(fee),acc(date,0)
{
    cout << date.year << "-" << date.month << "-" << date.day
         << "\t#" << id << " created" << endl;
}

//信用额度（credit） : 欠款 <= 信用额度
const double CreditAccount::getCredit()
{
    return credit;
}

//欠款 余额为负时要付利息
const double CreditAccount::getRate()
{
    return rate;
}

//年费（fee）：每年需要交一次年费（每年1月1日扣年费）
const double CreditAccount::getFee()
{
    return fee;
}

const double CreditAccount::getDebt()//欠款
{
    //return -rate-balance;//余额为负
    //如果余额为负则return -balance
    //为正说明无欠款 return 0
    return (balance < 0) ? -balance : 0;
}

const double CreditAccount::getAvailableCredit()
{
    //如果余额>0返回信用额度 如果余额<0 返回信用额度+余额 因为余额是负数
    return (balance >= 0) ? credit : credit + balance;
}

void CreditAccount::deposit(Date date, double amount, string use)//存款
{
    record(date, amount, use);//调用record函数输出记录

    if (balance > 0)
    {
        debt = 0;
    }

    acc.change(date, getDebt());//change函数更新日期
}

void CreditAccount::withdraw(Date date, double amount, string use)
{
    //获取金额和可用额度
    double balance = getBalance();
    double availableCredit = getAvailableCredit();

    //实际可用金额=账户余额可用额度
    double totalAvailable = balance + availableCredit;

    //完善无法取款的情况
    //检查是否有足够的可用信用额度
    if (amount > totalAvailable)
    {
        //此时要抛出错误信息
        throw AccoutException(this, "not enough credit " + id);
    }

    try
    {
        if (amount <= balance)
        {
            //记录取款操作，取款zmount为负
            record(date, -amount, use);
        }
        else
        {
            //如果取款金额大于余额，则需要从债务中扣除超出部分
            double excessAmount = amount - balance;
            //扩展债务
            debt += excessAmount;
            //取款是-amount
            record(date, -amount, use);
        }
        //更新日期并获取欠的债务
        acc.change(date, getDebt());
    }

    catch (const std::exception& e)
    {
        throw AccoutException(this, "can not withdraw " + id + ": " + e.what());
    }
}


void CreditAccount::settle(Date date)//结算利息，每月1日结算
{
    if (date.day == 1)//如果是1日
    {
        double interest = acc.getSum(date, false) * rate;//总钱数*利率
        interest = floor(interest * 100 + 0.5) / 100;
        if (interest != 0)//如果有欠款要付利息
        {
            record(date, -interest, "interest");//因为是要扣钱所以为-interest
        }
        if (date.month == 1)//一月一号要收年费
        {
            record(date, -fee, "annual fee");//也是扣钱所以-fee
        }
        acc.reset(date, getDebt());//结算之后要重置
    }
}

void CreditAccount::show()
{
    cout << id << "\tBalance: " << balance << "\tAvailable credit:" << getAvailableCredit();
}


AccountType SavingsAccount::getType()
{ return AccountType::Savings; }

SavingsAccount::SavingsAccount(Date pdate, string pid, double prate):Account(pdate,pid), lastDate(pdate),rate(prate), accumulation(0),acc(pdate,0)
{
    cout << pdate.year << "-" << pdate.month << "-" << pdate.day
         << "\t#" << id << " created" << endl;
}


const double SavingsAccount::getRate()
{
    return rate;
}

void SavingsAccount::show()//账户信息
{
    cout << id << "\tBalance: " << balance;
}

void SavingsAccount::deposit(Date date, double amount,string use)//存款
{

    record(date, amount, use);
    acc.change(date, balance);
}

void SavingsAccount::withdraw(Date date, double amount,string use)//取款
{
    record(date, -amount, use);
    acc.change(date, balance);

}

void SavingsAccount::settle(Date date)//结算利息
{

    if (date.month == 1)//1月1日结算
    {
        double interest = acc.getSum(date, false) * rate / 366;//储蓄账户的利息等于总钱数*利息/366
        if (interest != 0)
        {
            record(date, interest, "interest");//输出利息 储蓄账户的利息是赚的钱，要用+interest
            acc.reset(date, balance);//结算后要重置
        }
    }

}


double Account::total = 0;
