#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include "models.h"

class AdminWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit AdminWindow(const User& user, QWidget* parent = nullptr);

private slots:
    void loadMovies();
    void onAddMovie();
    void onEditMovie();
    void onDeleteMovie();
    void loadBookings();
    void loadUsers();
    void onLogout();
    void refreshDashboard();

private:
    void setupUi();
    void applyStyles();
    void setupDashboard(QWidget* page);
    void setupMoviesTab(QWidget* page);
    void setupBookingsTab(QWidget* page);
    void setupUsersTab(QWidget* page);

    User          m_user;
    QTabWidget*   m_tabs;
    QTableWidget* m_movieTable;
    QTableWidget* m_bookingTable;
    QTableWidget* m_userTable;

    // Dashboard stats
    QLabel* m_totalMoviesLbl;
    QLabel* m_totalBookingsLbl;
    QLabel* m_totalRevenueLbl;
    QLabel* m_totalUsersLbl;
};
