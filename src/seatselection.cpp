#include "seatselection.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QFrame>
#include <QLabel>
#include <QGroupBox>
#include <QStyle>
#include <algorithm>
#include <functional>

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
}

// ─── SeatButton ───────────────────────────────────────────────────────────────
SeatButton::SeatButton(const QString& code, bool booked, QWidget* parent)
    : QPushButton(code, parent), m_code(code), m_booked(booked) {
    setFixedSize(46, 38);
    setCursor(booked ? Qt::ArrowCursor : Qt::PointingHandCursor);
    setEnabled(!booked);
    if (booked) setObjectName("seatBooked");
    else setObjectName("seatAvailable");
    setToolTip(booked ? code + " (Booked)" : code + " (Available)");
}

void SeatButton::setChosen(bool c) {
    m_chosen = c;
    setObjectName(c ? "seatChosen" : "seatAvailable");
    // Force style refresh
    style()->unpolish(this);
    style()->polish(this);
}

// ─── SeatSelection Dialog ─────────────────────────────────────────────────────
SeatSelection::SeatSelection(const Movie& movie, const QString& showTime, const User& user, QWidget* parent)
    : QDialog(parent), m_movie(movie), m_showTime(showTime), m_user(user) {
    setWindowTitle(QString("Select Seats — %1 @ %2").arg(movie.title, showTime));
    setMinimumSize(700, 620);
    setupUi();
    applyStyles();
    loadSeats();
}

void SeatSelection::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    // Header
    auto* headerLay = new QVBoxLayout();
    auto* titleLbl = new QLabel(m_movie.title);
    titleLbl->setObjectName("dlgTitle");
    auto* infoLbl = new QLabel(QString("🕐 %1  |  💰 LKR %2 / ticket").arg(m_showTime).arg(m_movie.ticketPrice, 0, 'f', 2));
    infoLbl->setObjectName("dlgInfo");
    headerLay->addWidget(titleLbl);
    headerLay->addWidget(infoLbl);

    // Screen indicator
    auto* screenFrame = new QFrame();
    screenFrame->setObjectName("screenFrame");
    screenFrame->setFixedHeight(30);
    auto* screenLay = new QHBoxLayout(screenFrame);
    auto* screenLbl = new QLabel("▬▬▬▬▬▬▬▬▬  SCREEN  ▬▬▬▬▬▬▬▬▬");
    screenLbl->setObjectName("screenLabel");
    screenLbl->setAlignment(Qt::AlignCenter);
    screenLay->addWidget(screenLbl);

    // Legend
    auto* legendLay = new QHBoxLayout();
    auto mkLegend = [](const QString& color, const QString& label) {
        auto* w = new QWidget();
        auto* l = new QHBoxLayout(w);
        l->setSpacing(6);
        auto* dot = new QLabel("■");
        dot->setStyleSheet(QString("color: %1; font-size: 16px;").arg(color));
        auto* lbl = new QLabel(label);
        lbl->setStyleSheet("color: #8888aa; font-size: 12px; font-family: 'Segoe UI';");
        l->addWidget(dot); l->addWidget(lbl);
        return w;
    };
    legendLay->addStretch();
    legendLay->addWidget(mkLegend("#2d2d4e", "Available"));
    legendLay->addSpacing(16);
    legendLay->addWidget(mkLegend("#e94560", "Selected"));
    legendLay->addSpacing(16);
    legendLay->addWidget(mkLegend("#555577", "Booked"));
    legendLay->addStretch();

    // Seat grid
    auto* gridWrap = new QScrollArea();
    gridWrap->setWidgetResizable(true);
    gridWrap->setObjectName("gridWrap");
    auto* gridWidget = new QWidget();
    m_seatGrid = new QGridLayout(gridWidget);
    m_seatGrid->setSpacing(6);
    m_seatGrid->setContentsMargins(16, 16, 16, 16);
    gridWrap->setWidget(gridWidget);

    // Summary + confirm
    auto* bottomFrame = new QFrame();
    bottomFrame->setObjectName("bottomFrame");
    auto* bottomLay = new QHBoxLayout(bottomFrame);
    m_summaryLabel = new QLabel("Select seats to continue");
    m_summaryLabel->setObjectName("summaryLabel");
    m_confirmBtn = new QPushButton("✓  Confirm Booking");
    m_confirmBtn->setObjectName("confirmBtn");
    m_confirmBtn->setFixedHeight(46);
    m_confirmBtn->setFixedWidth(200);
    m_confirmBtn->setEnabled(false);
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    bottomLay->addWidget(m_summaryLabel, 1);
    bottomLay->addWidget(m_confirmBtn);

    root->addLayout(headerLay);
    root->addWidget(screenFrame);
    root->addLayout(legendLay);
    root->addWidget(gridWrap, 1);
    root->addWidget(bottomFrame);

    connect(m_confirmBtn, &QPushButton::clicked, this, &SeatSelection::onConfirmBooking);
}

