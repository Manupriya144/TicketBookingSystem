#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include "models.h"

class LoginWindow : public QDialog {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget* parent = nullptr);
    User loggedInUser() const { return m_user; }

private slots:
    void onLogin();
    void onRegister();
    void togglePasswordVisibility(bool show);

private:
    void setupUi();
    void applyStyles();
    void showError(const QString& message);
    void showInfo(const QString& message);

    QLineEdit*   m_usernameEdit;
    QLineEdit*   m_passwordEdit;
    QLabel*      m_errorLabel;
    QLabel*      m_dbInfoLabel;
    QPushButton* m_loginBtn;
    QPushButton* m_registerBtn;
    QCheckBox*   m_showPwdCheck;

    User m_user;
};
