#include "mainwindow.h"
#include<QMessageBox>
#include "./ui_mainwindow.h"
#include "createaccount.h"
#include<QDebug>
#include<QTimer>
#include <QTableWidgetItem>//QTableWidget表格的单元格头文件
#include<QStandardItemModel>//QTableView用到的model
#include <QJsonObject>//json对象结构，解析json的账户信息和交易记录等等
#include <QJsonArray>//json数组，用来存储账户列表和交易记录表
#include <QJsonDocument>//json文档,用于json格式读和写
#include <QFile>//读取或写入用户的信息到data文件里
#include <QTextStream>//qt的文件流操作
#include <set>//set集合,交易记录去重加的
#include "global.h"

//QT
//1.弹出的对话框类型
//QMessageBox::information  信息提示
//QMessageBox::warning  警告
//QMessageBox::critical  严重错误
//QMessageBox::question  问答选择
//this,"窗口标题","提示的具体内容"
//2.QVariant
//将任意类型的数据包装成QVariant对象
//v.toType() 取出原始数据
//如果存的是一个自定义类型或指针qvariant_cast


//任务
//1.完成用户注册登录界面，并存储账户信息（已完成）
//2.完成用户对帐户类型的选择（已完成）
//3.完成用户存款和取款的操作（已完成）
//4.完成对一段时间内交易记录的排序和查询（已完成）
//5.完成欠款信息的提示（已完成）

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始银行系统
    bankSystem = new BankSystem();

    //刷新账户下拉框
    if (currentUser)
    {
        refreshAccountList();
    }

    //设置账户类型
    qRegisterMetaType<Account*>("Account*");

    //连接创建账户按钮的信号和槽
    //格式:信号发出者 信号函数指针 信号接收者 槽函数响应信号 点击账户选择下拉框

    //1.用户在 QComboBox 中选择了一个新项
    //2.QComboBox 发出信号 currentIndexChanged(int)
    //3.connect 建立的连接捕捉到信号
    //4.调用 MainWindow::onAccountSelected(int index)
    connect(ui->accountComboBox, &QComboBox::currentIndexChanged,this, &MainWindow::onAccountSelected);

    //connect(ui->accountComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAccountSelected(int)));

    //日期设置成当前时间
    QDate today = QDate::currentDate();

    //查询时设置起始和结束日期
    ui->startdateEdit->setDate(QDate(today.year(), today.month(), 1));
    ui->enddateEdit->setDate(today);

    //余额为：
    ui->balanceLabel->setText("余额：");

    //connect(ui->accountComboBox, SIGNAL(activated(int)), this, SLOT(on_accountComboBox_activated(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//用槽函数来更新表格
