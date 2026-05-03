#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVector>
#include "models.h"

class Database {
public:
    static Database& instance();

    bool init(const QString& dbPath = "movie_booking.db");
    bool isOpen() const;
    QString lastError() const;
    QString databasePath() const;

    // ── User operations ──────────────────────────────────────────────────────
    bool       createUser(const User& user);
    User       getUserByUsername(const QString& username);
    bool       validateLogin(const QString& username, const QString& hashedPassword, User& outUser, const QString& plainPassword = QString());
    QVector<User> getAllUsers();
    bool       updateUserPassword(int userId, const QString& newHashedPwd);

    // ── Movie operations ──────────────────────────────────────────────────────
    bool          addMovie(const Movie& movie);
    bool          updateMovie(const Movie& movie);
    bool          deleteMovie(int movieId);
    Movie         getMovie(int movieId);
    QVector<Movie> getAllMovies(bool activeOnly = true);

    // ── Seat operations ───────────────────────────────────────────────────────
    bool          initSeatsForMovie(int movieId, const QString& showTime, int totalSeats);
    QVector<Seat> getSeats(int movieId, const QString& showTime);
    bool          bookSeat(int movieId, const QString& showTime, const QString& seatCode);
    bool          releaseSeat(int movieId, const QString& showTime, const QString& seatCode);
    int           countAvailableSeats(int movieId, const QString& showTime);

    // ── Booking operations ────────────────────────────────────────────────────
    int           createBookingWithSeats(const Booking& booking);
    int           createBooking(const Booking& booking);
    QVector<Booking> getUserBookings(int userId);
    QVector<Booking> getAllBookings();
    bool          cancelBooking(int bookingId);
    Booking       getBooking(int bookingId);

private:
    Database() = default;
    ~Database() = default;
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void createTables();
    void seedDefaultData();

    QSqlDatabase m_db;
    QString      m_lastError;
    QString      m_dbPath;
};
