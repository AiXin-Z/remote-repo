#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QStackedWidget>
#include<QComboBox>
#include <QTableWidgetItem>
#include "banksystem.h"
#include"account.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void refreshAccountList();//刷新
    void updateAccountDisplay();//更新账户显示
    void setCurrentUser(User* user)//是当前账户
    {
        currentUser = user;
    }

    void loadAllData();
    void saveAllData();
    void onLoginSuccessful(User* user);
    BankSystem* bankSystem;

private slots:

    void handleNewAccount(AccountType type, QString id, double credit);//新账户处理槽函数
    void on_depositButton_clicked();
    void on_withdrawButton_clicked();
    void on_quaryButton_clicked();
    void onAccountSelected(int index);
    void on_accountComboBox_activated(int index);
    void on_createAccountButton_clicked();
    void updateTransactionTable(QStandardItemModel* model);


private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    QWidget *loginPage;
    QWidget *mainPage;
    //BankSystem *bankSystem;
    Account *acc;
    User* currentUser;
    Account* currentAccount;
    QComboBox* accountComboBox;

protected:
    void showEvent(QShowEvent *event) override;

};

#endif // MAINWINDOW_H





