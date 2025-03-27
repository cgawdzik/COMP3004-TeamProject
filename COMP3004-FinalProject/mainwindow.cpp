#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupDateTime();
    setupBattery();
    ui->ProfileListWidget->setStyleSheet(R"(
        QListWidget::item {
            border: 2px solid black; /* Outline */
            padding: 10px; /* More space */
            margin: 5px; /* Space between items */
        }

        QListWidget::item:selected {
            border: 2px solid blue;
        }
    )");

    iobTimer = new QTimer(this);
    connect(iobTimer, &QTimer::timeout, this, [=]() {
        if (currentIOB > 0.0) {
            currentIOB = std::max(0.0, currentIOB - 0.05); // simple decay
            ui->IOBLabel->setText(QString("IOB: %1 units").arg(currentIOB, 0, 'f', 2));
        }
    });
    iobTimer->start(60000); // every 60 sec


    glucoseGraph = new GlucoseGraphWidget(this);
    auto *layout = new QVBoxLayout(ui->GraphLayout);
    layout->addWidget(glucoseGraph);


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
        ui->ProfileNameTextEdit->clear();
        ui->BasalRateTextEdit->clear();
        ui->CarbRatioTextEdit->clear();
        ui->CorrFactorTextEdit->clear();
        ui->TargetBGTextEdit->clear();
    });

    // Delete profile button on personal profile page
    connect(ui->DeleteProfileButton, &QPushButton::clicked, this, [this]() {
        QListWidgetItem *selectedItem = ui->ProfileListWidget->currentItem();
           if (selectedItem) {
               Profile *profile = selectedItem->data(Qt::UserRole).value<Profile*>();

               if (profile) delete profile; // Free memory

               delete ui->ProfileListWidget->takeItem(ui->ProfileListWidget->row(selectedItem));
           }
    });

    // Confirm profile button on create profile page
//    connect(ui->ConfirmProfileButton, &QPushButton::clicked, this, [this]() {
//        ui->Pages->setCurrentWidget(ui->PersonalProfileScreen);
//    });
    connect(ui->ConfirmProfileButton, &QPushButton::clicked, this, [this]()  {
        QString profileName = ui->ProfileNameTextEdit->toPlainText();
        Profile* profile = new Profile(profileName,
                        ui->BasalRateTextEdit->toPlainText().toDouble(),
                      ui->CarbRatioTextEdit->toPlainText().toDouble(),
                        ui->CorrFactorTextEdit->toPlainText().toDouble(),
                        ui->TargetBGTextEdit->toPlainText().toDouble());
        QList<QListWidgetItem *> matches = ui->ProfileListWidget->findItems(profileName, Qt::MatchExactly);
        if (!matches.isEmpty()) {
//            QMessageBox::information(this, "Warning", "Another Profile with the same name exists.\nPlease edit or delete existing profile.");
//            return;
            // Replace old profile with same name
            QListWidgetItem *selectedItem = ui->ProfileListWidget->currentItem();
            Profile *profile = selectedItem->data(Qt::UserRole).value<Profile*>();
            if (profile) delete profile; // Free memory
            delete ui->ProfileListWidget->takeItem(ui->ProfileListWidget->row(selectedItem));
        }

        QListWidgetItem *item = new QListWidgetItem(profile->getName());  // Display name
        item->setData(Qt::UserRole, QVariant::fromValue(profile));  // Store Profile object
        ui->ProfileListWidget->addItem(item);
        ui->Pages->setCurrentWidget(ui->PersonalProfileScreen);
    });

    // Edit Button on Profile Selection Page
    connect(ui->EditProfileButton, &QPushButton::clicked, this, [this]()  {
        ui->Pages->setCurrentWidget(ui->ProfileCreatorScreen);
        Profile* profile = ui->ProfileListWidget->currentItem()->data(Qt::UserRole).value<Profile*>();
        ui->ProfileNameTextEdit->setText(profile->getName());
        ui->BasalRateTextEdit->setText(QString::number(profile->getBasalRate()));
        ui->CarbRatioTextEdit->setText(QString::number(profile->getCarbRatio()));
        ui->CorrFactorTextEdit->setText(QString::number(profile->getCorrectionFactor()));
        ui->TargetBGTextEdit->setText(QString::number(profile->getTargetBG()));
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

    // Recharge button on pump settings widget
    connect(ui->RechargeButton, &QPushButton::clicked, this, [=]() {
        batteryLevel = 100;
        ui->Battery->setValue(batteryLevel);
        ui->BatteryPercentLabel->setText("100%");
        ui->BatteryPercentLabel->setStyleSheet("color: black;");

        QMessageBox::information(this, "Recharge Complete", "The pump battery has been fully recharged.");

        // Resume if shut down
        if (!batteryTimer->isActive()) {
            batteryTimer->start();
        }

        if (!cgmSim->isRunning()) {
            cgmSim->start();
        }

        ui->ConfirmButton->setEnabled(true);
        ui->InsulinStatusLabel->setText("Pump recharged. Insulin Active.");
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
        ui->InsulinStatusLabel_2->setText(QString("Glucose: %1 mmol/L — Insulin Active").arg(glucose, 0, 'f', 1));
        // Add to graph
        glucoseGraph->addReading(glucose);
    });

    connect(controlIQ, &ControlIQManager::suspendInsulin, this, [=]() {
        ui->ConfirmButton->setEnabled(false);
        ui->InsulinStatusLabel->setText("Insulin Suspended — Glucose too low!");
        ui->InsulinStatusLabel_2->setText("Insulin Suspended — Glucose too low!");
    });

    connect(controlIQ, &ControlIQManager::resumeInsulin, this, [=]() {
        ui->ConfirmButton->setEnabled(true);
        ui->InsulinStatusLabel->setText(QString("Glucose: %1 mmol/L — Insulin Active").arg(latestGlucose, 0, 'f', 1));
        ui->InsulinStatusLabel_2->setText(QString("Glucose: %1 mmol/L — Insulin Active").arg(latestGlucose, 0, 'f', 1));
    });

    connect(ui->ConfirmButton, &QPushButton::clicked, this, [=]() {
        double carbs = ui->CarbsSpinBox->value();
        double bg = ui->GlucoseSpinBox->value();
        double suggested = bolusMgr->calculateSuggestedBolus(bg, carbs);
        bolusMgr->deliverBolus(suggested);

        // Updating Insulin on board
        currentIOB = suggested;
        ui->IOBLabel->setText(QString("Insulin On Board %1 units").arg(currentIOB, 0, 'f', 2));

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
