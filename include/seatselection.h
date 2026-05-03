#pragma once
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include <QSet>
#include <QGridLayout>
#include "models.h"

class SeatButton : public QPushButton {
    Q_OBJECT
public:
    explicit SeatButton(const QString& code, bool booked, QWidget* parent = nullptr);
    QString seatCode() const { return m_code; }
    bool isBooked() const { return m_booked; }
    bool isChosen() const { return m_chosen; }
    void setChosen(bool c);
private:
    QString m_code;
    bool    m_booked;
    bool    m_chosen = false;
};

class SeatSelection : public QDialog {
    Q_OBJECT
public:
    explicit SeatSelection(const Movie& movie, const QString& showTime, const User& user, QWidget* parent = nullptr);
    int bookingId() const { return m_bookingId; }

private slots:
    void onSeatClicked(SeatButton* btn);
    void onConfirmBooking();

private:
    void setupUi();
    void applyStyles();
    void loadSeats();
    void updateSummary();

    Movie        m_movie;
    QString      m_showTime;
    User         m_user;
    int          m_bookingId = -1;

    QGridLayout*         m_seatGrid;
    QVector<SeatButton*> m_seatBtns;
    QSet<QString>        m_chosen;
    QLabel*              m_summaryLabel;
    QPushButton*         m_confirmBtn;
};
