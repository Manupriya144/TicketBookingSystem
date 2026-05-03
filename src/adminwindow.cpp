#include "adminwindow.h"
#include "database.h"
#include "loginwindow.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QApplication>
#include <QColor>
#include <QStringList>

namespace {
QStringList splitShowTimes(const QString& rawShowTimes) {
    QStringList shows = rawShowTimes.split(",", Qt::SkipEmptyParts);
    for (QString& show : shows) {
        show = show.trimmed();
    }
    shows.removeAll(QString());
    shows.removeDuplicates();
    return shows;
}

QString normalizedShowTimes(const QString& rawShowTimes) {
    return splitShowTimes(rawShowTimes).join(", ");
}

QString validateMovieInput(const Movie& movie) {
    if (movie.title.trimmed().isEmpty()) {
        return "Title is required.";
    }
    if (movie.genre.trimmed().isEmpty()) {
        return "Genre is required.";
    }
    if (movie.duration.trimmed().isEmpty()) {
        return "Duration is required.";
    }
    if (movie.ticketPrice <= 0) {
        return "Ticket price must be greater than zero.";
    }
    if (splitShowTimes(movie.showTimes).isEmpty()) {
        return "Please provide at least one show time.";
    }
    return QString();
}
}

AdminWindow::AdminWindow(const User& user, QWidget* parent)
    : QMainWindow(parent), m_user(user) {
    setWindowTitle("🎬 CineBook Admin Panel");
    resize(1280, 800);
    setupUi();
    applyStyles();
    refreshDashboard();
    loadMovies();
    loadBookings();
    loadUsers();
}

void AdminWindow::setupUi() {
    auto* central = new QWidget();
    setCentralWidget(central);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── Top bar ───────────────────────────────────────────────────────────────
    auto* topBar = new QFrame();
    topBar->setObjectName("adminTopBar");
    topBar->setFixedHeight(60);
    auto* topLay = new QHBoxLayout(topBar);
    topLay->setContentsMargins(24, 0, 24, 0);

    auto* logoLbl = new QLabel("🎬 CineBook  ·  Admin Panel");
    logoLbl->setObjectName("adminLogo");

    auto* userLbl = new QLabel(QString("👤 %1  (Admin)").arg(m_user.fullName.isEmpty() ? m_user.username : m_user.fullName));
    userLbl->setObjectName("adminUserLbl");

    auto* logoutBtn = new QPushButton("🚪 Logout");
    logoutBtn->setObjectName("adminLogoutBtn");
    logoutBtn->setFixedWidth(100);
    logoutBtn->setCursor(Qt::PointingHandCursor);

    topLay->addWidget(logoLbl);
    topLay->addStretch();
    topLay->addWidget(userLbl);
    topLay->addSpacing(16);
    topLay->addWidget(logoutBtn);

    // ── Tabs ──────────────────────────────────────────────────────────────────
    m_tabs = new QTabWidget();
    m_tabs->setObjectName("adminTabs");

    auto* dashPage    = new QWidget(); dashPage->setObjectName("adminPage");
    auto* moviesPage  = new QWidget(); moviesPage->setObjectName("adminPage");
    auto* bookPage    = new QWidget(); bookPage->setObjectName("adminPage");
    auto* usersPage   = new QWidget(); usersPage->setObjectName("adminPage");

    setupDashboard(dashPage);
    setupMoviesTab(moviesPage);
    setupBookingsTab(bookPage);
    setupUsersTab(usersPage);

    m_tabs->addTab(dashPage,   "📊  Dashboard");
    m_tabs->addTab(moviesPage, "🎬  Movies");
    m_tabs->addTab(bookPage,   "📋  Bookings");
    m_tabs->addTab(usersPage,  "👤  Users");

    root->addWidget(topBar);
    root->addWidget(m_tabs, 1);

    connect(logoutBtn, &QPushButton::clicked, this, &AdminWindow::onLogout);
}

