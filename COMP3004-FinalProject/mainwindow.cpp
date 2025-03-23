#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupDateTime();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupDateTime()
{
    dateTimeTimer = new QTimer(this);
    connect(dateTimeTimer, &QTimer::timeout, this, &MainWindow::updateDateTime);
    dateTimeTimer->start(1000); // every second
    updateDateTime();
}

void MainWindow::updateDateTime()
{
    QTime currentTime = QTime::currentTime();

    int hour = currentTime.hour();
    int minute = currentTime.minute();

    // Convert to 12-hour format
    int hour12 = hour % 12;
    if (hour12 == 0) hour12 = 12;

    // Format time string manually
    QString timeString = QString("%1:%2")
                             .arg(hour12, 2, 10, QChar('0'))
                             .arg(minute, 2, 10, QChar('0'));

    QString ampmString = hour < 12 ? "AM" : "PM";

    ui->Time->display(timeString);         // QLCDNumber: 07:34
    ui->AmPmLabel->setText(ampmString);    // QLabel: PM
}
