#include "mainwindow.h"
#include "database.h"
#include "seatselection.h"
#include "receiptdialog.h"
#include "loginwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QSignalBlocker>
#include <QColor>
#include <QStringList>
#include <QTableWidgetItem>
#include <algorithm>

namespace {
bool seatCodeLessThan(const QString& left, const QString& right) {
    int leftDigitIndex = 0;
    while (leftDigitIndex < left.size() && !left[leftDigitIndex].isDigit()) {
        ++leftDigitIndex;
    }

    int rightDigitIndex = 0;
    while (rightDigitIndex < right.size() && !right[rightDigitIndex].isDigit()) {
        ++rightDigitIndex;
    }

    const QString leftRow = left.left(leftDigitIndex);
    const QString rightRow = right.left(rightDigitIndex);
    if (leftRow != rightRow) {
        return leftRow < rightRow;
    }

    return left.mid(leftDigitIndex).toInt() < right.mid(rightDigitIndex).toInt();
}

QStringList splitShowTimes(const QString& rawShowTimes) {
    QStringList shows = rawShowTimes.split(",", Qt::SkipEmptyParts);
    for (QString& show : shows) {
        show = show.trimmed();
    }
    shows.removeAll(QString());
    return shows;
}
}

MainWindow::MainWindow(const User& user, QWidget* parent)
    : QMainWindow(parent), m_user(user) {
    setWindowTitle("ðŸŽ¬ CineBook â€“ Movie Ticket Booking System");
    resize(1200, 780);
    setupUi();
    applyStyles();
    resetMovieDetail();
    loadMovies();
}

