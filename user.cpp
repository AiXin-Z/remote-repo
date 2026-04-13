#include "user.h"
#include "banksystem.h"
#include "global.h"

User::User() {}

User::User(const std::string& username, const std::string& password)
    : username(username), password(password) {}

//验证密码输入
bool User::authenticate(const std::string& inputPassword) const
{
    //对于输入的密码的格式要做统一，要不然就会莫名的识别不进去，登陆不上
    std::string transformedPassword = password;
    std::transform(transformedPassword.begin(), transformedPassword.end(), transformedPassword.begin(), ::tolower);
    std::string transformedInputPassword = inputPassword;
    std::transform(transformedInputPassword.begin(), transformedInputPassword.end(), transformedInputPassword.begin(), ::tolower);

    return password == inputPassword;
}

void User::addAccount(Account* account)
{
    if (!account)
    {
        return;
    }

    //检查账户是否已经存在,防止重复添加
    for (Account* acc : accounts)
    {
        if (acc && acc->getId() == account->getId())
        {
            return;
        }
    }

    accounts.push_back(account);
}


//获取账户
const vector<Account*>& User::getAccounts() const
{
    return accounts;
}

//添加通知
void User::addNotification(string& message)
{
    notifications.push_back(message);
}

//获取通知
vector<std::string>& User::getNotifications()
{
    return notifications;
}

//清空通知
void User::clearNotifications()
{
    notifications.clear();
}

//获取用户名
string User::getUsername()const
{
    return username;
}

//加载用户账户数据
bool UserManager::loadUserAccounts(User* user, BankSystem* system)
{
    //打开文件 读取文件
    QFile file(QString("user_%1.json").arg(QString::fromStdString(user->getUsername())));
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return false;

    //读取json文档
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    //解析账户数据数组
    QJsonObject root = doc.object();
    QJsonArray accountsArray = root["accounts"].toArray();

    //遍历账户数组
    for (const QJsonValue& accValue : accountsArray)
    {
        QJsonObject accObj = accValue.toObject();
        std::string id = accObj["id"].toString().toStdString();
        AccountType type = (accObj["type"].toString() == "savings") ? AccountType::Savings : AccountType::Credit;
        double balance = accObj["balance"].toDouble();

        Account* account = nullptr;

        if (type == AccountType::Savings)
        {
            double interestRate = accObj["interestRate"].toDouble();
            account = new SavingsAccount(Date(), id, interestRate);
        }
        else
        {
            double creditLimit = accObj["creditLimit"].toDouble();
            double interestRate = accObj["interestRate"].toDouble();
            double annualFee = accObj["annualFee"].toDouble();
            account = new CreditAccount(Date(), id, creditLimit, interestRate, annualFee);
        }

        //set余额
        account->setBalance(balance);

        //把账户加到系统
        user->addAccount(account);
        if (system)
        {
            system->addAccount(account);
        }

        //加载交易记录
        //每个账户都有一个数组
        QJsonArray recordsArray = accObj["records"].toArray();
        for (const QJsonValue& recordValue : recordsArray)
        {
            //recordValue转换为json对象
            QJsonObject recordObj = recordValue.toObject();
            QStringList dateParts = recordObj["date"].toString().split("-");
            //日期格式错误跳过
            if (dateParts.size() != 3) continue;

            //日期对象 0年 1月 2日 日期是字符串类型 转化int
            Date date
                {
                dateParts[0].toInt(),
                dateParts[1].toInt(),
                dateParts[2].toInt()
                };

            //获取金额 用途
            double amount = recordObj["amount"].toDouble();
            std::string use = recordObj["use"].toString().toStdString();

            //添加这条记录到recordMap(key是日期 value是AccountRecord)
            Account::recordMap.emplace(date, AccountRecord(date, account, amount, account->getBalance(), use));
        }
    }

    return true;
}

