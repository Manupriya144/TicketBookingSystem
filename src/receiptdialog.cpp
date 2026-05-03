#include "receiptdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QTextEdit>
#include <QDateTime>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QFont>
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
}

ReceiptDialog::ReceiptDialog(const Booking& booking, QWidget* parent)
    : QDialog(parent), m_booking(booking) {
    setWindowTitle("🎟 Booking Confirmed!");
    setFixedSize(480, 600);
    setupUi();
    applyStyles();
}

void ReceiptDialog::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header
    auto* header = new QFrame();
    header->setObjectName("receiptHeader");
    header->setFixedHeight(100);
    auto* hLay = new QVBoxLayout(header);
    hLay->setAlignment(Qt::AlignCenter);
    auto* checkLbl = new QLabel("✅");
    checkLbl->setAlignment(Qt::AlignCenter);
    checkLbl->setStyleSheet("font-size: 32px;");
    auto* confirmedLbl = new QLabel("BOOKING CONFIRMED!");
    confirmedLbl->setObjectName("confirmedLabel");
    confirmedLbl->setAlignment(Qt::AlignCenter);
    hLay->addWidget(checkLbl);
    hLay->addWidget(confirmedLbl);

    // Receipt body
    auto* body = new QFrame();
    body->setObjectName("receiptBody");
    auto* bodyLay = new QVBoxLayout(body);
    bodyLay->setContentsMargins(32, 24, 32, 24);
    bodyLay->setSpacing(0);

    auto mkRow = [&](const QString& label, const QString& value, bool highlight = false) {
        auto* row = new QHBoxLayout();
        auto* lbl = new QLabel(label);
        lbl->setObjectName("rcptLabel");
        auto* val = new QLabel(value);
        val->setObjectName(highlight ? "rcptValueHL" : "rcptValue");
        val->setWordWrap(true);
        val->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        row->addWidget(lbl);
        row->addStretch();
        row->addWidget(val);

        auto* sep = new QFrame();
        sep->setObjectName("rcptSep");
        sep->setFixedHeight(1);

        bodyLay->addLayout(row);
        bodyLay->addWidget(sep);
        bodyLay->addSpacing(8);
    };

    // Booking ID
    auto* bookingIdLbl = new QLabel(QString("Booking #%1").arg(m_booking.id));
    bookingIdLbl->setObjectName("bookingIdLabel");
    bookingIdLbl->setAlignment(Qt::AlignCenter);
    bodyLay->addWidget(bookingIdLbl);
    bodyLay->addSpacing(16);

    auto* divider = new QFrame();
    divider->setObjectName("rcptDivider");
    divider->setFixedHeight(2);
    bodyLay->addWidget(divider);
    bodyLay->addSpacing(12);

    QStringList sortedSeats = m_booking.seats;
    std::sort(sortedSeats.begin(), sortedSeats.end(), seatCodeLessThan);

    mkRow("Movie",      m_booking.movieTitle);
    mkRow("Show Time",  m_booking.showTime);
    mkRow("Seats",      sortedSeats.join(", "));
    mkRow("Tickets",    QString::number(m_booking.numTickets));
    mkRow("Date",       (m_booking.bookedAt.isValid() ? m_booking.bookedAt : QDateTime::currentDateTime()).toString("dd MMM yyyy, hh:mm AP"));
    mkRow("Status",     "✅ CONFIRMED");

    bodyLay->addSpacing(8);
    auto* divider2 = new QFrame();
    divider2->setObjectName("rcptDivider");
    divider2->setFixedHeight(2);
    bodyLay->addWidget(divider2);
    bodyLay->addSpacing(12);

    // Total
    auto* totalRow = new QHBoxLayout();
    auto* totalLbl = new QLabel("TOTAL AMOUNT");
    totalLbl->setObjectName("totalLabel");
    auto* totalVal = new QLabel(QString("LKR %1").arg(m_booking.totalAmount, 0, 'f', 2));
    totalVal->setObjectName("totalValue");
    totalRow->addWidget(totalLbl);
    totalRow->addStretch();
    totalRow->addWidget(totalVal);
    bodyLay->addLayout(totalRow);

    bodyLay->addStretch();

    // Buttons
    auto* btnRow = new QHBoxLayout();
    auto* printBtn = new QPushButton("🖨  Print Receipt");
    printBtn->setObjectName("printBtn");
    printBtn->setFixedHeight(42);
    printBtn->setCursor(Qt::PointingHandCursor);
    auto* closeBtn = new QPushButton("Close");
    closeBtn->setObjectName("closeBtn");
    closeBtn->setFixedHeight(42);
    closeBtn->setCursor(Qt::PointingHandCursor);
    btnRow->addWidget(printBtn);
    btnRow->addWidget(closeBtn);

    root->addWidget(header);
    root->addWidget(body, 1);

    auto* btnWrap = new QWidget();
    btnWrap->setObjectName("btnWrap");
    auto* bwLay = new QHBoxLayout(btnWrap);
    bwLay->setContentsMargins(32, 12, 32, 16);
    bwLay->setSpacing(12);
    bwLay->addWidget(printBtn);
    bwLay->addWidget(closeBtn);
    root->addWidget(btnWrap);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(printBtn, &QPushButton::clicked, this, &ReceiptDialog::printReceipt);
}