void AdminWindow::setupDashboard(QWidget* page) {
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(32, 24, 32, 24);
    lay->setSpacing(24);

    auto* title = new QLabel("Dashboard Overview");
    title->setObjectName("sectionTitle");

    // Stat cards
    auto* cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(16);

    auto mkCard = [&](const QString& icon, const QString& label, QLabel*& valRef) {
        auto* card = new QFrame();
        card->setObjectName("statCard");
        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(24,20,24,20);
        auto* iconLbl = new QLabel(icon);
        iconLbl->setObjectName("cardIcon");
        valRef = new QLabel("—");
        valRef->setObjectName("cardValue");
        auto* nameLbl = new QLabel(label);
        nameLbl->setObjectName("cardName");
        cl->addWidget(iconLbl);
        cl->addWidget(valRef);
        cl->addWidget(nameLbl);
        return card;
    };

    cardsRow->addWidget(mkCard("🎬", "Total Movies",    m_totalMoviesLbl));
    cardsRow->addWidget(mkCard("📋", "Total Bookings",  m_totalBookingsLbl));
    cardsRow->addWidget(mkCard("💰", "Total Revenue",   m_totalRevenueLbl));
    cardsRow->addWidget(mkCard("👤", "Registered Users",m_totalUsersLbl));

    lay->addWidget(title);
    lay->addLayout(cardsRow);
    lay->addStretch();
}

void AdminWindow::setupMoviesTab(QWidget* page) {
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(24, 16, 24, 16);
    lay->setSpacing(12);

    auto* toolRow = new QHBoxLayout();
    auto* addBtn  = new QPushButton("➕ Add Movie");
    addBtn->setObjectName("actionBtn");
    addBtn->setCursor(Qt::PointingHandCursor);
    auto* editBtn = new QPushButton("✏️ Edit");
    editBtn->setObjectName("actionBtn");
    editBtn->setCursor(Qt::PointingHandCursor);
    auto* delBtn  = new QPushButton("🗑 Delete");
    delBtn->setObjectName("dangerBtn");
    delBtn->setCursor(Qt::PointingHandCursor);
    toolRow->addWidget(addBtn);
    toolRow->addWidget(editBtn);
    toolRow->addWidget(delBtn);
    toolRow->addStretch();

    m_movieTable = new QTableWidget();
    m_movieTable->setObjectName("adminTable");
    m_movieTable->setColumnCount(9);
    m_movieTable->setHorizontalHeaderLabels({"ID","Title","Genre","Duration","Price (LKR)","Seats","Rating","Shows","Status"});
    m_movieTable->horizontalHeader()->setStretchLastSection(true);
    m_movieTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_movieTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_movieTable->setAlternatingRowColors(true);
    m_movieTable->verticalHeader()->setVisible(false);
    m_movieTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    lay->addLayout(toolRow);
    lay->addWidget(m_movieTable, 1);

    connect(addBtn,  &QPushButton::clicked, this, &AdminWindow::onAddMovie);
    connect(editBtn, &QPushButton::clicked, this, &AdminWindow::onEditMovie);
    connect(delBtn,  &QPushButton::clicked, this, &AdminWindow::onDeleteMovie);
}

void AdminWindow::setupBookingsTab(QWidget* page) {
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(24, 16, 24, 16);
    lay->setSpacing(12);

    auto* toolRow = new QHBoxLayout();
    auto* refreshBtn = new QPushButton("🔄 Refresh");
    refreshBtn->setObjectName("actionBtn");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    connect(refreshBtn, &QPushButton::clicked, this, &AdminWindow::loadBookings);
    toolRow->addWidget(refreshBtn);
    toolRow->addStretch();

    m_bookingTable = new QTableWidget();
    m_bookingTable->setObjectName("adminTable");
    m_bookingTable->setColumnCount(9);
    m_bookingTable->setHorizontalHeaderLabels({"ID","User ID","Movie","Show Time","Booked At","Seats","Tickets","Total (LKR)","Status"});
    m_bookingTable->horizontalHeader()->setStretchLastSection(true);
    m_bookingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bookingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_bookingTable->setAlternatingRowColors(true);
    m_bookingTable->verticalHeader()->setVisible(false);
    m_bookingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    lay->addLayout(toolRow);
    lay->addWidget(m_bookingTable, 1);
}

void AdminWindow::setupUsersTab(QWidget* page) {
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(24, 16, 24, 16);

    m_userTable = new QTableWidget();
    m_userTable->setObjectName("adminTable");
    m_userTable->setColumnCount(5);
    m_userTable->setHorizontalHeaderLabels({"ID","Username","Full Name","Email","Role"});
    m_userTable->horizontalHeader()->setStretchLastSection(true);
    m_userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_userTable->setAlternatingRowColors(true);
    m_userTable->verticalHeader()->setVisible(false);
    m_userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    lay->addWidget(m_userTable, 1);
}