void MainWindow::setupUi() {
    auto* central = new QWidget();
    setCentralWidget(central);
    auto* mainLay = new QHBoxLayout(central);
    mainLay->setSpacing(0);
    mainLay->setContentsMargins(0, 0, 0, 0);

    setupSidebar();

    auto* sidebar = new QFrame();
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(280);
    auto* sbLay = new QVBoxLayout(sidebar);
    sbLay->setContentsMargins(0, 0, 0, 0);
    sbLay->setSpacing(0);

    auto* logoBar = new QFrame();
    logoBar->setObjectName("logoBar");
    logoBar->setFixedHeight(70);
    auto* logoLay = new QHBoxLayout(logoBar);
    auto* logoLbl = new QLabel("ðŸŽ¬ CineBook");
    logoLbl->setObjectName("sidebarLogo");
    logoLay->addWidget(logoLbl);

    m_welcomeLabel = new QLabel(QString("ðŸ‘¤ %1").arg(m_user.fullName.isEmpty() ? m_user.username : m_user.fullName));
    m_welcomeLabel->setObjectName("welcomeLabel");
    m_welcomeLabel->setContentsMargins(16, 8, 16, 8);

    auto* searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("ðŸ” Search movies...");
    searchEdit->setObjectName("searchEdit");
    auto* searchWrap = new QWidget();
    auto* sw = new QVBoxLayout(searchWrap);
    sw->setContentsMargins(12, 8, 12, 4);
    sw->addWidget(searchEdit);

    m_movieList = new QListWidget();
    m_movieList->setObjectName("movieList");
    m_movieList->setSpacing(2);

    auto* btnFrame = new QFrame();
    btnFrame->setObjectName("sidebarBtns");
    auto* btnLay = new QVBoxLayout(btnFrame);
    btnLay->setContentsMargins(12, 12, 12, 12);
    btnLay->setSpacing(8);

    auto* myBookingsBtn = new QPushButton("ðŸ“‹  My Bookings");
    myBookingsBtn->setObjectName("sideNavBtn");
    auto* logoutBtn = new QPushButton("ðŸšª  Logout");
    logoutBtn->setObjectName("logoutBtn");

    btnLay->addWidget(myBookingsBtn);
    btnLay->addWidget(logoutBtn);

    sbLay->addWidget(logoBar);
    sbLay->addWidget(m_welcomeLabel);
    sbLay->addWidget(searchWrap);
    sbLay->addWidget(m_movieList, 1);
    sbLay->addWidget(btnFrame);

    m_stack = new QStackedWidget();

    auto* detailScroll = new QScrollArea();
    detailScroll->setWidgetResizable(true);
    detailScroll->setFrameShape(QFrame::NoFrame);

    auto* detailContent = new QWidget();
    detailContent->setObjectName("detailContent");
    auto* detailLay = new QVBoxLayout(detailContent);
    detailLay->setContentsMargins(40, 32, 40, 32);
    detailLay->setSpacing(16);

    m_movieTitle = new QLabel("Select a movie from the list");
    m_movieTitle->setObjectName("detailTitle");
    m_movieTitle->setWordWrap(true);

    m_movieInfo = new QLabel();
    m_movieInfo->setObjectName("detailInfo");

    m_movieDesc = new QLabel();
    m_movieDesc->setObjectName("detailDesc");
    m_movieDesc->setWordWrap(true);

    auto* showRow = new QHBoxLayout();
    auto* showLabel = new QLabel("ðŸ• Show Time:");
    showLabel->setObjectName("fieldLabel");
    m_showTimeCombo = new QComboBox();
    m_showTimeCombo->setObjectName("showTimeCombo");
    m_showTimeCombo->setFixedWidth(180);
    showRow->addWidget(showLabel);
    showRow->addWidget(m_showTimeCombo);
    showRow->addStretch();

    auto* infoRow = new QHBoxLayout();
    m_priceLabel = new QLabel();
    m_priceLabel->setObjectName("priceLabel");
    m_availLabel = new QLabel();
    m_availLabel->setObjectName("availLabel");
    m_showCountLabel = new QLabel();
    m_showCountLabel->setObjectName("metaLabel");
    infoRow->addWidget(m_priceLabel);
    infoRow->addSpacing(24);
    infoRow->addWidget(m_availLabel);
    infoRow->addSpacing(24);
    infoRow->addWidget(m_showCountLabel);
    infoRow->addStretch();

    m_bookBtn = new QPushButton("ðŸŽŸ  Choose Seats & Book");
    m_bookBtn->setObjectName("bookBtn");
    m_bookBtn->setFixedHeight(50);
    m_bookBtn->setFixedWidth(260);
    m_bookBtn->setCursor(Qt::PointingHandCursor);

    detailLay->addWidget(m_movieTitle);
    detailLay->addWidget(m_movieInfo);
    detailLay->addWidget(m_movieDesc);
    detailLay->addSpacing(16);
    detailLay->addLayout(showRow);
    detailLay->addLayout(infoRow);
    detailLay->addSpacing(12);
    detailLay->addWidget(m_bookBtn, 0, Qt::AlignLeft);
    detailLay->addStretch();

    detailScroll->setWidget(detailContent);

    auto* bookingsPage = new QWidget();
    bookingsPage->setObjectName("bookingsPage");
    auto* bpLay = new QVBoxLayout(bookingsPage);
    bpLay->setContentsMargins(32, 24, 32, 24);

    auto* bpTitle = new QLabel("ðŸ“‹ My Bookings");
    bpTitle->setObjectName("pageTitle");
    m_bookingTable = new QTableWidget();
    m_bookingTable->setObjectName("bookingTable");
    m_bookingTable->setColumnCount(8);
    m_bookingTable->setHorizontalHeaderLabels({"#", "Movie", "Show Time", "Booked At", "Seats", "Tickets", "Total (LKR)", "Status"});
    m_bookingTable->horizontalHeader()->setStretchLastSection(true);
    m_bookingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bookingTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_bookingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_bookingTable->setAlternatingRowColors(true);
    m_bookingTable->verticalHeader()->setVisible(false);
    m_bookingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto* bookingActions = new QHBoxLayout();
    auto* refreshBookingsBtn = new QPushButton("ðŸ”„ Refresh");
    refreshBookingsBtn->setObjectName("backBtn");
    refreshBookingsBtn->setCursor(Qt::PointingHandCursor);
    m_viewReceiptBtn = new QPushButton("ðŸ§¾ View Receipt");
    m_viewReceiptBtn->setObjectName("backBtn");
    m_viewReceiptBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBookingBtn = new QPushButton("âœ• Cancel Selected");
    m_cancelBookingBtn->setObjectName("dangerInlineBtn");
    m_cancelBookingBtn->setCursor(Qt::PointingHandCursor);
    bookingActions->addWidget(refreshBookingsBtn);
    bookingActions->addWidget(m_viewReceiptBtn);
    bookingActions->addWidget(m_cancelBookingBtn);
    bookingActions->addStretch();

    auto* backBtn = new QPushButton("â† Back to Movies");
    backBtn->setObjectName("backBtn");
    backBtn->setFixedWidth(160);
    backBtn->setCursor(Qt::PointingHandCursor);

    bpLay->addWidget(bpTitle);
    bpLay->addWidget(m_bookingTable, 1);
    bpLay->addLayout(bookingActions);
    bpLay->addWidget(backBtn, 0, Qt::AlignLeft);

    m_stack->addWidget(detailScroll);
    m_stack->addWidget(bookingsPage);

    mainLay->addWidget(sidebar);
    mainLay->addWidget(m_stack, 1);

    connect(m_movieList, &QListWidget::currentRowChanged, this, &MainWindow::onMovieSelected);
    connect(m_showTimeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onShowTimeChanged);
    connect(m_bookBtn, &QPushButton::clicked, this, &MainWindow::onBookNow);
    connect(myBookingsBtn, &QPushButton::clicked, this, &MainWindow::onMyBookings);
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearch);
    connect(backBtn, &QPushButton::clicked, [this] { m_stack->setCurrentIndex(0); });
    connect(refreshBookingsBtn, &QPushButton::clicked, this, &MainWindow::onMyBookings);
    connect(m_viewReceiptBtn, &QPushButton::clicked, this, &MainWindow::onViewReceipt);
    connect(m_cancelBookingBtn, &QPushButton::clicked, this, &MainWindow::onCancelBooking);
    connect(m_bookingTable, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem*) { onViewReceipt(); });
    connect(m_bookingTable, &QTableWidget::itemSelectionChanged, this, [this] {
        const bool hasSelection = m_bookingTable->currentRow() >= 0;
        m_viewReceiptBtn->setEnabled(hasSelection);
        m_cancelBookingBtn->setEnabled(hasSelection);
    });

    m_viewReceiptBtn->setEnabled(false);
    m_cancelBookingBtn->setEnabled(false);
}

