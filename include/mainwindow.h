#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>
#include <QTableWidget>
#include "models.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const User& user, QWidget* parent = nullptr);

private slots:
    void loadMovies();
    void onMovieSelected(int row);
    void onShowTimeChanged(int index);
    void onBookNow();
    void onMyBookings();
    void onLogout();
    void onSearch(const QString& text);
    void onCancelBooking();
    void onViewReceipt();

private:
    void setupUi();
    void applyStyles();
    void setupSidebar();
    void setupMovieList();
    void setupMovieDetail();
    void resetMovieDetail();
    void updateMovieDetail(const Movie& movie);
    void populateMovieList(const QVector<Movie>& movies);
    void loadBookingTable();
    QString selectedShowTime() const;

    User            m_user;
    QListWidget*    m_movieList;
    QLabel*         m_movieTitle;
    QLabel*         m_movieInfo;
    QLabel*         m_movieDesc;
    QLabel*         m_priceLabel;
    QLabel*         m_availLabel;
    QLabel*         m_showCountLabel;
    QComboBox*      m_showTimeCombo;
    QPushButton*    m_bookBtn;
    QStackedWidget* m_stack;
    QTableWidget*   m_bookingTable;
    QLabel*         m_welcomeLabel;
    QPushButton*    m_viewReceiptBtn;
    QPushButton*    m_cancelBookingBtn;

    Movie           m_selectedMovie;
    QVector<Movie>  m_movies;
    QVector<Movie>  m_visibleMovies;
};
