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

    // Back button on personal profiles page
    connect(ui->PersonalProfileBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->OptionScreen);
    });

    // Tandem Logo, Main Screen
    connect(ui->TandemLogo, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->HomeScreen);
    });

    // Tandem Logo, Main Screen
    connect(ui->StatusButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->StatusScreen);
    });

    // Personal profiles button on options page
    connect(ui->PersonalProfilesButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->PersonalProfileScreen);
    });

    // Create profile button on personal profile page
    connect(ui->CreateProfileButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->ProfileCreatorScreen);
    });

    // Confirm profile button on create profile page
    connect(ui->ConfirmProfileButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->PersonalProfileScreen);
    });

    // Cancel profile button on create profile page
    connect(ui->CancelProfileButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->PersonalProfileScreen);
    });

    // Pump settings button on options page
    connect(ui->PumpSettingsButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->PumpSettingsScreen);
    });

    // Back button on pump settings page
    connect(ui->PumpSettingsBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->OptionScreen);
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

        // Show current glucose and update status if active
        ui->InsulinStatusLabel->setText(QString("Glucose: %1 mmol/L — Insulin Active").arg(glucose, 0, 'f', 1));
    });

    connect(controlIQ, &ControlIQManager::suspendInsulin, this, [=]() {
        ui->ConfirmButton->setEnabled(false);
        ui->InsulinStatusLabel->setText("Insulin Suspended — Glucose too low!");
    });

    connect(controlIQ, &ControlIQManager::resumeInsulin, this, [=]() {
        ui->ConfirmButton->setEnabled(true);
        ui->InsulinStatusLabel->setText(QString("Glucose: %1 mmol/L — Insulin Active").arg(latestGlucose, 0, 'f', 1));
    });

    connect(ui->ConfirmButton, &QPushButton::clicked, this, [=]() {
        double carbs = ui->CarbsSpinBox->value();
        double bg = ui->GlucoseSpinBox->value();
        double suggested = bolusMgr->calculateSuggestedBolus(bg, carbs);
        bolusMgr->deliverBolus(suggested);
        if (suggested < 0) suggested = 0; // if negative 0 for lower bound
        ui->UnitsLCD->display(suggested); // updates the "units" display
    });

    // View Calculation Button
    connect(ui->ViewCalculationButton, &QPushButton::clicked, this, [=]() {
        double carbs = ui->CarbsSpinBox->value();
        double bg = ui->GlucoseSpinBox->value();

        // Hardcoded parameters used in BolusManager
        double targetBG = 5.5;
        double correctionFactor = 2.0;
        double carbRatio = 10.0;

        double correction = (bg - targetBG) / correctionFactor;
        if (correction < 0) correction = 0;

        double meal = carbs / carbRatio;
        double total = correction + meal;

        QString message = QString(
                              "Bolus Calculation Breakdown:\n\n"
                              "Glucose: %1 mmol/L\n"
                              "Target BG: %2 mmol/L\n"
                              "Correction Factor: %3\n"
                              "→ Correction Dose: (%1 - %2) / %3 = %4 units\n\n"
                              "Carbs: %5 g\n"
                              "Carb Ratio: %6 g/unit\n"
                              "→ Meal Dose: %5 / %6 = %7 units\n\n"
                              "Total Suggested Dose: %8 units"
                              ).arg(bg)
                              .arg(targetBG)
                              .arg(correctionFactor)
                              .arg(correction, 0, 'f', 2)
                              .arg(carbs)
                              .arg(carbRatio)
                              .arg(meal, 0, 'f', 2)
                              .arg(total, 0, 'f', 2);

        QMessageBox::information(this, "Calculation Breakdown", message);
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
    batteryLevel = 21;  // Start full

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

    // Update progress bar and label
    ui->Battery->setValue(batteryLevel);
    ui->BatteryPercentLabel->setText(QString::number(batteryLevel) + "%");

    // Visual warning when low
    if (batteryLevel <= 20) {
        ui->BatteryPercentLabel->setStyleSheet("color: red;");
        QMessageBox::warning(this, "Low Battery", "Battery is critically low. Please recharge soon.");
    } else {
        ui->BatteryPercentLabel->setStyleSheet("color: black;");
    }

    // Shutdown at 0%
    if (batteryLevel == 0) {
        ui->ConfirmButton->setEnabled(false);
        ui->InsulinStatusLabel->setText("Pump shut down due to dead battery!");
        QMessageBox::critical(this, "Pump Shutdown", "Battery has fully drained. Pump is shutting down.");
        batteryTimer->stop();
        cgmSim->stop();
    }
}
