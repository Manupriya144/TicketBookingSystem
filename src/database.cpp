#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QVariant>
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

QString seatRowCode(int rowIndex) {
    QString code;
    int index = rowIndex;
    do {
        code.prepend(QChar('A' + (index % 26)));
        index = (index / 26) - 1;
    } while (index >= 0);
    return code;
}
}

// ─── Singleton ────────────────────────────────────────────────────────────────
Database& Database::instance() {
    static Database db;
    return db;
}

// ─── Init ─────────────────────────────────────────────────────────────────────
bool Database::init(const QString& dbPath) {
    if (m_db.isOpen()) {
        return true;
    }

    if (QSqlDatabase::contains("cinebook")) {
        m_db = QSqlDatabase::database("cinebook");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "cinebook");
    }

    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    m_dbPath = dbPath;
    m_lastError.clear();

    // Enable foreign keys
    QSqlQuery q(m_db);
    q.exec("PRAGMA foreign_keys = ON");
    q.exec("PRAGMA journal_mode = WAL");

    createTables();
    seedDefaultData();
    return true;
}

bool Database::isOpen() const { return m_db.isOpen(); }
QString Database::lastError() const { return m_lastError; }
QString Database::databasePath() const { return m_dbPath; }

// ─── Create Tables ────────────────────────────────────────────────────────────
void Database::createTables() {
    QSqlQuery q(m_db);

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            username  TEXT UNIQUE NOT NULL,
            password  TEXT NOT NULL,
            role      TEXT NOT NULL DEFAULT 'user',
            full_name TEXT,
            email     TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )");

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS movies (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            title        TEXT NOT NULL,
            genre        TEXT,
            duration     TEXT,
            description  TEXT,
            ticket_price REAL DEFAULT 0.0,
            show_times   TEXT,
            total_seats  INTEGER DEFAULT 60,
            is_active    INTEGER DEFAULT 1,
            rating       TEXT
        )
    )");

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS seats (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            movie_id   INTEGER NOT NULL,
            show_time  TEXT NOT NULL,
            seat_code  TEXT NOT NULL,
            is_booked  INTEGER DEFAULT 0,
            FOREIGN KEY(movie_id) REFERENCES movies(id) ON DELETE CASCADE,
            UNIQUE(movie_id, show_time, seat_code)
        )
    )");

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS bookings (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id      INTEGER NOT NULL,
            movie_id     INTEGER NOT NULL,
            show_time    TEXT NOT NULL,
            seats        TEXT NOT NULL,
            num_tickets  INTEGER NOT NULL,
            total_amount REAL NOT NULL,
            booked_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
            status       TEXT DEFAULT 'confirmed',
            FOREIGN KEY(user_id)  REFERENCES users(id),
            FOREIGN KEY(movie_id) REFERENCES movies(id)
        )
    )");

    q.exec("CREATE INDEX IF NOT EXISTS idx_movies_active_title ON movies(is_active, title)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_seats_lookup ON seats(movie_id, show_time, is_booked)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_bookings_user_time ON bookings(user_id, booked_at DESC)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_bookings_movie_time ON bookings(movie_id, show_time)");
}