void SeatSelection::applyStyles() {
    setStyleSheet(R"(
        QDialog { background: #0f0f1a; color: #e0e0f0; }
        #dlgTitle { color: #ffffff; font-size: 20px; font-weight: 800; font-family: 'Georgia'; }
        #dlgInfo  { color: #8888aa; font-size: 13px; font-family: 'Segoe UI'; }
        #screenFrame { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1e1e35, stop:1 #12121f); border-radius: 4px; border: 1px solid #2d2d4e; }
        #screenLabel { color: #555577; font-size: 11px; letter-spacing: 2px; font-family: 'Consolas'; }
        #gridWrap { background: #12121f; border: 1px solid #1e1e35; border-radius: 8px; }
        #seatAvailable { background: #1e1e35; color: #8888aa; border: 1px solid #2d2d4e; border-radius: 6px; font-size: 10px; font-family: 'Consolas'; }
        #seatAvailable:hover { background: #2d2d4e; border-color: #e94560; color: #ffffff; }
        #seatChosen { background: #e94560; color: #ffffff; border: 1px solid #ff6b80; border-radius: 6px; font-size: 10px; font-family: 'Consolas'; font-weight: bold; }
        #seatBooked { background: #1a1a2e; color: #333355; border: 1px solid #1e1e35; border-radius: 6px; font-size: 10px; font-family: 'Consolas'; }
        #bottomFrame { background: #12121f; border-top: 1px solid #1e1e35; border-radius: 8px; padding: 12px; }
        #summaryLabel { color: #ccccee; font-size: 14px; font-family: 'Segoe UI'; }
        #confirmBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #e94560, stop:1 #c73652); color: white; border: none; border-radius: 8px; font-size: 14px; font-weight: 700; font-family: 'Segoe UI'; }
        #confirmBtn:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ff5577, stop:1 #e94560); }
        #confirmBtn:disabled { background: #2d2d4e; color: #555577; }
    )");
}

void SeatSelection::loadSeats() {
    while (QLayoutItem* item = m_seatGrid->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
    m_seatBtns.clear();

    QVector<Seat> seats = Database::instance().getSeats(m_movie.id, m_showTime);

    // Group by row
    QMap<QChar, QVector<Seat>> rows;
    for (const Seat& s : seats) {
        QChar row = s.seatCode[0];
        rows[row].append(s);
    }

    int gridRow = 0;
    for (auto it = rows.begin(); it != rows.end(); ++it, ++gridRow) {
        // Row label
        auto* rowLabel = new QLabel(QString(it.key()));
        rowLabel->setStyleSheet("color: #555577; font-size: 12px; font-family: 'Consolas'; font-weight: bold;");
        rowLabel->setFixedWidth(20);
        rowLabel->setAlignment(Qt::AlignCenter);
        m_seatGrid->addWidget(rowLabel, gridRow, 0);

        // Aisle spacer columns
        const QVector<Seat>& rowSeats = it.value();
        for (int col = 0; col < rowSeats.size(); ++col) {
            int gridCol = col + 1;
            if (col == 4) { // aisle
                auto* spacer = new QLabel(" ");
                spacer->setFixedWidth(16);
                m_seatGrid->addWidget(spacer, gridRow, gridCol);
                gridCol++;
            }
            auto* btn = new SeatButton(rowSeats[col].seatCode, rowSeats[col].isBooked);
            m_seatBtns.append(btn);
            connect(btn, &QPushButton::clicked, this, [this, btn]{ onSeatClicked(btn); });
            m_seatGrid->addWidget(btn, gridRow, gridCol + (col >= 4 ? 1 : 0));
        }
    }
}

void SeatSelection::onSeatClicked(SeatButton* btn) {
    if (btn->isBooked()) return;
    if (btn->isChosen()) {
        m_chosen.remove(btn->seatCode());
        btn->setChosen(false);
    } else {
        if (m_chosen.size() >= 8) {
            QMessageBox::information(this, "Limit Reached", "You can book a maximum of 8 seats at once.");
            return;
        }
        m_chosen.insert(btn->seatCode());
        btn->setChosen(true);
    }
    updateSummary();
}

void SeatSelection::updateSummary() {
    if (m_chosen.isEmpty()) {
        m_summaryLabel->setText("Select seats to continue");
        m_confirmBtn->setEnabled(false);
        return;
    }
    QStringList chosen = m_chosen.values();
    std::sort(chosen.begin(), chosen.end(), seatCodeLessThan);
    double total = m_chosen.size() * m_movie.ticketPrice;
    m_summaryLabel->setText(QString("%1 seat(s) selected: %2  |  Total: LKR %3")
        .arg(m_chosen.size()).arg(chosen.join(", ")).arg(total, 0, 'f', 2));
    m_confirmBtn->setEnabled(true);
}

void SeatSelection::onConfirmBooking() {
    if (m_chosen.isEmpty()) return;

    QStringList chosenList = m_chosen.values();
    std::sort(chosenList.begin(), chosenList.end(), seatCodeLessThan);

    // Create booking record
    Booking b;
    b.userId      = m_user.id;
    b.movieId     = m_movie.id;
    b.movieTitle  = m_movie.title;
    b.showTime    = m_showTime;
    b.seats       = chosenList;
    b.numTickets  = chosenList.size();
    b.totalAmount = chosenList.size() * m_movie.ticketPrice;

    m_bookingId = Database::instance().createBookingWithSeats(b);
    if (m_bookingId < 0) {
        QMessageBox::warning(this, "Booking Failed", Database::instance().lastError());
        m_chosen.clear();
        loadSeats();
        updateSummary();
        return;
    }
    accept();
}
