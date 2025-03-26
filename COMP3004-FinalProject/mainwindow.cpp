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

    // Back button on options page
    connect(ui->OptionBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->HomeScreen);
    });

    // Tandem Logo, Main Screen
    connect(ui->TandemLogo, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->HomeScreen);
    });

    // Tandem Logo, Main Screen
    connect(ui->StatusButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->StatusScreen);
    });


    // bolous logic follows

    // CGM + ControlIQ Setup
    cgmSim = new CGMSimulator(this);
    controlIQ = new ControlIQManager(this);
    latestGlucose = 6.0;
    bolusMgr = new BolusManager();

    connect(cgmSim, &CGMSimulator::newGlucoseReading, this, [=](double glucose) {
        latestGlucose = glucose;
        controlIQ->handleCGM(glucose);
    });

    connect(controlIQ, &ControlIQManager::suspendInsulin, this, [=]() {
        ui->ConfirmButton->setEnabled(false);
    });

    connect(controlIQ, &ControlIQManager::resumeInsulin, this, [=]() {
        ui->ConfirmButton->setEnabled(true);
    });



    connect(ui->ConfirmButton, &QPushButton::clicked, this, [=]() {
        double carbs = ui->CarbsSpinBox->value();
        double bg = ui->GlucoseSpinBox->value();
        double suggested = bolusMgr->calculateSuggestedBolus(bg, carbs);
        bolusMgr->deliverBolus(suggested);

        ui->UnitsLCD->display(suggested); // ← updates the "units" display
    });


    cgmSim->start();


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