// ─── Seed Data ────────────────────────────────────────────────────────────────
void Database::seedDefaultData() {
    QSqlQuery q(m_db);
    q.exec("SELECT COUNT(*) FROM users");
    if (q.next() && q.value(0).toInt() > 0) return;   // already seeded

    // Default admin
    QString adminPwd = QCryptographicHash::hash("admin123", QCryptographicHash::Sha256).toHex();
    q.prepare("INSERT INTO users (username,password,role,full_name,email) VALUES (?,?,?,?,?)");
    q.addBindValue("admin");
    q.addBindValue(adminPwd);
    q.addBindValue("admin");
    q.addBindValue("System Administrator");
    q.addBindValue("admin@cinema.lk");
    q.exec();

    // Default user
    QString userPwd = QCryptographicHash::hash("user123", QCryptographicHash::Sha256).toHex();
    q.prepare("INSERT INTO users (username,password,role,full_name,email) VALUES (?,?,?,?,?)");
    q.addBindValue("user1");
    q.addBindValue(userPwd);
    q.addBindValue("user");
    q.addBindValue("Test User");
    q.addBindValue("user@cinema.lk");
    q.exec();

    // Sample movies
    struct SampleMovie { QString title,genre,dur,desc,rating,shows; double price; int seats; };
    QVector<SampleMovie> movies = {
        {"Avengers: Endgame","Action","3h 2m","The Avengers assemble once more.","PG-13","10:00,14:00,18:00,21:30",1200.0,60},
        {"The Lion King","Animation","1h 58m","A young lion cub's journey to become king.","G","09:00,13:00,17:00",900.0,60},
        {"Inception","Sci-Fi","2h 28m","A thief who enters dreams to steal secrets.","PG-13","11:00,15:30,20:00",1100.0,60},
        {"Interstellar","Sci-Fi","2h 49m","Astronauts travel through a wormhole.","PG","12:00,16:00,20:30",1100.0,60},
        {"Joker","Drama","2h 2m","The origin story of the iconic villain.","R","13:00,17:00,21:00",1000.0,60},
    };

    for (auto& m : movies) {
        q.prepare(R"(INSERT INTO movies(title,genre,duration,description,ticket_price,show_times,total_seats,rating)
                     VALUES(?,?,?,?,?,?,?,?))");
        q.addBindValue(m.title);  q.addBindValue(m.genre);  q.addBindValue(m.dur);
        q.addBindValue(m.desc);   q.addBindValue(m.price);  q.addBindValue(m.shows);
        q.addBindValue(m.seats);  q.addBindValue(m.rating);
        q.exec();
        int movieId = q.lastInsertId().toInt();

        // Init seats for each show time
        for (const QString& show : splitShowTimes(m.shows)) {
            initSeatsForMovie(movieId, show, m.seats);
        }
    }
}

// ─── User Operations ──────────────────────────────────────────────────────────
bool Database::createUser(const User& user) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO users(username,password,role,full_name,email) VALUES(?,?,?,?,?)");
    q.addBindValue(user.username);
    q.addBindValue(user.password);
    q.addBindValue(user.role.isEmpty() ? "user" : user.role);
    q.addBindValue(user.fullName);
    q.addBindValue(user.email);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    return true;
}

User Database::getUserByUsername(const QString& username) {
    User u;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM users WHERE username=?");
    q.addBindValue(username);
    if (q.exec() && q.next()) {
        u.id       = q.value("id").toInt();
        u.username = q.value("username").toString();
        u.password = q.value("password").toString();
        u.role     = q.value("role").toString();
        u.fullName = q.value("full_name").toString();
        u.email    = q.value("email").toString();
    }
    return u;
}

bool Database::validateLogin(const QString& username, const QString& hashedPwd, User& outUser, const QString& plainPassword) {
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM users WHERE username=?");
    q.addBindValue(username);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }

    m_lastError.clear();

    if (q.next()) {
        const QString storedPassword = q.value("password").toString();
        if (storedPassword != hashedPwd) {
            if (plainPassword.isEmpty() || storedPassword != plainPassword) {
                return false;
            }

            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE users SET password=? WHERE id=?");
            updateQuery.addBindValue(hashedPwd);
            updateQuery.addBindValue(q.value("id").toInt());
            if (!updateQuery.exec()) {
                m_lastError = updateQuery.lastError().text();
                return false;
            }
        }

        outUser.id       = q.value("id").toInt();
        outUser.username = q.value("username").toString();
        outUser.role     = q.value("role").toString();
        outUser.fullName = q.value("full_name").toString();
        outUser.email    = q.value("email").toString();
        m_lastError.clear();
        return true;
    }

    return false;
}

QVector<User> Database::getAllUsers() {
    QVector<User> users;
    QSqlQuery q(m_db);
    q.exec("SELECT * FROM users ORDER BY id");
    while (q.next()) {
        User u;
        u.id       = q.value("id").toInt();
        u.username = q.value("username").toString();
        u.role     = q.value("role").toString();
        u.fullName = q.value("full_name").toString();
        u.email    = q.value("email").toString();
        users.append(u);
    }
    return users;
}

bool Database::updateUserPassword(int userId, const QString& newHashedPwd) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE users SET password=? WHERE id=?");
    q.addBindValue(newHashedPwd);
    q.addBindValue(userId);
    return q.exec();
}

