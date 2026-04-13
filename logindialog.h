#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include"user.h"
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    User* getAuthenticatedUser()const;

private slots:
    void on_loginButton_clicked();
    void on_pushButton_clicked();

private:
    Ui::LoginDialog *ui;
    User* authenticatedUser;
    bool isRegistered;//是否注册成功
};

#endif // LOGINDIALOG_H