void ReceiptDialog::applyStyles() {
    setStyleSheet(R"(
        QDialog { background: #0f0f1a; }
        #receiptHeader {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1a1a2e, stop:1 #0f3460);
            border-bottom: 2px solid #4ade80;
        }
        #confirmedLabel { color: #4ade80; font-size: 16px; font-weight: 800; font-family: 'Segoe UI'; letter-spacing: 3px; }
        #receiptBody { background: #12121f; }
        #bookingIdLabel { color: #e94560; font-size: 18px; font-weight: 700; font-family: 'Consolas'; letter-spacing: 2px; }
        #rcptLabel { color: #8888aa; font-size: 13px; font-family: 'Segoe UI'; padding: 6px 0; }
        #rcptValue { color: #ccccee; font-size: 13px; font-family: 'Segoe UI'; font-weight: 600; }
        #rcptValueHL { color: #4ade80; font-size: 13px; font-family: 'Segoe UI'; font-weight: 700; }
        #rcptSep { background: #1e1e35; }
        #rcptDivider { background: #2d2d4e; }
        #totalLabel { color: #ffffff; font-size: 15px; font-weight: 800; font-family: 'Segoe UI'; letter-spacing: 1px; }
        #totalValue { color: #4ade80; font-size: 20px; font-weight: 900; font-family: 'Georgia'; }
        #btnWrap { background: #0f0f1a; border-top: 1px solid #1e1e35; }
        #printBtn { background: #1e1e35; color: #ccccee; border: 1px solid #2d2d4e; border-radius: 8px; font-size: 13px; font-family: 'Segoe UI'; }
        #printBtn:hover { background: #2d2d4e; }
        #closeBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #e94560, stop:1 #c73652); color: white; border: none; border-radius: 8px; font-size: 13px; font-weight: 700; font-family: 'Segoe UI'; }
        #closeBtn:hover { background: #ff5577; }
    )");
}

void ReceiptDialog::printReceipt() {
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog printDlg(&printer, this);
    if (printDlg.exec() != QDialog::Accepted) return;

    QStringList sortedSeats = m_booking.seats;
    std::sort(sortedSeats.begin(), sortedSeats.end(), seatCodeLessThan);

    QString html = QString(R"(
        <html><body style='font-family: Arial; padding: 20px;'>
        <h1 style='text-align:center; color:#c73652;'>🎬 CineBook</h1>
        <h2 style='text-align:center;'>Movie Ticket Receipt</h2>
        <hr>
        <p><b>Booking ID:</b> #%1</p>
        <p><b>Movie:</b> %2</p>
        <p><b>Show Time:</b> %3</p>
        <p><b>Seats:</b> %4</p>
        <p><b>Number of Tickets:</b> %5</p>
        <p><b>Date:</b> %6</p>
        <hr>
        <h3><b>Total: LKR %7</b></h3>
        <p style='color:green;'><b>Status: CONFIRMED ✅</b></p>
        <hr>
        <p style='font-size:10px; color:gray;'>Thank you for booking with CineBook!
        Please arrive 15 minutes before the show.</p>
        </body></html>
    )").arg(m_booking.id)
       .arg(m_booking.movieTitle)
       .arg(m_booking.showTime)
       .arg(sortedSeats.join(", "))
       .arg(m_booking.numTickets)
       .arg((m_booking.bookedAt.isValid() ? m_booking.bookedAt : QDateTime::currentDateTime()).toString("dd MMM yyyy, hh:mm AP"))
       .arg(m_booking.totalAmount, 0, 'f', 2);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);
}
