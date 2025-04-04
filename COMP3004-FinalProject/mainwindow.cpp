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

    // CGM + ControlIQ Setup
    cgmSim = new CGMSimulator(this);
    controlIQ = new ControlIQManager(this);
    latestGlucose = 6.0;
    bolusMgr = new BolusManager();

//+=======================+ HOME SCREEN +=======================+//

    // Tandem Logo, Main Screen
    connect(ui->TandemLogo, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->HomeScreen);
    });
    //ui->TandemLogo->setIcon(QIcon("images/tandem.png"));

    // Status Button, Main Screen
    connect(ui->StatusButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->StatusScreen);
        updateStatus();
    });

    // Bolus Button
    connect(ui->BolusOption, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->BolusScreen);
    });

    // Options Button
    connect(ui->OptionsButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->OptionScreen);
    });

//+=======================+ STATUS SCREEN +=======================+//
    connect(ui->StatusBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->HomeScreen);
    });

//+=======================+ BOLUS SCREEN +=======================+//

    // Back button
    connect(ui->BolusBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->HomeScreen);
    });

    // Confirm button
    connect(ui->ConfirmButton, &QPushButton::clicked, this, [=]() {
        double carbs = ui->CarbsSpinBox->value();
        double bg = ui->GlucoseSpinBox->value();
        double suggested = bolusMgr->calculateSuggestedBolus(bg, carbs);

        if (suggested < 0) suggested = 0; // if negative 0 for lower bound
        bolusMgr->deliverBolus(suggested);

        // Updating Insulin on board
        currentIOB = suggested;
        ui->IOBLabel->setText(QString("Insulin On Board %1 units").arg(currentIOB, 0, 'f', 2));

        // === Update bolus level progress bar ===
        constexpr double visualMax = 15.0;
        int bolusPercent = static_cast<int>((suggested / visualMax) * 100.0);
        if (bolusPercent > 100) bolusPercent = 100;
        ui->BolusLevel->setValue(bolusPercent);


        ui->UnitsLCD->display(suggested); // updates the "units" display

        // === Update insulin remaining ===
        insulinRemaining = std::max(0.0, insulinRemaining - suggested);
        ui->InsulinRemainingBar->setValue(insulinRemaining);

        if (insulinRemaining < 30.0) {
            QMessageBox::warning(this, "Low Insulin", "Insulin cartridge is below 10%. Please refill soon.");
        }

        if (insulinRemaining == 0.0) {
            ui->ConfirmButton->setEnabled(false);
            ui->InsulinStatusLabel->setText("Pump stopped — insulin depleted.");
            QMessageBox::critical(this, "Insulin Depleted", "Insulin has run out. Pump has been stopped.");
        }
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


//+=======================+ OPTIONS SCREEN +=======================+//

    // Back button
    connect(ui->OptionBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->HomeScreen);
    });

    // Personal profiles button
    connect(ui->PersonalProfilesButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->PersonalProfileScreen);
    });

    // Pump settings button
    connect(ui->PumpSettingsButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->PumpSettingsScreen);
    });

//+=======================+ PROFILES SCREEN +=======================+//

    // Back button
    connect(ui->PersonalProfileBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->OptionScreen);
    });

    // Create profile button
    connect(ui->CreateProfileButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->ProfileCreatorScreen);
        ui->ProfileNameTextEdit->clear();
        ui->BasalRateTextEdit->clear();
        ui->CarbRatioTextEdit->clear();
        ui->CorrFactorTextEdit->clear();
        ui->TargetBGTextEdit->clear();
    });

    // Delete profile button
    connect(ui->DeleteProfileButton, &QPushButton::clicked, this, [this]() {
        QListWidgetItem *selectedItem = ui->ProfileListWidget->currentItem();
           if (selectedItem) {
               Profile *profile = selectedItem->data(Qt::UserRole).value<Profile*>();

               if (profile) delete profile; // Free memory

               delete ui->ProfileListWidget->takeItem(ui->ProfileListWidget->row(selectedItem));
           }
    });

    // Confirm profile button
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

