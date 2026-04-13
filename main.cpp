#include "mainwindow.h"
#include"logindialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //注册自定义的Account*
    qRegisterMetaType<Account*>("Account*");
    //登陆界面
    LoginDialog login;
    if (login.exec() != QDialog::Accepted)
    {
        return 0;
    }

    MainWindow w;
    //获取刚刚登陆的用户信息,传递给mainwindow
    w.setCurrentUser(login.getAuthenticatedUser());
    w.show();
    return a.exec();
}

