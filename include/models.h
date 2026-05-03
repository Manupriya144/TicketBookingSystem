#pragma once
#include <QString>
#include <QDateTime>
#include <QVector>

// ─── User Model ───────────────────────────────────────────────────────────────
struct User {
    int    id       = 0;
    QString username;
    QString password;   // stored as SHA-256 hash
    QString role;       // "admin" | "user"
    QString fullName;
    QString email;
    QDateTime createdAt;
};

// ─── Movie Model ──────────────────────────────────────────────────────────────
struct Movie {
    int     id          = 0;
    QString title;
    QString genre;
    QString duration;   // e.g. "2h 15m"
    QString description;
    double  ticketPrice = 0.0;
    QString showTimes;  // comma-separated, e.g. "10:00,14:00,18:00"
    int     totalSeats  = 60;
    bool    isActive    = true;
    QString rating;     // e.g. "PG-13"
};

// ─── Seat Model ───────────────────────────────────────────────────────────────
struct Seat {
    int     id       = 0;
    int     movieId  = 0;
    QString showTime;
    QString seatCode;   // e.g. "A1", "B3"
    bool    isBooked = false;
};

// ─── Booking Model ────────────────────────────────────────────────────────────
struct Booking {
    int      id          = 0;
    int      userId      = 0;
    int      movieId     = 0;
    QString  movieTitle;
    QString  showTime;
    QVector<QString> seats;
    int      numTickets  = 0;
    double   totalAmount = 0.0;
    QDateTime bookedAt;
    QString  status;    // "confirmed" | "cancelled"
};