// ─── Movie Operations ─────────────────────────────────────────────────────────
bool Database::addMovie(const Movie& movie) {
    QSqlQuery q(m_db);
    q.prepare(R"(INSERT INTO movies(title,genre,duration,description,ticket_price,show_times,total_seats,rating)
                 VALUES(?,?,?,?,?,?,?,?))");
    q.addBindValue(movie.title);       q.addBindValue(movie.genre);
    q.addBindValue(movie.duration);    q.addBindValue(movie.description);
    q.addBindValue(movie.ticketPrice); q.addBindValue(movie.showTimes);
    q.addBindValue(movie.totalSeats);  q.addBindValue(movie.rating);
    if (!q.exec()) { m_lastError = q.lastError().text(); return false; }
    int id = q.lastInsertId().toInt();
    for (const QString& show : splitShowTimes(movie.showTimes)) {
        initSeatsForMovie(id, show, movie.totalSeats);
    }
    return true;
}

bool Database::updateMovie(const Movie& movie) {
    QSqlQuery q(m_db);
    q.prepare(R"(UPDATE movies SET title=?,genre=?,duration=?,description=?,ticket_price=?,
                 show_times=?,total_seats=?,rating=?,is_active=? WHERE id=?)");
    q.addBindValue(movie.title);       q.addBindValue(movie.genre);
    q.addBindValue(movie.duration);    q.addBindValue(movie.description);
    q.addBindValue(movie.ticketPrice); q.addBindValue(movie.showTimes);
    q.addBindValue(movie.totalSeats);  q.addBindValue(movie.rating);
    q.addBindValue(movie.isActive ? 1 : 0);
    q.addBindValue(movie.id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }

    for (const QString& show : splitShowTimes(movie.showTimes)) {
        initSeatsForMovie(movie.id, show, movie.totalSeats);
    }
    return true;
}

bool Database::deleteMovie(int movieId) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE movies SET is_active=0 WHERE id=?");
    q.addBindValue(movieId);
    return q.exec();
}

Movie Database::getMovie(int movieId) {
    Movie m;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM movies WHERE id=?");
    q.addBindValue(movieId);
    if (q.exec() && q.next()) {
        m.id          = q.value("id").toInt();
        m.title       = q.value("title").toString();
        m.genre       = q.value("genre").toString();
        m.duration    = q.value("duration").toString();
        m.description = q.value("description").toString();
        m.ticketPrice = q.value("ticket_price").toDouble();
        m.showTimes   = q.value("show_times").toString();
        m.totalSeats  = q.value("total_seats").toInt();
        m.isActive    = q.value("is_active").toInt() == 1;
        m.rating      = q.value("rating").toString();
    }
    return m;
}

QVector<Movie> Database::getAllMovies(bool activeOnly) {
    QVector<Movie> list;
    QString sql = activeOnly
        ? "SELECT * FROM movies WHERE is_active=1 ORDER BY title"
        : "SELECT * FROM movies ORDER BY title";
    QSqlQuery q(m_db);
    q.exec(sql);
    while (q.next()) {
        Movie m;
        m.id          = q.value("id").toInt();
        m.title       = q.value("title").toString();
        m.genre       = q.value("genre").toString();
        m.duration    = q.value("duration").toString();
        m.description = q.value("description").toString();
        m.ticketPrice = q.value("ticket_price").toDouble();
        m.showTimes   = q.value("show_times").toString();
        m.totalSeats  = q.value("total_seats").toInt();
        m.isActive    = q.value("is_active").toInt() == 1;
        m.rating      = q.value("rating").toString();
        list.append(m);
    }
    return list;
}

// ─── Seat Operations ──────────────────────────────────────────────────────────
bool Database::initSeatsForMovie(int movieId, const QString& showTime, int totalSeats) {
    const int seatsPerRow = 10;
    for (int seatIndex = 0; seatIndex < totalSeats; ++seatIndex) {
        const int rowIndex = seatIndex / seatsPerRow;
        const int seatNumber = (seatIndex % seatsPerRow) + 1;
        QString code = seatRowCode(rowIndex) + QString::number(seatNumber);

        QSqlQuery q(m_db);
        q.prepare("INSERT OR IGNORE INTO seats(movie_id,show_time,seat_code,is_booked) VALUES(?,?,?,0)");
        q.addBindValue(movieId);
        q.addBindValue(showTime);
        q.addBindValue(code);
        if (!q.exec()) {
            m_lastError = q.lastError().text();
            return false;
        }
    }
    return true;
}