void MainWindow::updateTransactionTable(QStandardItemModel* model)
{
    //告诉QTableView视图现在的数据内容是这个model
    ui->tableView->setModel(model);
    //获取列标题,自动调整列宽  Stretch是平均拉伸
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//登陆成功后，要刷新下拉框，使得下拉框可以显示用户之前创建过的账户
//还得清空旧记录，要不然不能正确显示
//如果信用账户有欠款，在登录成功之后要向他们发送提示信息提醒还款
void MainWindow::onLoginSuccessful(User* user)
{
    currentUser = user;

    //清空旧记录，防止查询重复
    Account::recordMap.clear();

    //加载用户信息,之前创建的账户和相应的金额,刷新下拉框,显示用户拥有的所有账户
    if (userManager.loadUserAccounts(currentUser, bankSystem))
    {
        refreshAccountList();
    }
    //如果用户是新注册的没有账户,提示用户还没有账户,要先创建
    else
    {
        QMessageBox::warning(this, "提示", "账户加载失败，您还未创建账户，请先创建账户");
        return;
    }

    //检查信用账户欠款并提示还款
    bool hasDebt = false;
    double totalDebt = 0.0;

    //遍历有账户，获取账户类型，只有信用账户才有，检查有无欠款，有欠款的余额为负数，这时发送提示信息
    for (Account* account : currentUser->getAccounts())
    {
        if (account && account->getType() == AccountType::Credit)
        {
            CreditAccount* creditAcc = dynamic_cast<CreditAccount*>(account);
            if (creditAcc)
            {
                double balance = creditAcc->getBalance();
                //余额为负数说明有欠款
                if (balance < 0)
                {
                    hasDebt = true;
                    //欠款的金额为负数
                    totalDebt += (-balance);
                }
            }
        }
    }

    //如果hasDebt为true,在用户下一次登陆系统的时候给出还款提示
    if (hasDebt)
    {
        //qt格式(最小宽度,浮点,保留小数点后几位)
        QMessageBox::information(this, "还款提示",
                                 QString("您的信用账户存在未还款金额，共计 ¥%1。\n请您尽快还款！")
                                     .arg(totalDebt, 0, 'f', 2));
    }

    //设置日期范围
    QDate startDate = ui->startdateEdit->date();
    QDate endDate = ui->enddateEdit->date();
}

//更新显示用户所有账户的下拉框
void MainWindow::refreshAccountList()
{
    //清空当前账户选择下拉框
    ui->accountComboBox->clear();

    //如果是当前用户
    if (currentUser)
    {
        //每次刷新都重新从文件加载账户
        userManager.loadUserAccounts(currentUser, bankSystem);

        //获取用户信息并验证
        const auto& accounts = currentUser->getAccounts();
        for (Account* account : accounts)
        {
            if (!account) continue;

            QString accountInfo;
            //在下拉框中显示储蓄账户的余额
            if (account->getType() == AccountType::Savings)
            {
                //将sring的id转化成qt的QsString,填到占位符%1里,qt格式(最小宽度,浮点,保留小数点后几位)
                accountInfo = QString("%1 (储蓄账户 余额: ¥%2)")
                                  .arg(QString::fromStdString(account->getId()))
                                  .arg(account->getBalance(), 0, 'f', 2);
            }
            //在下拉框中显示信用账户的余额和可用额度
            else
            {
                //将account指针转换为CreditAccount*,使用dynamic_cast进行转换
                CreditAccount* creditAcc = dynamic_cast<CreditAccount*>(account);
                if (creditAcc)
                {
                    //arg()是格式化字符串函数
                    accountInfo = QString("%1 (信用账户 余额: ¥%2 可用额度: ¥%3)")
                                      .arg(QString::fromStdString(account->getId()))
                                      .arg(account->getBalance(), 0, 'f', 2)
                                      .arg(creditAcc->getAvailableCredit(), 0, 'f', 2);
                }
            }

            //addItem(文本, 数据)添加
            //accountInfo:显示在界面上的字符串
            ui->accountComboBox->addItem(accountInfo, QVariant::fromValue(account));
        }

        //如果有账户，设置默认选中第一个账户，解决总是显示请选择账户的问题
        if (ui->accountComboBox->count() > 0)
        {
            ui->accountComboBox->setCurrentIndex(0);
            onAccountSelected(0);
        }
    }
}

//处理新账户的创建
void MainWindow::handleNewAccount(AccountType type, QString id, double credit)
{
    //1.检查是否登录
    if (!currentUser)
    {
        QMessageBox::warning(this, "错误", "请先登录");
        return;
    }

    try
    {
        //2.检查账户是否已存在
        if (bankSystem->getAccount(currentUser, id.toStdString()))
        {
            QMessageBox::warning(this, "错误", "账户ID已存在");
            return;
        }

        //创建新账户
        if (type == AccountType::Savings)
        {
            bankSystem->createAccount(currentUser, AccountType::Savings, id.toStdString(), 0, 0, 0);
        }
        else if (type == AccountType::Credit)
        {
            //在创建信用账户时，传递信用额度
            bankSystem->createAccount(currentUser, AccountType::Credit, id.toStdString(), credit, 0.05, 100);
        }

        //刷新账户列表
        refreshAccountList();

        //提示账户创建成功
        QMessageBox::information(this, "成功", "账户创建成功");

        //调用userManager保存账户信息到文件
        userManager.saveUserAccounts(currentUser);
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(this, "错误", e.what());
    }
}

//存款
void MainWindow::on_depositButton_clicked()
{
    //1.如果用户没登陆或者没有选择账户,给出提示
    if (!currentUser || !currentAccount)
    {
        QMessageBox::warning(this, "错误", "请先选择有效账户");
        return;
    }

    //2.获取用户在输入框里输入的金额,并将金额转化成double类型
    //存款金额要大于0,如果用户输入的<=0要给出提示
    double amount = ui->amountEdit->text().toDouble();
    if (amount <= 0)
    {
        QMessageBox::warning(this, "错误", "金额必须大于0");
        return;
    }

    try
    {
        //存款
        bankSystem->deposit(currentUser, currentAccount->getId(), amount);
        //存款后保存账户信息,userManager调写入文件
        userManager.saveUserAccounts(currentUser);
        //保持当前账户选中的状态
        int currentIndex = ui->accountComboBox->currentIndex();
        //刷新下拉框显示更新后的金额
        refreshAccountList();
        //刷新后接着选中这个账户,记录当前索引
        ui->accountComboBox->setCurrentIndex(currentIndex);

        //更新状态栏,在左下角显示存款成功和金额
        statusBar()->showMessage(QString("存款成功 ¥%1").arg(amount), 3000);

        //更新账户的余额等信息
        onAccountSelected(currentIndex);
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(this, "错误", e.what());
    }
}

//取款(上面的存款差不多)
void MainWindow::on_withdrawButton_clicked()
{
    if (!currentUser || !currentAccount)
    {
        QMessageBox::warning(this, "错误", "请先选择有效账户");
        return;
    }

    double amount = ui->amountEdit_2->text().toDouble();
    if (amount <= 0)
    {
        QMessageBox::warning(this, "错误", "金额必须大于0");
        return;
    }

    try
    {
        //取款
        bankSystem->withdraw(currentUser, currentAccount->getId(), amount);
        //取款后保存账户信息
        userManager.saveUserAccounts(currentUser);
        //保持当前账户选中的状态
        int currentIndex = ui->accountComboBox->currentIndex();
        refreshAccountList();
        ui->accountComboBox->setCurrentIndex(currentIndex);

        //显示状态栏信息("内容",显示时长)
        //QString("内容 ¥%1").arg(替换到占位符%1位置)
        statusBar()->showMessage(QString("取款成功 ¥%1").arg(amount), 3000);

        onAccountSelected(currentIndex);
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(this, "错误", e.what());
    }
}

//点击选择账户的下拉框
void MainWindow::onAccountSelected(int index)
{
    //如果index<0或者是下拉框中没有账户,return
    if (index < 0 || ui->accountComboBox->count() == 0)
    {
        currentAccount = nullptr;
        ui->balanceLabel->setText("余额: ¥0.00");
        return;
    }

    //获取选中的账户 QVariant类型
    QVariant accountData = ui->accountComboBox->itemData(index);
    //检测QVariant中保存的数据是否为Account*类型
    if (!accountData.canConvert<Account*>())
    {
        currentAccount = nullptr;
        ui->balanceLabel->setText("余额: ¥0.00");
        return;
    }

    //从QVariant取出数据,给currentAccount
    currentAccount = accountData.value<Account*>();

    //获取账户余额,按照qt格式显示
    double balance = currentAccount->getBalance();
    //arg(要插入字符串,字符串的最小宽度,浮点,保留两位小数)
    QString balanceText = QString("余额: ¥%1").arg(balance, 0, 'f', 2);

    //如果是信用账户，还需要显示可用额度
    if (currentAccount->getType() == AccountType::Credit)
    {
        //将基类转化为派生类指针
        CreditAccount* creditAcc = dynamic_cast<CreditAccount*>(currentAccount);
        //转化后获取可用额度并附加在字符串后面
        if (creditAcc)
        {
            double availableCredit = creditAcc->getAvailableCredit();
            balanceText += QString(" 可用额度: ¥%1").arg(availableCredit, 0, 'f', 2);
        }
    }

    //更新余额显示
    ui->balanceLabel->setText(balanceText);
}

//选择账户,下拉框选项被激活的槽函数,刷新界面
void MainWindow::on_accountComboBox_activated(int index)
{
    QString selectedAccount = ui->accountComboBox->itemText(index);
    onAccountSelected(index);
}

//当创建账户按钮被按下
void MainWindow::on_createAccountButton_clicked()
{
    //设置对话框
    createaccount dlg(this);
    //连接槽函数
    connect(&dlg, &createaccount::accountCreated, this, &MainWindow::handleNewAccount);
    //进入事件循环的函数
    dlg.exec();
}

//当点击查询按钮
void MainWindow::on_quaryButton_clicked()
{
    //创建表格模型
    QStandardItemModel *model = new QStandardItemModel(this);

    //设置表头
    QStringList headers;
    headers << "账户ID" << "日期" << "金额" << "用途";
    //将表头应用到model
    model->setHorizontalHeaderLabels(headers);

    //获取日期范围
    QDate start = ui->startdateEdit->date();
    QDate end = ui->enddateEdit->date();

    //检查日期范围
    if (start > end)
    {
        QMessageBox::warning(this, "错误", "开始日期不能晚于结束日期");
        return;
    }

    //转化string类型为qt的QString
    QString accountId = QString::fromStdString(currentAccount->getId());

    //用set集合避免重复记录,存储重复进行过的交易,要不然会重复显示3万多条记录()
    std::set<QString> seenTransactions;

    //遍历map中存储的所有记录 日期record.first 交易金额,id和用途record.second
    for (auto& record : Account::recordMap)
    {
        //匹配当前id
        if (record.second.account->getId() == currentAccount->getId())
        {
            //转化日期为QDate
            QDate recordDate(record.first.year, record.first.month, record.first.day);
            //使用多个字段组合成唯一的key,防止重复显示
            //用途+年月日+金额组组成key；
            QString recordKey = QString::fromStdString(record.second.use) + QString::number(record.first.year) +
                                QString::number(record.first.month) + QString::number(record.first.day) +
                                QString::number(record.second.amount);

            //如果记录已经显示过，则跳过
            if (seenTransactions.count(recordKey) > 0)
                continue;

            //判断交易记录是否在选择查询的时间内
            if (recordDate >= start && recordDate <= end)
            {
                //记录交易，避免重复
                seenTransactions.insert(recordKey);

                //将交易转化成行插入表格 QT的格式
                //使用<<运算符把每一列的信息加入这一行
                QList<QStandardItem*> row;
                row << new QStandardItem(accountId);
                //用QDate的格式
                row << new QStandardItem(recordDate.toString("yyyy-MM-dd"));
                row << new QStandardItem(QString::number(record.second.amount));
                row << new QStandardItem(QString::fromStdString(record.second.use));
                //像表格添加一行数据
                model->appendRow(row);
            }
        }
    }

    //并添加信息到表格中,并自适应表格宽度
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //调用bankSystem的查询函数
    bankSystem->queryRecords(start, end);
}

//show事件函数,在窗口显示时调用成功登录的函数
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (currentUser)
    {
        onLoginSuccessful(currentUser);
    }
}
