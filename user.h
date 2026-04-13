#ifndef USER_H
#define USER_H

class BankSystem;

#include <string>
#include <vector>
#include "account.h"
#include <fstream>
#include <sstream>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

//emplace_back() 在容器末尾就地构造一个对象


//新建用户类
class User
{
public:
    User();
    User(const std::string& username, const std::string& password);

    //验证用户名密码是否输入正确
    bool authenticate(const std::string& inputPassword) const;

    //添加账户
    void addAccount(Account* account);
    const vector<Account*>& getAccounts()const;

    //添加消息通知
    void addNotification(string& message);

    //获取消息
    vector<string>& getNotifications();

    //清空消息
    void clearNotifications();

    //获取用户名
    string getUsername()const;

    string username;
    string password;
    vector<Account*> accounts;
    vector<string> notifications;


};

//管理用户类
class UserManager
{
public:

    UserManager()
    {
        //先加载记录在文件里注册过的用户信息(用户名和密码)
        loadUsersFromFile();
    }

    //用户是否存在
    bool hasUsers() const
    {
        //返回true
        return !users.empty();
    }

    //用户的注册
    //检查用户名：不能重复
    bool registerUser(const std::string& username, const std::string& password)
    {
        //检查用户名是否已存在
        //语法:遍历整个user的容器,引用将用户名的变量传给匿名函数
        auto it = std::find_if(users.begin(), users.end(),
                               [&username](const User& user)
                               {
                                   return user.getUsername() == username;
                               });

        //如果找到了就不再注册了
        if (it != users.end())
        {
            return false;
        }

        //保存新用户信息到文本
        users.emplace_back(username, password);
        saveUsersToFile();
        //注册成功
        return true;
    }

    //验证用户名和密码
    User* authenticate(const std::string& username, const std::string& password)
    {
        //验证用户名和密码是否都匹配
        for (auto& user : users)
        {
            if (user.getUsername() == username && user.authenticate(password))
            {
                return &user;
            }
        }
        return nullptr;
    }


    //保存用户账户数据
    bool saveUserAccounts(User* user)
    {
        QJsonObject root;
        QJsonArray accountsArray;

        //遍历用户拥有的所有账户,将id 类型 余额写入json
        for (Account* account : user->getAccounts())
        {
            QJsonObject accObj;
            //转化QString
            accObj["id"] = QString::fromStdString(account->getId());
            accObj["type"] = (account->getType() == AccountType::Savings) ? "savings" : "credit";
            accObj["balance"] = account->getBalance();

            //转化信用账户类型 将信用额度 利率 年费等写入json
            if (account->getType() == AccountType::Credit)
            {
                CreditAccount* creditAcc = dynamic_cast<CreditAccount*>(account);
                accObj["creditLimit"] = creditAcc->getCredit();
                accObj["interestRate"] = creditAcc->getRate();
                accObj["annualFee"] = creditAcc->getFee();
            }

            //转化储蓄账户类型 将利率写入json
            if (account->getType() == AccountType::Savings)
            {
                SavingsAccount* savingsAcc = dynamic_cast<SavingsAccount*>(account);
                if (savingsAcc)
                {
                    accObj["interestRate"] = savingsAcc->getRate();
                }
            }

            //保存交易记录
            QJsonArray recordsArray;
            for (const auto& recordPair : Account::recordMap)
            {
                if (recordPair.second.account->getId() == account->getId())
                {
                    QJsonObject recordObj;
                    //日期为 年-月-日 取出交易记录中的amount赋值给json对象的amountkey
                    recordObj["date"] = QString("%1-%2-%3")
                                            .arg(recordPair.first.year)
                                            .arg(recordPair.first.month)
                                            .arg(recordPair.first.day);
                    recordObj["amount"] = recordPair.second.amount;
                    recordObj["use"] = QString::fromStdString(recordPair.second.use);
                    recordsArray.append(recordObj);
                }
            }
            //把交易记录放入账户对象
            accObj["records"] = recordsArray;

            //再把账户放进整个数组
            accountsArray.append(accObj);
        }

        root["accounts"] = accountsArray;

        //保存到json文件,以为用户名来命名文件
        QFile file(QString("user_%1.json").arg(QString::fromStdString(user->getUsername())));
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(QJsonDocument(root).toJson());
            file.close();
            return true;
        }
        return false;
    }


    //保存信息到文件
    bool saveUsersToFile()
    {
        //ofstream写文件
        std::ofstream outFile(userDataFile);
        //打开文件失败
        if (!outFile)
        {
            return false;
        }
        //读取数据
        for (const auto& user : users)
        {
            //遍历users并保存相应的用户名和密码到文件
            outFile << user.getUsername() << "," << user.password << "\n";
        }

        return true;
    }

    //加载信息
    bool loadUsersFromFile()
    {
        //ifstream读文件
        std::ifstream inFile(userDataFile);

        if (!inFile)
        {
            return false;
        }

        users.clear();
        std::string line;
        //创建user保存用户名和密码并添加到users里
        //读取 拆分 保存
        while (std::getline(inFile, line))
        {
            //依次读取字符串 用户名,密码  ,之前部分存入username 后面存入password
            std::istringstream iss(line);
            std::string username, password;
            if (std::getline(iss, username, ',') && std::getline(iss, password))
            {
                //添加到user里
                users.emplace_back(username, password);
            }
        }
        return true;
    }

    bool loadUserAccounts(User* user, BankSystem* system);


private:
    std::vector<User> users;
    //将用户数据存储到文件
    const std::string userDataFile = "users.dat";
};
#endif // USER_H