QVector<Seat> Database::getSeats(int movieId, const QString& showTime) {
    QVector<Seat> seats;
    QSqlQuery q(m_db);
    q.prepare(R"(
        SELECT *
        FROM seats
        WHERE movie_id=? AND show_time=?
        ORDER BY SUBSTR(seat_code, 1, 1), CAST(SUBSTR(seat_code, 2) AS INTEGER)
    )");
    q.addBindValue(movieId); q.addBindValue(showTime);
    if (q.exec()) {
        while (q.next()) {
            Seat s;
            s.id       = q.value("id").toInt();
            s.movieId  = movieId;
            s.showTime = showTime;
            s.seatCode = q.value("seat_code").toString();
            s.isBooked = q.value("is_booked").toInt() == 1;
            seats.append(s);
        }
    }
    return seats;
}

bool Database::bookSeat(int movieId, const QString& showTime, const QString& seatCode) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE seats SET is_booked=1 WHERE movie_id=? AND show_time=? AND seat_code=? AND is_booked=0");
    q.addBindValue(movieId); q.addBindValue(showTime); q.addBindValue(seatCode);
    return q.exec() && q.numRowsAffected() > 0;
}

bool Database::releaseSeat(int movieId, const QString& showTime, const QString& seatCode) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE seats SET is_booked=0 WHERE movie_id=? AND show_time=? AND seat_code=?");
    q.addBindValue(movieId); q.addBindValue(showTime); q.addBindValue(seatCode);
    return q.exec();
}

int Database::countAvailableSeats(int movieId, const QString& showTime) {
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM seats WHERE movie_id=? AND show_time=? AND is_booked=0");
    q.addBindValue(movieId); q.addBindValue(showTime);
    if (q.exec() && q.next()) return q.value(0).toInt();
    return 0;
}

// ─── Booking Operations ───────────────────────────────────────────────────────
int Database::createBookingWithSeats(const Booking& b) {
    if (b.seats.isEmpty()) {
        m_lastError = "Please select at least one seat.";
        return -1;
    }

    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return -1;
    }

    for (const QString& seatCode : b.seats) {
        QSqlQuery seatQuery(m_db);
        seatQuery.prepare("UPDATE seats SET is_booked=1 WHERE movie_id=? AND show_time=? AND seat_code=? AND is_booked=0");
        seatQuery.addBindValue(b.movieId);
        seatQuery.addBindValue(b.showTime);
        seatQuery.addBindValue(seatCode);
        if (!seatQuery.exec()) {
            m_lastError = seatQuery.lastError().text();
            m_db.rollback();
            return -1;
        }
        if (seatQuery.numRowsAffected() == 0) {
            m_lastError = QString("Seat %1 is no longer available.").arg(seatCode);
            m_db.rollback();
            return -1;
        }
    }

    QSqlQuery bookingQuery(m_db);
    bookingQuery.prepare(R"(INSERT INTO bookings(user_id,movie_id,show_time,seats,num_tickets,total_amount,status)
                            VALUES(?,?,?,?,?,?,?))");
    bookingQuery.addBindValue(b.userId);
    bookingQuery.addBindValue(b.movieId);
    bookingQuery.addBindValue(b.showTime);
    bookingQuery.addBindValue(b.seats.join(","));
    bookingQuery.addBindValue(b.numTickets);
    bookingQuery.addBindValue(b.totalAmount);
    bookingQuery.addBindValue("confirmed");

    if (!bookingQuery.exec()) {
        m_lastError = bookingQuery.lastError().text();
        m_db.rollback();
        return -1;
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return -1;
    }

    m_lastError.clear();
    return bookingQuery.lastInsertId().toInt();
}

