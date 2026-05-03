#include "loginwindow.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QInputDialog>
#include <QFrame>
#include <QFont>
#include <QDir>

LoginWindow::LoginWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("CineBook - Movie Ticket Booking");
    setFixedSize(500, 610);
    setModal(true);
    setupUi();
    applyStyles();
}

void LoginWindow::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    auto* header = new QFrame();
    header->setObjectName("headerBanner");
    header->setFixedHeight(160);
    auto* headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(28, 24, 28, 24);
    headerLayout->setSpacing(6);
    headerLayout->setAlignment(Qt::AlignCenter);

    auto* badge = new QLabel("CB");
    badge->setObjectName("brandBadge");
    badge->setAlignment(Qt::AlignCenter);

    auto* title = new QLabel("CineBook");
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);

    auto* subtitle = new QLabel("Professional movie ticket booking");
    subtitle->setObjectName("subtitleLabel");
    subtitle->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(badge, 0, Qt::AlignCenter);
    headerLayout->addWidget(title);
    headerLayout->addWidget(subtitle);

    auto* formFrame = new QFrame();
    formFrame->setObjectName("formFrame");
    auto* formLayout = new QVBoxLayout(formFrame);
    formLayout->setSpacing(14);
    formLayout->setContentsMargins(40, 30, 40, 30);

    auto* formTitle = new QLabel("Sign in to continue");
    formTitle->setObjectName("formTitle");

    auto* formSubtitle = new QLabel("Use your account credentials or the seeded demo accounts below.");
    formSubtitle->setObjectName("formSubtitle");
    formSubtitle->setWordWrap(true);

    auto* userLabel = new QLabel("Username");
    userLabel->setObjectName("fieldLabel");

    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("Enter your username");
    m_usernameEdit->setObjectName("inputField");
    m_usernameEdit->setClearButtonEnabled(true);

    auto* pwdLabel = new QLabel("Password");
    pwdLabel->setObjectName("fieldLabel");

    auto* passwordRow = new QHBoxLayout();
    passwordRow->setSpacing(10);
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("Enter your password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setObjectName("inputField");

    m_showPwdCheck = new QCheckBox("Show");
    m_showPwdCheck->setObjectName("showPwdCheck");
    passwordRow->addWidget(m_passwordEdit, 1);
    passwordRow->addWidget(m_showPwdCheck);

    m_errorLabel = new QLabel();
    m_errorLabel->setObjectName("messageLabel");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();

    m_loginBtn = new QPushButton("Sign In");
    m_loginBtn->setObjectName("loginBtn");
    m_loginBtn->setFixedHeight(46);
    m_loginBtn->setCursor(Qt::PointingHandCursor);

    auto* divider = new QLabel("Need a new account?");
    divider->setObjectName("dividerLabel");
    divider->setAlignment(Qt::AlignCenter);

    m_registerBtn = new QPushButton("Create New Account");
    m_registerBtn->setObjectName("registerBtn");
    m_registerBtn->setFixedHeight(42);
    m_registerBtn->setCursor(Qt::PointingHandCursor);

    auto* demoHint = new QLabel("Demo accounts: admin / admin123    user1 / user123");
    demoHint->setObjectName("hintLabel");
    demoHint->setWordWrap(true);
    demoHint->setAlignment(Qt::AlignCenter);

    m_dbInfoLabel = new QLabel(QString("Active database: %1").arg(QDir::toNativeSeparators(Database::instance().databasePath())));
    m_dbInfoLabel->setObjectName("dbInfoLabel");
    m_dbInfoLabel->setWordWrap(true);

    formLayout->addWidget(formTitle);
    formLayout->addWidget(formSubtitle);
    formLayout->addSpacing(4);
    formLayout->addWidget(userLabel);
    formLayout->addWidget(m_usernameEdit);
    formLayout->addWidget(pwdLabel);
    formLayout->addLayout(passwordRow);
    formLayout->addWidget(m_errorLabel);
    formLayout->addSpacing(4);
    formLayout->addWidget(m_loginBtn);
    formLayout->addWidget(divider);
    formLayout->addWidget(m_registerBtn);
    formLayout->addSpacing(8);
    formLayout->addWidget(demoHint);
    formLayout->addWidget(m_dbInfoLabel);

    root->addWidget(header);
    root->addWidget(formFrame, 1);

    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWindow::onLogin);
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginWindow::onRegister);
    connect(m_showPwdCheck, &QCheckBox::toggled, this, &LoginWindow::togglePasswordVisibility);
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLogin);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLogin);
    connect(m_usernameEdit, &QLineEdit::textChanged, this, [this] { m_errorLabel->hide(); });
    connect(m_passwordEdit, &QLineEdit::textChanged, this, [this] { m_errorLabel->hide(); });
}

