#pragma once
#include <QDialog>
#include "models.h"

class ReceiptDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReceiptDialog(const Booking& booking, QWidget* parent = nullptr);
private:
    void setupUi();
    void applyStyles();
    void printReceipt();
    Booking m_booking;
};
