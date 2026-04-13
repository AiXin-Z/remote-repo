#include "banksystem.h"
#include"user.h"
#include<algorithm>//算法
#include<QDate>//用于处理交易时间
#include <map>
#include<cmath>
#include <QJsonObject>//处理json格式的类
#include <QJsonArray>//json数组
#include <QJsonDocument>//处理json文档，解析json字符串到jsonobject jsonarray
#include <QFile>//文本操作，读写文件
#include <QTextStream>//文本输入输出
#include <qstandarditemmodel.h>//qtableview用到的模型
#include <QObject>
#include<QDebug>

//Lambda 表达式  []捕获列表 ()参数列表 {}函数

BankSystem::BankSystem() {}

//当前时间
Date BankSystem::getCurrentDate()
{
    QDate current = QDate::currentDate();
    return Date(current.year(),current.month(),current.day());
}

//创建新账户
void BankSystem::createAccount(User* user, AccountType type, const string &id, double credit, double rate, double fee)
{
    //用户名或这密码输入不合理
    if (!user || id.empty())
    {
        throw std::invalid_argument("用户或账户ID无效");
    }

    // 检查ID是否已存在
    if (getAccount(user, id))
    {
        throw std::runtime_error("账户ID已存在");
    }

    Date currentDate = getCurrentDate();
    Account* newAccount = nullptr;

    //账户种类
    switch (type)
    {
    case AccountType::Savings:
        newAccount = new SavingsAccount(currentDate, id, credit);
        break;
    case AccountType::Credit:
        newAccount = new CreditAccount(currentDate, id, credit, rate, fee);
        break;
    default:
        throw std::runtime_error("未知账户类型");
    }

    //保存
    user->addAccount(newAccount);
    accounts.push_back(newAccount);

}


//获取账户信息
Account* BankSystem::getAccount(const User* user, const std::string& accountId) const
{
    //如果不是这个用户
    if (!user) return nullptr;

    for (Account* account : accounts)
    {
        if (account && account->getId() == accountId)
        {
            //验证用户是否拥有该账户
            for (Account* userAcc : user->getAccounts())
            {
                if (userAcc == account)
                {
                    return account;
                }
            }
            break;
        }
    }
    return nullptr;
}


//查询记录
void BankSystem::queryRecords(QDate &start,QDate &end)
{
    //利用lower_bound和upper_bound可以查询一段时间内的账目记录
    RecordMap::iterator it1 = recordMap.lower_bound(start);
    RecordMap::iterator it2 = recordMap.upper_bound(end);

    for (RecordMap::iterator it = it1; it != it2; ++it)
    {
        it->second.show();//输出AccountRecord的信息
    }
}

//排序账户
void BankSystem::sortAccounts(SortType type)
{
    //按ID排序
    switch (type)
    {
    case SortType::SortById:
        sort(accounts.begin(), accounts.end(),
             [](Account* a, Account* b)
             {
                 return a->getId() < b->getId();
             });
        break;

    //按余额排序
    case SortType::SortByBalance:
        sort(accounts.begin(), accounts.end(),
             [](Account* a, Account* b)
             {
                 return a->getBalance() < b->getBalance();
             });
        break;

    //按信用账户的可用信用额度排序
    case SortType::SortByAvailableCredit:
        sort(accounts.begin(), accounts.end(),
             [](Account* a, Account* b)
             {
                //只有信用账户才有，先将*Account转换成*CreditAccount类
                auto ca = dynamic_cast<CreditAccount*>(a);
                auto cb = dynamic_cast<CreditAccount*>(b);
                if (ca && cb)//如果两个的账户都是信用账户类型排序，不是返回false
                {
                    return ca->getAvailableCredit() < cb->getAvailableCredit();
                }
                return false;
            });
        break;

    default:
        break;
    }
}

//创建信用账户
bool BankSystem::createCreditAccount(const QString &id, double creditLimit)
{
    if (id.isEmpty() || creditLimit < 1000) return false;
    return true;
}

//创建储蓄账户
bool BankSystem::createSavingsAccount(const QString &id)
{
    if (id.isEmpty()) return false;
    return true;
}

//存款
void BankSystem::deposit(User* user, const std::string& accountId, double amount)
{
    //错误提示
    if (!user || amount <= 0)
    {
        throw std::invalid_argument("错误的用户信息或账户");
    }

    //获取账户信息
    Account* account = getAccount(user,accountId);
    if (!account)
    {
        throw std::runtime_error("未找到账户");
    }

    bool ownsAccount = false;

    //验证账户
    for (Account* userAccount : user->getAccounts())
    {
        if (userAccount->getId() == accountId)
        {
            ownsAccount = true;
            break;
        }
    }

    //检查用户是否拥有这个账户
    if (!ownsAccount)
    {
        throw std::runtime_error("这不是您的账户");
    }

    //记录当前日期
    Date currentDate = getCurrentDate();
    account->deposit(currentDate, amount, "存款");
}

//取款
void BankSystem::withdraw(User* user, const std::string& accountId, double amount)
{
    if (!user || amount <= 0)
    {
        throw std::invalid_argument("错误的用户信息或账户");
    }

    Account* account = getAccount(user, accountId);

    if (!account)
    {
        throw std::runtime_error("未找到账户");
    }

    //检查用户是否拥有该账户
    bool ownsAccount = false;
    for (Account* userAccount : user->getAccounts())
    {
        if (userAccount->getId() == accountId)
        {
            ownsAccount = true;
            break;
        }
    }

    if (!ownsAccount)
    {
        throw std::runtime_error("这不是您的账户");
    }

    CreditAccount* creditAccount = dynamic_cast<CreditAccount*>(account);

    if (creditAccount)
    {
        double balance = creditAccount->getBalance();
        double availableCredit = creditAccount->getAvailableCredit();

        // qDebug() << "账户余额: " << balance
        //          << ", 可用额度: " << availableCredit
        //          << ", 取款金额: " << amount;

        //如果是信用账户，检查是否有足够的额度

        //case1:账户余额足够，可以取款
        if (balance >= amount)
        {

        }
        //case2:账户余额不够,但可用额度够，也能取款
        else if (balance + availableCredit >= amount)
        {

        }
        //case3:账户金额和可用额度都不够,不能取款,提示用户信用额度不足
        else
        {
            throw std::runtime_error("信用额度不足");
        }
    }
    else
    {
        //储蓄账户判断余额
        if (account->getBalance() < amount)
        {
            throw std::runtime_error("余额不足");
        }
    }

    //获取现在的日期
    Date currentDate = getCurrentDate();

    account->withdraw(currentDate, amount, "取款");
}


//添加账户
void BankSystem::addAccount(Account* account)
{
    accounts.push_back(account);
}