int Database::createBooking(const Booking& b) {
    QSqlQuery q(m_db);
    q.prepare(R"(INSERT INTO bookings(user_id,movie_id,show_time,seats,num_tickets,total_amount,status)
                 VALUES(?,?,?,?,?,?,?))");
    q.addBindValue(b.userId);
    q.addBindValue(b.movieId);
    q.addBindValue(b.showTime);
    q.addBindValue(b.seats.join(","));
    q.addBindValue(b.numTickets);
    q.addBindValue(b.totalAmount);
    q.addBindValue("confirmed");
    if (!q.exec()) { m_lastError = q.lastError().text(); return -1; }
    return q.lastInsertId().toInt();
}

QVector<Booking> Database::getUserBookings(int userId) {
    QVector<Booking> list;
    QSqlQuery q(m_db);
    q.prepare(R"(SELECT b.*, m.title FROM bookings b JOIN movies m ON b.movie_id=m.id
                 WHERE b.user_id=? ORDER BY b.booked_at DESC)");
    q.addBindValue(userId);
    if (q.exec()) {
        while (q.next()) {
            Booking b;
            b.id          = q.value("id").toInt();
            b.userId      = q.value("user_id").toInt();
            b.movieId     = q.value("movie_id").toInt();
            b.movieTitle  = q.value("title").toString();
            b.showTime    = q.value("show_time").toString();
            b.seats       = q.value("seats").toString().split(",");
            b.numTickets  = q.value("num_tickets").toInt();
            b.totalAmount = q.value("total_amount").toDouble();
            b.bookedAt    = q.value("booked_at").toDateTime();
            b.status      = q.value("status").toString();
            list.append(b);
        }
    }
    return list;
}

QVector<Booking> Database::getAllBookings() {
    QVector<Booking> list;
    QSqlQuery q(m_db);
    q.exec(R"(SELECT b.*, m.title FROM bookings b JOIN movies m ON b.movie_id=m.id ORDER BY b.booked_at DESC)");
    while (q.next()) {
        Booking b;
        b.id          = q.value("id").toInt();
        b.userId      = q.value("user_id").toInt();
        b.movieId     = q.value("movie_id").toInt();
        b.movieTitle  = q.value("title").toString();
        b.showTime    = q.value("show_time").toString();
        b.seats       = q.value("seats").toString().split(",");
        b.numTickets  = q.value("num_tickets").toInt();
        b.totalAmount = q.value("total_amount").toDouble();
        b.bookedAt    = q.value("booked_at").toDateTime();
        b.status      = q.value("status").toString();
        list.append(b);
    }
    return list;
}

bool Database::cancelBooking(int bookingId) {
    Booking booking = getBooking(bookingId);
    if (booking.id == 0) {
        m_lastError = "Booking not found.";
        return false;
    }
    if (booking.status == "cancelled") {
        m_lastError = "This booking is already cancelled.";
        return false;
    }

    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.prepare("UPDATE bookings SET status='cancelled' WHERE id=?");
    q.addBindValue(bookingId);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        m_db.rollback();
        return false;
    }

    for (const QString& seatCode : booking.seats) {
        QSqlQuery seatQuery(m_db);
        seatQuery.prepare("UPDATE seats SET is_booked=0 WHERE movie_id=? AND show_time=? AND seat_code=?");
        seatQuery.addBindValue(booking.movieId);
        seatQuery.addBindValue(booking.showTime);
        seatQuery.addBindValue(seatCode);
        if (!seatQuery.exec()) {
            m_lastError = seatQuery.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    m_lastError.clear();
    return true;
}

Booking Database::getBooking(int bookingId) {
    Booking b;
    QSqlQuery q(m_db);
    q.prepare("SELECT b.*, m.title FROM bookings b JOIN movies m ON b.movie_id=m.id WHERE b.id=?");
    q.addBindValue(bookingId);
    if (q.exec() && q.next()) {
        b.id          = q.value("id").toInt();
        b.userId      = q.value("user_id").toInt();
        b.movieId     = q.value("movie_id").toInt();
        b.movieTitle  = q.value("title").toString();
        b.showTime    = q.value("show_time").toString();
        b.seats       = q.value("seats").toString().split(",");
        b.numTickets  = q.value("num_tickets").toInt();
        b.totalAmount = q.value("total_amount").toDouble();
        b.bookedAt    = q.value("booked_at").toDateTime();
        b.status      = q.value("status").toString();
    }
    return b;
}