void LoginWindow::applyStyles() {
    setStyleSheet(R"(
        QDialog { background: #0f1220; }
        #headerBanner {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #17203a, stop:0.55 #1f2d52, stop:1 #10243e);
            border-bottom: 2px solid #ef476f;
        }
        #brandBadge {
            min-width: 58px;
            max-width: 58px;
            min-height: 58px;
            max-height: 58px;
            border-radius: 29px;
            background: rgba(255,255,255,0.08);
            border: 1px solid rgba(255,255,255,0.16);
            color: #ffffff;
            font-size: 22px;
            font-weight: 800;
            font-family: 'Segoe UI';
        }
        #titleLabel {
            color: #ffffff;
            font-size: 30px;
            font-weight: 800;
            font-family: 'Georgia', serif;
            letter-spacing: 2px;
        }
        #subtitleLabel {
            color: #ff6b8a;
            font-size: 12px;
            letter-spacing: 1px;
            font-family: 'Segoe UI';
        }
        #formFrame { background: #111423; }
        #formTitle {
            color: #ffffff;
            font-size: 22px;
            font-weight: 700;
            font-family: 'Segoe UI';
        }
        #formSubtitle {
            color: #9aa4c7;
            font-size: 13px;
            font-family: 'Segoe UI';
        }
        #fieldLabel {
            color: #9aa4c7;
            font-size: 12px;
            font-weight: 600;
            letter-spacing: 1px;
            font-family: 'Segoe UI';
        }
        #inputField {
            background: #1b2140;
            border: 1px solid #30385c;
            border-radius: 10px;
            color: #ffffff;
            font-size: 14px;
            padding: 11px 14px;
            font-family: 'Segoe UI';
            min-height: 22px;
        }
        #inputField:focus {
            border-color: #ef476f;
            background: #20284a;
        }
        #loginBtn {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ef476f, stop:1 #d63862);
            color: #ffffff;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: 700;
            font-family: 'Segoe UI';
        }
        #loginBtn:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ff5d83, stop:1 #ef476f); }
        #registerBtn {
            background: transparent;
            color: #ef476f;
            border: 1px solid #ef476f;
            border-radius: 10px;
            font-size: 13px;
            font-weight: 600;
            font-family: 'Segoe UI';
        }
        #registerBtn:hover { background: rgba(239,71,111,0.10); }
        #dividerLabel {
            color: #6f7898;
            font-size: 12px;
            font-family: 'Segoe UI';
        }
        #messageLabel {
            background: rgba(239,71,111,0.10);
            border: 1px solid rgba(239,71,111,0.28);
            border-radius: 8px;
            color: #ff90a7;
            font-size: 12px;
            padding: 10px 12px;
            font-family: 'Segoe UI';
        }
        #hintLabel {
            color: #7f88a8;
            font-size: 11px;
            font-family: 'Consolas';
        }
        #dbInfoLabel {
            color: #6f7898;
            font-size: 11px;
            font-family: 'Segoe UI';
            padding-top: 4px;
        }
        #showPwdCheck {
            color: #9aa4c7;
            font-size: 12px;
            font-family: 'Segoe UI';
        }
    )");
}

void LoginWindow::showError(const QString& message) {
    m_errorLabel->setStyleSheet(
        "background: rgba(239,71,111,0.10);"
        "border: 1px solid rgba(239,71,111,0.28);"
        "border-radius: 8px;"
        "color: #ff90a7;"
        "font-size: 12px;"
        "padding: 10px 12px;"
        "font-family: 'Segoe UI';"
    );
    m_errorLabel->setText(message);
    m_errorLabel->show();
}

void LoginWindow::showInfo(const QString& message) {
    m_errorLabel->setStyleSheet(
        "background: rgba(74,222,128,0.10);"
        "border: 1px solid rgba(74,222,128,0.28);"
        "border-radius: 8px;"
        "color: #86efac;"
        "font-size: 12px;"
        "padding: 10px 12px;"
        "font-family: 'Segoe UI';"
    );
    m_errorLabel->setText(message);
    m_errorLabel->show();
}

void LoginWindow::onLogin() {
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        showError("Please enter both your username and password.");
        return;
    }

    const QString hashed = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
    User user;
    if (Database::instance().validateLogin(username, hashed, user, password)) {
        m_user = user;
        accept();
        return;
    }

    QString message = "Invalid username or password.";
    if (!Database::instance().lastError().isEmpty()) {
        message += "\nDatabase error: " + Database::instance().lastError();
    }
    showError(message);
    m_passwordEdit->clear();
    m_passwordEdit->setFocus();
}

void LoginWindow::onRegister() {
    bool ok = false;
    const QString fullName = QInputDialog::getText(this, "Register", "Full Name:", QLineEdit::Normal, "", &ok).trimmed();
    if (!ok || fullName.isEmpty()) return;

    const QString username = QInputDialog::getText(this, "Register", "Choose Username:", QLineEdit::Normal, "", &ok).trimmed();
    if (!ok || username.isEmpty()) return;

    const QString email = QInputDialog::getText(this, "Register", "Email:", QLineEdit::Normal, "", &ok).trimmed();
    if (!ok) return;

    const QString password = QInputDialog::getText(this, "Register", "Choose Password:", QLineEdit::Password, "", &ok);
    if (!ok || password.isEmpty()) return;

    if (username.contains(' ')) {
        QMessageBox::warning(this, "Registration Failed", "Username cannot contain spaces.");
        return;
    }
    if (!email.isEmpty() && (!email.contains('@') || !email.contains('.'))) {
        QMessageBox::warning(this, "Registration Failed", "Please enter a valid email address.");
        return;
    }
    if (password.size() < 6) {
        QMessageBox::warning(this, "Registration Failed", "Password must be at least 6 characters long.");
        return;
    }

    const User existing = Database::instance().getUserByUsername(username);
    if (existing.id > 0) {
        QMessageBox::warning(this, "Registration Failed", "Username already exists. Choose another one.");
        return;
    }

    User newUser;
    newUser.username = username;
    newUser.password = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
    newUser.role = "user";
    newUser.fullName = fullName;
    newUser.email = email;

    if (!Database::instance().createUser(newUser)) {
        QMessageBox::critical(this, "Registration Failed", "Could not create account:\n" + Database::instance().lastError());
        return;
    }

    m_usernameEdit->setText(username);
    m_passwordEdit->clear();
    m_passwordEdit->setFocus();
    showInfo("Account created successfully. You can sign in now.");
}

void LoginWindow::togglePasswordVisibility(bool show) {
    m_passwordEdit->setEchoMode(show ? QLineEdit::Normal : QLineEdit::Password);
}
