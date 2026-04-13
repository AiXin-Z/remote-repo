#include "logindialog.h"
#include "ui_logindialog.h"
#include "user.h"
#include<QMessageBox>
#include "global.h"
//用于用户注册登陆的类
//管理所有用户的注册登陆操作
UserManager userManager;

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    //初始化已认证的用户为nullptr
    authenticatedUser=nullptr;
    ui->setupUi(this);

    //加载已注册并被记录在文档中的用户
    userManager.loadUsersFromFile();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

//按下登录按钮
void LoginDialog::on_loginButton_clicked()
{
    qDebug() << "click login but";
    //用户名和密码
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();

    if(username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
        return;
    }
    //验证用户名和密码
    authenticatedUser = userManager.authenticate(username.toStdString(), password.toStdString());
    //如果验证成功
    if(authenticatedUser)
    {
        accept();//登录
    } else
    {
        QMessageBox::warning(this, "错误", "用户名或密码错误");
    }
}

//当点击注册按钮
void LoginDialog::on_pushButton_clicked()
{

    //获取用户输入的用户名和密码
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();

    //输入为空情况
    if(username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
        return;
    }

    //调用registerUser注册
    if(userManager.registerUser(username.toStdString(), password.toStdString()))
    {
        QMessageBox::information(this, "成功", "注册成功，请登录");
        //设置成已经注册
        isRegistered = true;
        //保存新注册的用户的信息到文本
        userManager.saveUsersToFile();
    }
    else
    {
        QMessageBox::warning(this, "错误", "用户名已存在");
    }
}

//获取认证成功的用户
User* LoginDialog::getAuthenticatedUser() const
{
    return authenticatedUser;
}