void MainWindow::setupSidebar() {}
void MainWindow::setupMovieList() {}
void MainWindow::setupMovieDetail() {}

void MainWindow::applyStyles() {
    setStyleSheet(R"(
        QMainWindow, QWidget { background: #0f0f1a; color: #e0e0f0; }
        #sidebar { background: #12121f; border-right: 1px solid #1e1e35; }
        #logoBar { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #1a1a2e, stop:1 #0f3460); border-bottom: 1px solid #e94560; }
        #sidebarLogo { color: #ffffff; font-size: 18px; font-weight: 800; font-family: 'Georgia', serif; padding-left: 16px; letter-spacing: 2px; }
        #welcomeLabel { color: #8888aa; font-size: 12px; font-family: 'Segoe UI'; background: #0f0f1a; padding: 6px 16px; }
        #searchEdit { background: #1e1e35; border: 1px solid #2d2d4e; border-radius: 6px; color: #e0e0f0; padding: 8px 12px; font-size: 13px; font-family: 'Segoe UI'; }
        #searchEdit:focus { border-color: #e94560; }
        #movieList { background: transparent; border: none; outline: none; }
        #movieList::item { background: #1a1a2e; border-radius: 8px; padding: 12px 16px; color: #ccccee; font-size: 13px; font-family: 'Segoe UI'; margin: 2px 8px; }
        #movieList::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #e94560, stop:1 #c73652); color: white; }
        #movieList::item:hover:!selected { background: #22223a; }
        #sidebarBtns { background: #0f0f1a; border-top: 1px solid #1e1e35; }
        #sideNavBtn { background: #1e1e35; color: #ccccee; border: none; border-radius: 6px; padding: 10px; font-size: 13px; font-family: 'Segoe UI'; text-align: left; }
        #sideNavBtn:hover { background: #2d2d4e; }
        #logoutBtn { background: rgba(233,69,96,0.15); color: #e94560; border: 1px solid rgba(233,69,96,0.3); border-radius: 6px; padding: 10px; font-size: 13px; font-family: 'Segoe UI'; text-align: left; }
        #logoutBtn:hover { background: rgba(233,69,96,0.25); }
        #detailContent { background: #0f0f1a; }
        QScrollArea { background: #0f0f1a; border: none; }
        #detailTitle { color: #ffffff; font-size: 28px; font-weight: 800; font-family: 'Georgia', serif; }
        #detailInfo { color: #8888aa; font-size: 13px; font-family: 'Segoe UI'; letter-spacing: 1px; }
        #detailDesc { color: #aaaacc; font-size: 14px; font-family: 'Segoe UI'; line-height: 1.6; }
        #fieldLabel { color: #8888aa; font-size: 13px; font-weight: 600; font-family: 'Segoe UI'; }
        #showTimeCombo { background: #1e1e35; border: 1.5px solid #2d2d4e; border-radius: 6px; color: #e0e0f0; padding: 6px 10px; font-size: 13px; font-family: 'Segoe UI'; }
        #priceLabel { color: #4ade80; font-size: 18px; font-weight: 700; font-family: 'Segoe UI'; }
        #availLabel { color: #60a5fa; font-size: 14px; font-family: 'Segoe UI'; }
        #metaLabel { color: #f59e0b; font-size: 14px; font-family: 'Segoe UI'; }
        #bookBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #e94560, stop:1 #c73652); color: white; border: none; border-radius: 10px; font-size: 16px; font-weight: 700; font-family: 'Segoe UI'; letter-spacing: 1px; }
        #bookBtn:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ff5577, stop:1 #e94560); }
        #bookBtn:disabled { background: #2d2d4e; color: #555577; }
        #bookingsPage { background: #0f0f1a; }
        #pageTitle { color: #ffffff; font-size: 22px; font-weight: 700; font-family: 'Georgia', serif; margin-bottom: 12px; }
        #bookingTable { background: #12121f; border: 1px solid #1e1e35; border-radius: 8px; gridline-color: #1e1e35; color: #e0e0f0; font-family: 'Segoe UI'; font-size: 13px; alternate-background-color: #1a1a2e; }
        QHeaderView::section { background: #1e1e35; color: #8888aa; border: none; padding: 10px; font-size: 12px; font-weight: 600; letter-spacing: 1px; }
        #backBtn { background: #1e1e35; color: #ccccee; border: none; border-radius: 6px; padding: 8px 16px; font-size: 13px; font-family: 'Segoe UI'; }
        #backBtn:hover { background: #2d2d4e; }
        #dangerInlineBtn { background: rgba(233,69,96,0.15); color: #e94560; border: 1px solid rgba(233,69,96,0.3); border-radius: 6px; padding: 8px 16px; font-size: 13px; font-family: 'Segoe UI'; }
        #dangerInlineBtn:hover { background: rgba(233,69,96,0.3); }
        #dangerInlineBtn:disabled, #backBtn:disabled { background: #1e1e35; color: #555577; border-color: #2d2d4e; }
    )");
}

void MainWindow::resetMovieDetail() {
    m_selectedMovie = Movie();
    m_movieTitle->setText("Select a movie from the list");
    m_movieInfo->setText("Browse active shows and choose a time to continue.");
    m_movieDesc->setText("Your selected movie details will appear here.");
    m_priceLabel->setText("ðŸ’° LKR --");
    m_availLabel->setText("ðŸª‘ Availability will appear here");
    m_showCountLabel->setText("ðŸŽž Choose a movie");

    const QSignalBlocker blocker(m_showTimeCombo);
    m_showTimeCombo->clear();
    m_bookBtn->setEnabled(false);
}

void MainWindow::populateMovieList(const QVector<Movie>& movies) {
    m_visibleMovies = movies;
    m_movieList->clear();

    for (const Movie& movie : movies) {
        QString itemText = QString("ðŸŽ¬ %1\n   %2 Â· %3").arg(movie.title, movie.genre, movie.duration);
        m_movieList->addItem(itemText);
    }
}

void MainWindow::loadMovies() {
    m_movies = Database::instance().getAllMovies(true);
    onSearch(QString());
}

void MainWindow::onMovieSelected(int row) {
    if (row < 0 || row >= m_visibleMovies.size()) {
        resetMovieDetail();
        return;
    }

    m_selectedMovie = m_visibleMovies[row];
    updateMovieDetail(m_selectedMovie);
    m_stack->setCurrentIndex(0);
}

void MainWindow::updateMovieDetail(const Movie& movie) {
    m_movieTitle->setText(movie.title);
    m_movieInfo->setText(QString("â­ %1  |  ðŸŽ­ %2  |  â± %3").arg(movie.rating, movie.genre, movie.duration));
    m_movieDesc->setText(movie.description);
    m_priceLabel->setText(QString("ðŸ’° LKR %1 / ticket").arg(movie.ticketPrice, 0, 'f', 2));

    const QStringList times = splitShowTimes(movie.showTimes);
    m_showCountLabel->setText(QString("ðŸŽž %1 show time(s)").arg(times.size()));

    const QSignalBlocker blocker(m_showTimeCombo);
    m_showTimeCombo->clear();
    for (const QString& time : times) {
        m_showTimeCombo->addItem("ðŸ• " + time, time);
    }

    if (times.isEmpty()) {
        m_availLabel->setText("ðŸª‘ No scheduled show times");
        m_bookBtn->setEnabled(false);
        return;
    }

    m_bookBtn->setEnabled(true);
    onShowTimeChanged(0);
}

QString MainWindow::selectedShowTime() const {
    const QString showTime = m_showTimeCombo->currentData().toString().trimmed();
    if (!showTime.isEmpty()) {
        return showTime;
    }

    return m_showTimeCombo->currentText().replace("ðŸ• ", "").trimmed();
}

void MainWindow::onShowTimeChanged(int index) {
    if (m_selectedMovie.id == 0 || index < 0) {
        return;
    }

    const QString showTime = selectedShowTime();
    if (showTime.isEmpty()) {
        m_bookBtn->setEnabled(false);
        m_availLabel->setText("ðŸª‘ No show time selected");
        return;
    }

    const int availableSeats = Database::instance().countAvailableSeats(m_selectedMovie.id, showTime);
    if (availableSeats == 0) {
        m_availLabel->setText("ðŸš« Sold out for this show");
    } else {
        m_availLabel->setText(QString("ðŸª‘ %1 seats available").arg(availableSeats));
    }
}

void MainWindow::onBookNow() {
    if (m_selectedMovie.id == 0) {
        return;
    }

    const QString showTime = selectedShowTime();
    if (showTime.isEmpty()) {
        QMessageBox::information(this, "Select Show Time", "Please choose a show time before booking.");
        return;
    }

    const int availableSeats = Database::instance().countAvailableSeats(m_selectedMovie.id, showTime);
    if (availableSeats == 0) {
        QMessageBox::warning(this, "Fully Booked", "No seats available for this show time.");
        onShowTimeChanged(m_showTimeCombo->currentIndex());
        return;
    }

    SeatSelection seatDialog(m_selectedMovie, showTime, m_user, this);
    if (seatDialog.exec() == QDialog::Accepted) {
        const int bookingId = seatDialog.bookingId();
        if (bookingId > 0) {
            ReceiptDialog receipt(Database::instance().getBooking(bookingId), this);
            receipt.exec();
            onShowTimeChanged(m_showTimeCombo->currentIndex());
        }
    }
}

void MainWindow::loadBookingTable() {
    const QVector<Booking> bookings = Database::instance().getUserBookings(m_user.id);
    m_bookingTable->setRowCount(bookings.size());

    for (int row = 0; row < bookings.size(); ++row) {
        const Booking& booking = bookings[row];
        QStringList seats = booking.seats;
        std::sort(seats.begin(), seats.end(), seatCodeLessThan);

        m_bookingTable->setItem(row, 0, new QTableWidgetItem(QString::number(booking.id)));
        m_bookingTable->setItem(row, 1, new QTableWidgetItem(booking.movieTitle));
        m_bookingTable->setItem(row, 2, new QTableWidgetItem(booking.showTime));
        m_bookingTable->setItem(row, 3, new QTableWidgetItem(booking.bookedAt.isValid() ? booking.bookedAt.toString("dd MMM yyyy hh:mm AP") : "--"));
        m_bookingTable->setItem(row, 4, new QTableWidgetItem(seats.join(", ")));
        m_bookingTable->setItem(row, 5, new QTableWidgetItem(QString::number(booking.numTickets)));
        m_bookingTable->setItem(row, 6, new QTableWidgetItem(QString("LKR %1").arg(booking.totalAmount, 0, 'f', 2)));

        auto* statusItem = new QTableWidgetItem(booking.status.toUpper());
        statusItem->setForeground(booking.status == "confirmed" ? QColor("#4ade80") : QColor("#f87171"));
        m_bookingTable->setItem(row, 7, statusItem);
    }

    if (bookings.isEmpty()) {
        m_viewReceiptBtn->setEnabled(false);
        m_cancelBookingBtn->setEnabled(false);
        return;
    }

    m_bookingTable->selectRow(0);
    m_viewReceiptBtn->setEnabled(true);
    m_cancelBookingBtn->setEnabled(true);
}

void MainWindow::onMyBookings() {
    loadBookingTable();
    m_stack->setCurrentIndex(1);
}

void MainWindow::onViewReceipt() {
    const int row = m_bookingTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Booking", "Please select a booking first.");
        return;
    }

    const int bookingId = m_bookingTable->item(row, 0)->text().toInt();
    const Booking booking = Database::instance().getBooking(bookingId);
    if (booking.id == 0) {
        QMessageBox::warning(this, "Missing Booking", "This booking could not be loaded.");
        return;
    }

    ReceiptDialog receipt(booking, this);
    receipt.exec();
}

void MainWindow::onCancelBooking() {
    const int row = m_bookingTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Booking", "Please select a booking first.");
        return;
    }

    const int bookingId = m_bookingTable->item(row, 0)->text().toInt();
    const QString status = m_bookingTable->item(row, 7)->text().trimmed().toLower();
    if (status == "cancelled") {
        QMessageBox::information(this, "Already Cancelled", "This booking has already been cancelled.");
        return;
    }

    if (QMessageBox::question(this, "Cancel Booking", "Do you want to cancel the selected booking and release its seats?") != QMessageBox::Yes) {
        return;
    }

    if (!Database::instance().cancelBooking(bookingId)) {
        QMessageBox::warning(this, "Cancellation Failed", Database::instance().lastError());
        return;
    }

    loadBookingTable();
    if (m_selectedMovie.id != 0) {
        onShowTimeChanged(m_showTimeCombo->currentIndex());
    }
}

void MainWindow::onLogout() {
    if (QMessageBox::question(this, "Logout", "Are you sure you want to logout?") != QMessageBox::Yes) {
        return;
    }

    close();
    LoginWindow login;
    if (login.exec() == QDialog::Accepted) {
        MainWindow* window = new MainWindow(login.loggedInUser());
        window->show();
    }
}

void MainWindow::onSearch(const QString& text) {
    QVector<Movie> filteredMovies;
    for (const Movie& movie : m_movies) {
        if (text.trimmed().isEmpty()
            || movie.title.contains(text, Qt::CaseInsensitive)
            || movie.genre.contains(text, Qt::CaseInsensitive)
            || movie.rating.contains(text, Qt::CaseInsensitive)) {
            filteredMovies.append(movie);
        }
    }

    const int currentMovieId = m_selectedMovie.id;
    populateMovieList(filteredMovies);

    if (m_visibleMovies.isEmpty()) {
        resetMovieDetail();
        return;
    }

    int selectedRow = 0;
    for (int row = 0; row < m_visibleMovies.size(); ++row) {
        if (m_visibleMovies[row].id == currentMovieId) {
            selectedRow = row;
            break;
        }
    }

    m_movieList->setCurrentRow(selectedRow);
}
