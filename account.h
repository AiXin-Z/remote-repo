#pragma once
#include <QMetaType>
#include<cstring>
#include<string>
#include<map>
#include"date.h"
#include"accumulator.h"
#include <stdexcept>
#include<QString>
#include<QDebug>
#pragma once

//账户种类：储蓄和信用
enum class AccountType
{
    Savings,
    Credit
};

class Account;

class AccountRecord
{
public:
    AccountRecord(const Date date, Account * account, double amount, double balance, const std::string use);
    void show();
    Date date;
    Account *account;
    double amount;
    double balance;
    string use;

};

typedef std::multimap<Date, AccountRecord> RecordMap;//first指Date second指AccountRecord

class Account
{
public:
    Account(Date date, string id);
    void record(Date date, double amount, string use);
    virtual AccountType getType()= 0;
    const string error(string msg);
    const string getId();
    const double getBalance();
    virtual void show() = 0;
    virtual void deposit(Date date, double amount, string use) = 0;
    virtual void withdraw(Date date, double amount, string use) = 0;
    virtual void settle(Date date) = 0;
    static double getTotal();
    string id;
    double balance;
    static double total;
    static void query(Date date1,Date date2);
    static RecordMap recordMap;
    void setBalance(double newBalance)
    {
        balance = newBalance;
    }
};

class CreditAccount:public Account
{
public:
    AccountType getType();
    CreditAccount(Date date, string id, double creidt, double rate, double fee);
    const double getCredit();
    const double getRate();
    const double getFee();
    const double getDebt();
    const double getAvailableCredit();
    void deposit(Date date, double amount, string use);
    void withdraw(Date date, double amount, string use);
    void settle(Date date);
    void show();

private:
    double credit;
    double rate;
    double fee;
    Accumulator acc;
    double debt;
};

class SavingsAccount :public Account
{
public:

    AccountType getType();
    SavingsAccount(Date, string, double);
    const double getRate();
    void deposit(Date date, double amount, string use);
    void withdraw(Date date, double amount, string use);
    void settle(Date date);
    void show();

private:

    Date lastDate;
    double rate;
    double accumulation;
    Accumulator acc;
};

//定义一个处理错误的类，继承于runtime_error
class AccoutException :public runtime_error
{
public:
    AccoutException(Account*account,const string &s):runtime_error(s),account(account)
    {

    }
    Account* getAccount()
    {
        return account;
    }
private:
    Account* account;
};

Q_DECLARE_METATYPE(Account*)




