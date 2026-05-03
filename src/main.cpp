#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include "database.h"
#include "loginwindow.h"
#include "mainwindow.h"
#include "adminwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("CineBook");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Group 24 - University of Ruhuna");

    const QString dbDirPath = QDir(QApplication::applicationDirPath()).filePath("data");
    QDir().mkpath(dbDirPath);
    const QString dbPath = QDir(dbDirPath).filePath("movie_booking.db");

    if (!Database::instance().init(dbPath)) {
        QMessageBox::critical(nullptr, "Database Error",
            "Failed to initialize database:\n" + Database::instance().lastError());
        return 1;
    }

    LoginWindow login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    const User user = login.loggedInUser();
    if (user.role == "admin") {
        AdminWindow adminWin(user);
        adminWin.show();
        return app.exec();
    }

    MainWindow mainWin(user);
    mainWin.show();
    return app.exec();
}
