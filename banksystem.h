#ifndef BANKSYSTEM_H
#define BANKSYSTEM_H

class User;//防止和user的头文件相互包含

#include <qstandarditemmodel.h>
#include <QObject>
#include<vector>
#include<cmath>
#include<map>
#include<QDate>
#include"account.h"


//2.每个用户登陆后可以创建自己的两类账户，并进行操作，完成3个月内的操作记录。
//4.可以查询某个月按照时间排序的账户查询信息
//5.可以查询某个月按照交易金额从大到小排序的账户查询信息
#pragma once

enum class SortType
{
    SortById,//按账户ID排序
    SortByBalance,//按余额排序
    SortByAvailableCredit//按可用信用额度排序
};


class BankSystem
{

public:
    BankSystem();

    ~BankSystem();

    //创建账户
    void createAccount(User* user,AccountType type,const string &id,double credit,double rate,double fee);
    bool createSavingsAccount(const QString &id);
    bool createCreditAccount(const QString &id, double creditLimit);

    //获取账户
    Account* getAccount(const User* user, const std::string& accountId) const;

    //存款
    void deposit(User* user, const std::string& accountId, double amount);

    //取款
    void withdraw(User* user, const std::string& accountId, double amount);

    //增加查询功能
    void queryRecords(QDate &start,QDate &end);

    //增加排序功能
    void sortAccounts(SortType type);

    //改变日期
    void changeDate(Date&newDate);
    void nextmonth();

    //获取现有的数据
    double getTotal();
    Date getCurrentDate();

    void addAccount(Account* account);


private:

    Date currentDate;
    using RecordMap = std::map<QDate, AccountRecord>;

    //账户存在
    bool accountExists(const std::string& id) const;

    //添加交易记录
    void addTransactionRecord(const AccountRecord& record);

    //数组存储所有账户
    std::vector<Account*> accounts;

    //交易记录按日期存储
    std::map<QDate, AccountRecord> recordMap;


};


#endif // BANKSYSTEM_H