//+=======================+ PUMP SETTINGS SCREEN +=======================+//

    // Back button
    connect(ui->PumpSettingsBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->OptionScreen);
    });

    // Stop delivery button
    connect(ui->StopBasalButton, &QPushButton::clicked, this, [this]() {
        controlIQ->setBasal(0);

        // Show popup to user
        QMessageBox::information(this, "Basal Delivery Stopped",
                                 "Insulin delivery has been stopped. Basal rate is now 0 u/hr.");

        // Disable confirm button for safety
        ui->ConfirmButton->setEnabled(false);
    });


    // Set delivery rate button
    connect(ui->ConfigureBasalButton, &QPushButton::clicked, this, [this]() {
        controlIQ->setBasal(QInputDialog::getDouble(this, "Basal Rate", "u/hr:", 1.0));
    });

    // Set delivery rate to basal rate on active profile
    connect(ui->ProfileBasalButton, &QPushButton::clicked, this, [this]() {
        //Set to active profile basal rate  (not 0)
        controlIQ->setBasal(0);
    });

    // Recharge button
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

    // Change Cartridge button logic
    connect(ui->ChangeCartridgeButton, &QPushButton::clicked, this, [=]() {
        insulinRemaining = 300.0;
        ui->InsulinRemainingBar->setValue(insulinRemaining);

        // Re-enable the confirm button (if it was disabled by empty cartridge)
        ui->ConfirmButton->setEnabled(true);
        ui->InsulinStatusLabel->setText("Pump Ready");

        QMessageBox::information(this, "Cartridge Replaced", "New 300-unit insulin cartridge inserted successfully.");
    });


//+=======================+ INSULIN DELIVERY LOGIC +=======================+//

    connect(cgmSim, &CGMSimulator::newGlucoseReading, this, [=](double glucose) {

        latestGlucose = glucose;
        ui->GlucoseStatusLabel->setText(QString("Glucose: %1 mmol/L").arg(latestGlucose, 0, 'f', 1));

        // Hyperglycemia Warning
        if (glucose >= 13.0) {
            QMessageBox::warning(this, "High Glucose", "Hyperglycemia detected! Glucose level is very high.");
        }

        //If insulin not suspended
        if (controlIQ->handleCGM(glucose)) {
            latestGlucose -= controlIQ->getBasal() * 5 / 3600;
            //Decrease glucose by basal delivery for 5 seconds of the u/hr
        }

        // Add to graph
        glucoseGraph->addReading(glucose);
    });

    connect(controlIQ, &ControlIQManager::suspendInsulin, this, [=](int flag) {
        ui->ConfirmButton->setEnabled(false);
        if (flag == 0) {
            ui->InsulinStatusLabel->setText("Insulin Suspended — Glucose too low!");
            ui->InsulinStatusLabel_2->setText("Insulin Suspended — Glucose too low!");
        } else if (flag == 1) {
            ui->InsulinStatusLabel->setText("Insulin Suspended by User");
            ui->InsulinStatusLabel_2->setText("Insulin Suspended by User");
        }

    });

    connect(controlIQ, &ControlIQManager::resumeInsulin, this, [=]() {
        ui->ConfirmButton->setEnabled(true);
        ui->InsulinStatusLabel->setText(QString("Insulin Active: %2 u/hr").arg(controlIQ->getBasal()));
        ui->InsulinStatusLabel_2->setText(QString("Insulin Active: %2 u/hr").arg(controlIQ->getBasal()));
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

void MainWindow::updateStatus()
{
    /*
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
    */


    // Set Weekday
    QDate date = QDate::currentDate();
    ui->weekday->setText(date.toString("dddd"));



    // Set Last Basal Rate
    ui->basalrate->setText(QString::number(controlIQ->getBasal()));


    // Set Last Bolus Rate
    double carbs = ui->CarbsSpinBox->value();
    double bg = ui->GlucoseSpinBox->value();
    double suggested = bolusMgr->calculateSuggestedBolus(bg, carbs);
    ui->lastbolus->setText(QString::number(suggested));


    // Set Carbs
    ui->carbo->setText(QString::number(carbs));


}
