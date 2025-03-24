#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupDateTime();
    setupBattery();

    // Bolus Button
    connect(ui->BolusOption, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->BolusScreen);
    });

    // Options Button
    connect(ui->OptionsButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->OptionScreen);
    });

    // Back button on bolus page
    connect(ui->BolusBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->HomeScreen);
    });

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

    ui->Time->display(timeString);
    ui->AmPmLabel->setText(ampmString);
    ui->Date->setText(QDate::currentDate().toString("MMMM d, yyyy"));

}

void MainWindow::setupBattery()
{
    batteryLevel = 100;  // Start full

    ui->Battery->setMinimum(0);
    ui->Battery->setMaximum(100);
    ui->Battery->setValue(batteryLevel);
    ui->BatteryPercentLabel->setText(QString::number(batteryLevel) + "%");

    batteryTimer = new QTimer(this);
    connect(batteryTimer, &QTimer::timeout, this, &MainWindow::updateBattery);
    batteryTimer->start(60000); // every 60 seconds

}

void MainWindow::updateBattery()
{
    if (batteryLevel > 0) {
        batteryLevel -= 1;  // simulate drain
    }

    ui->Battery->setValue(batteryLevel);
    ui->BatteryPercentLabel->setText(QString::number(batteryLevel) + "%");
}