void AdminWindow::applyStyles() {
    setStyleSheet(R"(
        QMainWindow, QWidget { background: #0f0f1a; color: #e0e0f0; }
        #adminTopBar { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #1a1a2e, stop:1 #0f3460); border-bottom: 2px solid #e94560; }
        #adminLogo { color: white; font-size: 18px; font-weight: 800; font-family: 'Georgia', serif; letter-spacing: 2px; }
        #adminUserLbl { color: #8888aa; font-size: 13px; font-family: 'Segoe UI'; }
        #adminLogoutBtn { background: rgba(233,69,96,0.15); color: #e94560; border: 1px solid rgba(233,69,96,0.3); border-radius: 6px; padding: 6px; font-size: 12px; font-family: 'Segoe UI'; }
        #adminLogoutBtn:hover { background: rgba(233,69,96,0.3); }
        #adminTabs { background: #0f0f1a; border: none; }
        QTabBar::tab { background: #12121f; color: #8888aa; border: none; padding: 10px 20px; font-size: 13px; font-family: 'Segoe UI'; border-bottom: 2px solid transparent; }
        QTabBar::tab:selected { color: #e94560; border-bottom: 2px solid #e94560; background: #0f0f1a; }
        QTabBar::tab:hover:!selected { color: #ccccee; background: #1a1a2e; }
        QTabWidget::pane { border: none; background: #0f0f1a; }
        #adminPage { background: #0f0f1a; }
        #sectionTitle { color: #ffffff; font-size: 20px; font-weight: 700; font-family: 'Georgia'; margin-bottom: 8px; }
        #statCard { background: #12121f; border: 1px solid #1e1e35; border-radius: 12px; min-width: 180px; }
        #cardIcon { font-size: 32px; }
        #cardValue { color: #ffffff; font-size: 28px; font-weight: 900; font-family: 'Georgia'; }
        #cardName { color: #8888aa; font-size: 13px; font-family: 'Segoe UI'; letter-spacing: 1px; }
        #actionBtn { background: #1e1e35; color: #ccccee; border: 1px solid #2d2d4e; border-radius: 6px; padding: 8px 16px; font-size: 13px; font-family: 'Segoe UI'; }
        #actionBtn:hover { background: #2d2d4e; }
        #dangerBtn { background: rgba(233,69,96,0.15); color: #e94560; border: 1px solid rgba(233,69,96,0.3); border-radius: 6px; padding: 8px 16px; font-size: 13px; font-family: 'Segoe UI'; }
        #dangerBtn:hover { background: rgba(233,69,96,0.3); }
        #adminTable { background: #12121f; border: 1px solid #1e1e35; border-radius: 8px; gridline-color: #1e1e35; color: #e0e0f0; font-family: 'Segoe UI'; font-size: 13px; alternate-background-color: #1a1a2e; }
        QHeaderView::section { background: #1e1e35; color: #8888aa; border: none; padding: 10px; font-size: 12px; font-weight: 600; letter-spacing: 1px; }
        QDialogButtonBox QPushButton { background: #1e1e35; color: #ccccee; border: 1px solid #2d2d4e; border-radius: 6px; padding: 6px 16px; }
        QLineEdit, QTextEdit, QDoubleSpinBox, QSpinBox { background: #1e1e35; border: 1.5px solid #2d2d4e; border-radius: 6px; color: #e0e0f0; padding: 6px 10px; font-family: 'Segoe UI'; font-size: 13px; }
        QLineEdit:focus, QTextEdit:focus { border-color: #e94560; }
    )");
}

void AdminWindow::refreshDashboard() {
    auto movies   = Database::instance().getAllMovies(false);
    auto bookings = Database::instance().getAllBookings();
    auto users    = Database::instance().getAllUsers();

    double revenue = 0;
    for (const Booking& b : bookings)
        if (b.status == "confirmed") revenue += b.totalAmount;

    m_totalMoviesLbl->setText(QString::number(movies.size()));
    m_totalBookingsLbl->setText(QString::number(bookings.size()));
    m_totalRevenueLbl->setText(QString("LKR %1").arg(revenue, 0, 'f', 0));
    m_totalUsersLbl->setText(QString::number(users.size()));
}

void AdminWindow::loadMovies() {
    auto movies = Database::instance().getAllMovies(false);
    m_movieTable->setRowCount(movies.size());
    for (int i = 0; i < movies.size(); ++i) {
        const Movie& m = movies[i];
        m_movieTable->setItem(i, 0, new QTableWidgetItem(QString::number(m.id)));
        m_movieTable->setItem(i, 1, new QTableWidgetItem(m.title));
        m_movieTable->setItem(i, 2, new QTableWidgetItem(m.genre));
        m_movieTable->setItem(i, 3, new QTableWidgetItem(m.duration));
        m_movieTable->setItem(i, 4, new QTableWidgetItem(QString::number(m.ticketPrice, 'f', 2)));
        m_movieTable->setItem(i, 5, new QTableWidgetItem(QString::number(m.totalSeats)));
        m_movieTable->setItem(i, 6, new QTableWidgetItem(m.rating));
        m_movieTable->setItem(i, 7, new QTableWidgetItem(normalizedShowTimes(m.showTimes)));
        auto* statusItem = new QTableWidgetItem(m.isActive ? "ACTIVE" : "INACTIVE");
        statusItem->setForeground(m.isActive ? QColor("#4ade80") : QColor("#f87171"));
        m_movieTable->setItem(i, 8, statusItem);
    }
    refreshDashboard();
}

void AdminWindow::loadBookings() {
    auto bookings = Database::instance().getAllBookings();
    m_bookingTable->setRowCount(bookings.size());
    for (int i = 0; i < bookings.size(); ++i) {
        const Booking& b = bookings[i];
        m_bookingTable->setItem(i, 0, new QTableWidgetItem(QString::number(b.id)));
        m_bookingTable->setItem(i, 1, new QTableWidgetItem(QString::number(b.userId)));
        m_bookingTable->setItem(i, 2, new QTableWidgetItem(b.movieTitle));
        m_bookingTable->setItem(i, 3, new QTableWidgetItem(b.showTime));
        m_bookingTable->setItem(i, 4, new QTableWidgetItem(b.bookedAt.isValid() ? b.bookedAt.toString("dd MMM yyyy hh:mm AP") : "--"));
        m_bookingTable->setItem(i, 5, new QTableWidgetItem(b.seats.join(", ")));
        m_bookingTable->setItem(i, 6, new QTableWidgetItem(QString::number(b.numTickets)));
        m_bookingTable->setItem(i, 7, new QTableWidgetItem(QString("LKR %1").arg(b.totalAmount, 0,'f',2)));
        auto* st = new QTableWidgetItem(b.status.toUpper());
        st->setForeground(b.status == "confirmed" ? QColor("#4ade80") : QColor("#f87171"));
        m_bookingTable->setItem(i, 8, st);
    }
    refreshDashboard();
}

void AdminWindow::loadUsers() {
    auto users = Database::instance().getAllUsers();
    m_userTable->setRowCount(users.size());
    for (int i = 0; i < users.size(); ++i) {
        const User& u = users[i];
        m_userTable->setItem(i, 0, new QTableWidgetItem(QString::number(u.id)));
        m_userTable->setItem(i, 1, new QTableWidgetItem(u.username));
        m_userTable->setItem(i, 2, new QTableWidgetItem(u.fullName));
        m_userTable->setItem(i, 3, new QTableWidgetItem(u.email));
        auto* roleItem = new QTableWidgetItem(u.role.toUpper());
        roleItem->setForeground(u.role == "admin" ? QColor("#e94560") : QColor("#60a5fa"));
        m_userTable->setItem(i, 4, roleItem);
    }
}

void AdminWindow::onAddMovie() {
    QDialog dlg(this);
    dlg.setWindowTitle("Add New Movie");
    dlg.setFixedSize(460, 500);
    dlg.setStyleSheet(styleSheet());

    auto* lay = new QFormLayout(&dlg);
    lay->setContentsMargins(24, 20, 24, 20);
    lay->setSpacing(12);

    auto* titleEdit = new QLineEdit();
    auto* genreEdit = new QLineEdit();
    auto* durEdit   = new QLineEdit(); durEdit->setPlaceholderText("e.g. 2h 15m");
    auto* descEdit  = new QTextEdit(); descEdit->setFixedHeight(80);
    auto* priceEdit = new QDoubleSpinBox(); priceEdit->setRange(0,9999); priceEdit->setValue(1000); priceEdit->setPrefix("LKR ");
    auto* seatsEdit = new QSpinBox();   seatsEdit->setRange(10,200); seatsEdit->setValue(60);
    auto* showEdit  = new QLineEdit(); showEdit->setPlaceholderText("10:00,14:00,18:00");
    auto* ratingEdit= new QLineEdit(); ratingEdit->setPlaceholderText("PG, PG-13, R ...");

    lay->addRow("Title:",      titleEdit);
    lay->addRow("Genre:",      genreEdit);
    lay->addRow("Duration:",   durEdit);
    lay->addRow("Description:",descEdit);
    lay->addRow("Ticket Price:", priceEdit);
    lay->addRow("Total Seats:", seatsEdit);
    lay->addRow("Show Times:", showEdit);
    lay->addRow("Rating:",     ratingEdit);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    lay->addRow(btns);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        Movie m;
        m.title       = titleEdit->text().trimmed();
        m.genre       = genreEdit->text().trimmed();
        m.duration    = durEdit->text().trimmed();
        m.description = descEdit->toPlainText().trimmed();
        m.ticketPrice = priceEdit->value();
        m.totalSeats  = seatsEdit->value();
        m.showTimes   = normalizedShowTimes(showEdit->text());
        m.rating      = ratingEdit->text().trimmed().isEmpty() ? "NR" : ratingEdit->text().trimmed();
        const QString validationError = validateMovieInput(m);
        if (!validationError.isEmpty()) {
            QMessageBox::warning(this, "Error", validationError);
            return;
        }
        if (Database::instance().addMovie(m))
            loadMovies();
        else
            QMessageBox::critical(this,"Error","Failed to add movie: " + Database::instance().lastError());
    }
}

void AdminWindow::onEditMovie() {
    int row = m_movieTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"Select Movie","Please select a movie to edit."); return; }
    int movieId = m_movieTable->item(row,0)->text().toInt();
    Movie m = Database::instance().getMovie(movieId);

    QDialog dlg(this);
    dlg.setWindowTitle("Edit Movie");
    dlg.setFixedSize(460, 500);
    dlg.setStyleSheet(styleSheet());

    auto* lay = new QFormLayout(&dlg);
    lay->setContentsMargins(24,20,24,20);
    lay->setSpacing(12);

    auto* titleEdit = new QLineEdit(m.title);
    auto* genreEdit = new QLineEdit(m.genre);
    auto* durEdit   = new QLineEdit(m.duration);
    auto* descEdit  = new QTextEdit(); descEdit->setText(m.description); descEdit->setFixedHeight(80);
    auto* priceEdit = new QDoubleSpinBox(); priceEdit->setRange(0,9999); priceEdit->setValue(m.ticketPrice); priceEdit->setPrefix("LKR ");
    auto* seatsEdit = new QSpinBox();   seatsEdit->setRange(10,200); seatsEdit->setValue(m.totalSeats);
    auto* showEdit  = new QLineEdit(m.showTimes);
    auto* ratingEdit= new QLineEdit(m.rating);

    lay->addRow("Title:",       titleEdit);
    lay->addRow("Genre:",       genreEdit);
    lay->addRow("Duration:",    durEdit);
    lay->addRow("Description:", descEdit);
    lay->addRow("Ticket Price:", priceEdit);
    lay->addRow("Total Seats:", seatsEdit);
    lay->addRow("Show Times:",  showEdit);
    lay->addRow("Rating:",      ratingEdit);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    lay->addRow(btns);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        m.title       = titleEdit->text().trimmed();
        m.genre       = genreEdit->text().trimmed();
        m.duration    = durEdit->text().trimmed();
        m.description = descEdit->toPlainText().trimmed();
        m.ticketPrice = priceEdit->value();
        m.totalSeats  = seatsEdit->value();
        m.showTimes   = normalizedShowTimes(showEdit->text());
        m.rating      = ratingEdit->text().trimmed().isEmpty() ? "NR" : ratingEdit->text().trimmed();
        const QString validationError = validateMovieInput(m);
        if (!validationError.isEmpty()) {
            QMessageBox::warning(this, "Error", validationError);
            return;
        }
        if (Database::instance().updateMovie(m))
            loadMovies();
        else
            QMessageBox::critical(this,"Error","Failed to update movie: " + Database::instance().lastError());
    }
}

void AdminWindow::onDeleteMovie() {
    int row = m_movieTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"Select Movie","Please select a movie to delete."); return; }
    int movieId = m_movieTable->item(row,0)->text().toInt();
    QString title = m_movieTable->item(row,1)->text();
    if (QMessageBox::question(this,"Confirm Delete",
        QString("Are you sure you want to remove '%1'?").arg(title)) == QMessageBox::Yes) {
        if (Database::instance().deleteMovie(movieId))
            loadMovies();
    }
}

void AdminWindow::onLogout() {
    if (QMessageBox::question(this,"Logout","Are you sure you want to logout?") == QMessageBox::Yes) {
        close();
        LoginWindow* login = new LoginWindow();
        if (login->exec() == QDialog::Accepted) {
            User u = login->loggedInUser();
            if (u.role == "admin") {
                AdminWindow* aw = new AdminWindow(u);
                aw->show();
            } else {
                MainWindow* mw = new MainWindow(u);
                mw->show();
            }
        }
        delete login;
    }
}
