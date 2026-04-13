#include "createaccount.h"
#include "ui_createaccount.h"
#include <QMessageBox>

//创建账户类
createaccount::createaccount(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::createaccount)
{
    ui->setupUi(this);
}

createaccount::~createaccount()
{
    delete ui;
}

void createaccount::on_accountTypeCombo_currentIndexChanged(int index)
{
    //索引为1时为信用账户
    bool isCredit = (index == 1);
    ui->creditLimitSpin->setVisible(isCredit);
    ui->label_creditLimit->setVisible(isCredit);
}

//当创建账户的按钮被点击
void createaccount::on_confirmButton_clicked()
{
    //用户输入
    //获取用户输入的账户Id
    QString accountId = ui->accountIdEdit->text();
    //在下拉框控件中选中账户类型的索引，并将索引值转化为账户种类
    AccountType type = static_cast<AccountType>(ui->accountTypeCombo->currentIndex());
    //输入信用额度
    double creditLimit = ui->creditLimitSpin->value();

    //验证
    if (accountId.isEmpty())
    {
        QMessageBox::warning(this, "错误", "账户ID不能为空");
        return;
    }

    if (type == AccountType::Credit && creditLimit < 1)
    {
        QMessageBox::warning(this, "错误", "信用额度最低¥1");
        return;
    }

    //发射信号
    emit accountCreated(type, accountId, creditLimit);
    //关闭对话框
    accept();
}

//当点击取消按钮
void createaccount::on_cancelButton_clicked()
{
    reject();
}


