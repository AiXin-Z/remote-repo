#ifndef CREATEACCOUNT_H
#define CREATEACCOUNT_H

#include <QDialog>
#include"account.h"
namespace Ui {
class createaccount;
}

class createaccount : public QDialog
{
    Q_OBJECT

public:
    explicit createaccount(QWidget *parent = nullptr);
    ~createaccount();

signals:
    void accountCreated(AccountType accountType, QString accountId, double creditLimit);

private slots:
    void on_accountTypeCombo_currentIndexChanged(int index);
    void on_confirmButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::createaccount *ui;
};

#endif // CREATEACCOUNT_H







