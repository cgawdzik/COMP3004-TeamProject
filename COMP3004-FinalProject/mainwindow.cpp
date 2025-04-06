#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupDateTime();
    setupBattery();
    setupProfiles();

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

    // History Data Setup
    history = new HistoryData(this);

    // Status Screen Update at Boot
    updateStatus();

    // History Screen Update at Boot
    updateHistory(0);

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


        // History Lists Updating

        // New Entry, Datetime
        QDateTime datetime = QDateTime::currentDateTime();
        history->addEntryDateTime(datetime.toString("yyyy-MM-dd hh:mm:ss"));

        // New Entry, Basal Rate
        history->addEntryBasal(controlIQ->getBasal());

        // New Entry, Injected Bolus
        history->addEntryBolus(suggested);

        // New Entry, Insulin Remaining
        history->addEntryInsulin(insulinRemaining);

        // Calculation of the Correction Factor
        // Hardcoded parameters used in BolusManager
        double targetBG = 5.5;
        double correctionFactor = 2.0;
        double carbRatio = 10.0;

        // Calculation, 0 if lower
        double correction = (bg - targetBG) / correctionFactor;
        if (correction < 0) correction = 0;

        // New Entry, Correction Factor
        history->addEntryCorrection(correction);

        // New Entry, Carbs
        history->addEntryCarbs(carbs);

        // New Entry, Glucose
        history->addEntryGlucose(bg);

        // Increment the List Size by 1
        //auto lambda = [history_size]() mutable {
        //    history_size++;
        //};
        history->setHistorySize(history->getHistorySize() + 1);

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

//+=======================+ PERSONAL PROFILES SCREEN +=======================+//

    // Back button
    connect(ui->PersonalProfileBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->OptionScreen);
        profileState = NONE;
        currentProfile = nullptr;
    });

    // Create profile button
    connect(ui->CreateProfileButton, &QPushButton::clicked, this, [this]() {
        profileState = CREATE;
        ui->Pages->setCurrentWidget(ui->ProfileOptionScreen);
        ui->ProfileNameLineEdit->clear();

        Profile* newProfile = new Profile();
        currentProfile = newProfile;
        model->setProfile(currentProfile);
        updateBolusTable();
    });

    // Delete profile button
    connect(ui->DeleteProfileButton, &QPushButton::clicked, this, [this]() {
        QListWidgetItem *selectedItem = ui->ProfileListWidget->currentItem();
           if (selectedItem) {
               Profile *profile = selectedItem->data(Qt::UserRole).value<Profile*>();
               if (profile == activeProfile) {
                   QMessageBox::StandardButton reply;
                   reply = QMessageBox::question(this, "Active Profile Deletion",
                                                    "An active profile is being deleted.\nDo you want to continue?",
                                                    QMessageBox::Yes | QMessageBox::No);
                   if (reply == QMessageBox::No) return; // cancel the delete
                   activeProfile = nullptr;
               }
               if (profile) delete profile; // Free memory
               delete ui->ProfileListWidget->takeItem(ui->ProfileListWidget->row(selectedItem));
        }
        currentProfile = nullptr;
    });

    // Edit Button on Profile Selection Page
    connect(ui->EditProfileButton, &QPushButton::clicked, this, [this]()  {
        profileState = EDIT;
        if (!ui->ProfileListWidget->currentItem()) {
            return;
        }
        currentProfile = ui->ProfileListWidget->currentItem()->data(Qt::UserRole).value<Profile*>();
        ui->ProfileNameLineEdit->setText(currentProfile->getName());
        model->setProfile(currentProfile);
        updateBolusTable();
        ui->Pages->setCurrentWidget(ui->ProfileOptionScreen);
    });

    // Activate Button
    connect(ui->ActivateProfileButton, &QPushButton::clicked, this, [this]() {
        QListWidgetItem *selectedItem = ui->ProfileListWidget->currentItem();
        if (selectedItem) {
            if (activeProfile != nullptr) {
                // Change label of old active profile
                QListWidgetItem *previousItem = findItemForProfile(ui->ProfileListWidget, activeProfile);
                previousItem->setText(activeProfile->getName() + ": OFF");
            }

            // Change label of new active profile
            Profile *profile = selectedItem->data(Qt::UserRole).value<Profile*>();
            selectedItem->setText(profile->getName() + ": ON");
            activeProfile = profile;

            int row = ui->ProfileListWidget->row(selectedItem);
            QListWidgetItem* item = ui->ProfileListWidget->takeItem(row);
            ui->ProfileListWidget->insertItem(0, item);
            ui->ProfileListWidget->setCurrentItem(item);
        }
    });

//+=======================+ PROFILE OPTIONS SCREEN +=======================+//

    // Back button
    connect(ui->ProfileOptionsBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->PersonalProfileScreen);
    });

    // Edit Button for Timed Settings
    connect(ui->TimedSettingsEditButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->BasalScheduleScreen);
    });

    // Edit Button for Bolus Settings
    connect(ui->BolusSettingEditButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->ProfileBolusScreen);
    });

    // Confirm profile button
    connect(ui->ProfileConfirmButton, &QPushButton::clicked, this, [this]()  {
        if (profileState == CREATE) {
            QString profileName = ui->ProfileNameLineEdit->text();

            // Check for duplicates
            QList<QListWidgetItem*> matches;
            for (int i = 0; i < ui->ProfileListWidget->count(); ++i) {
                QListWidgetItem* item = ui->ProfileListWidget->item(i);
                if (item->text().section(':', 0, 0).compare(profileName, Qt::CaseSensitive) == 0) {
                    matches.append(item);
                }
            }

            // Matches found
            if (!matches.isEmpty()) {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "Duplicate Profile Name",
                                                 "A profile with this name already exists.\nDo you want to replace it?",
                                                 QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::No) return; // cancel the save or creatio
                // Delete old profile with same name
                QListWidgetItem *duplicateItem = matches.first();
                Profile *profile = duplicateItem->data(Qt::UserRole).value<Profile*>();
                if (profile) delete profile; // Free memory
                delete ui->ProfileListWidget->takeItem(ui->ProfileListWidget->row(duplicateItem));
            }

            currentProfile->setName(profileName);
            QListWidgetItem *item = new QListWidgetItem(currentProfile->getName() + ": OFF");  // Display name
            item->setData(Qt::UserRole, QVariant::fromValue(currentProfile));  // Store Profile object
            ui->ProfileListWidget->addItem(item);
        }
        else if (profileState == EDIT){
            currentProfile->setName(ui->ProfileNameLineEdit->text());
            QListWidgetItem *item = findItemForProfile(ui->ProfileListWidget, currentProfile);
            QString suffix = item->text().section(':', 1, 1);
            item->setText(currentProfile->getName() + ":" + suffix);
        }
        profileState = NONE;
        ui->Pages->setCurrentWidget(ui->PersonalProfileScreen);
    });
//+=======================+ PROFILE TIMED SETTINGS SCREEN +=======================+//

    // Add Button for Basal Schedule in Timed Settings Page
    connect(ui->CreateScheduleButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->ProfileBasalScreen);
        timedState = CREATE;
        ui->TimeLineEdit->setText("0:00");
        ui->BasalRateLineEdit->clear();
        ui->CarbRatioLineEdit->clear();
        ui->CorrFactorLineEdit->clear();
        ui->TargetBGLineEdit->clear();
    });

    // Delete Button for Basal Schedule in Timed Settings Page
    connect(ui->DeleteScheduleButton, &QPushButton::clicked, this, [this]() {
        QModelIndex selectedIndex = ui->BasalScheduleTableView->selectionModel()->currentIndex();
        if (selectedIndex.isValid()) {
            int row = selectedIndex.row();
            const QVector<BasalSchedule*>& schedule = currentProfile->getSchedule();
            if (row < schedule.size()) {
                bool check = model->removeBasalRow(row);
                if (!check) {
                       qDebug() << "Failed to remove row";
                }
            }
        } else {
            qDebug() << "Invalid selected index";
        }
    });

    // Back Button for Basal Schedule in Timed Settings Page
    connect(ui->BasalScheduleBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->ProfileOptionScreen);
    });

    // Edit Button for Basal Schedule in Basal Setting Page
    connect(ui->EditScheduleButton, &QPushButton::clicked, this, [this]() {
       timedState = EDIT;
       ui->Pages->setCurrentWidget(ui->ProfileBasalScreen);
       QModelIndex selectedIndex = ui->BasalScheduleTableView->selectionModel()->currentIndex();
       if (!selectedIndex.isValid()) {
           qDebug() << "Invalid Basal Schedule selection";
           return;
       }
       int row = selectedIndex.row();
       BasalSchedule* schedule = currentProfile->getSchedule().at(row);
       ui->TimeLineEdit->setText(schedule->getTime().toString("hh:mm"));
       ui->BasalRateLineEdit->setText(QString::number(schedule->getBasalRate()));
       ui->CarbRatioLineEdit->setText(QString::number(schedule->getCarbRatio()));
       ui->CorrFactorLineEdit->setText(QString::number(schedule->getCorrFactor()));
       ui->TargetBGLineEdit->setText(QString::number(schedule->getTargetBG()));
       ui->Pages->setCurrentWidget(ui->ProfileBasalScreen);
    });


//+=======================+ PROFILE BASAL SETTINGS SCREEN +=======================+//

    // Back Button for Basal Schedule in Basal Setting PPage
    connect(ui->BasalSettingBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->BasalScheduleScreen);
    });

    // Confirm Button
    connect(ui->BasalSettingConfirmButton, &QPushButton::clicked, this, [this]() {
        BasalSchedule* newSchedule = new BasalSchedule(
                    QTime::fromString(ui->TimeLineEdit->text(), "h:mm"),
                    ui->BasalRateLineEdit->text().toDouble(),
                    ui->CarbRatioLineEdit->text().toDouble(),
                    ui->CorrFactorLineEdit->text().toDouble(),
                    ui->TargetBGLineEdit->text().toDouble());
        if (timedState == CREATE) {
            model->addBasalRow(newSchedule);
            ui->Pages->setCurrentWidget(ui->BasalScheduleScreen);
        } else if (timedState == EDIT) {
            QModelIndex selectedIndex = ui->BasalScheduleTableView->selectionModel()->currentIndex();
            if (!selectedIndex.isValid()) {
                qDebug() << "Invalid Basal Schedule selection";
                delete newSchedule;
                return;
            }
            int row = selectedIndex.row();
            model->removeBasalRow(row);
            model->addBasalRow(newSchedule);
        } else {
            delete newSchedule;
        }
        ui->Pages->setCurrentWidget(ui->BasalScheduleScreen);
    });

//+=======================+ PROFILE BOLUS SETTINGS SCREEN +=======================+//

    // Back Button
    connect(ui->BolusSettingBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->ProfileOptionScreen);
    });


    // Confirm Button for Bolus Setting in Bolus Setting Page
    connect(ui->BolusSettingConfirmButton, &QPushButton::clicked, this, [this]() {
        currentProfile->setBolusDuration(ui->BolusSettingDurationSpinBox->value());
        currentProfile->setCarb(ui->BolusSettingCarbRadioButton->isChecked());
        updateBolusTable();
        ui->Pages->setCurrentWidget(ui->ProfileOptionScreen);
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

    //+=======================+ History SCREEN +=======================+//
    connect(ui->HistoryButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->HistoryScreen);
        updateHistory(0);
    });

    // Back button
    connect(ui->HistoryBackButton, &QPushButton::clicked, this, [this]() {
        ui->Pages->setCurrentWidget(ui->OptionScreen);
    });

    // Previous button
    connect(ui->previousButton, &QPushButton::clicked, this, [this]() {
        if(history->getHistoryIndex() > 0){
            updateHistory(1);
        }
    });

    // Next button
    connect(ui->nextButton, &QPushButton::clicked, this, [this]() {
        if(history->getHistoryIndex() < history->getHistorySize() - 1){
            updateHistory(2);
        }
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

    // Set Datetime
    QDateTime datetime = QDateTime::currentDateTime();
    ui->weekday->setText(datetime.toString("yyyy-MM-dd hh:mm:ss"));

    // Set Last Bolus Rate
    ui->lastbolus->setText(QString::number(history->getLastBolus()));

    // Set Basal Variation
    if(history->getBasalVariation() >= 0){
        ui->basalrate->setText("+" + QString::number(history->getBasalVariation()));
    }
    else{
        ui->basalrate->setText(QString::number(history->getBasalVariation()));
    }

    // Set Carbs Variation
    if(history->getCarbsVariation() >= 0){
        ui->carbo->setText("+" + QString::number(history->getCarbsVariation()));
    }
    else{
        ui->carbo->setText(QString::number(history->getCarbsVariation()));
    }

    // Set Glucose Variation
    if(history->getGlucoseVariation() >= 0){
        ui->glucose->setText("+" + QString::number(history->getGlucoseVariation()));
    }
    else{
        ui->glucose->setText(QString::number(history->getGlucoseVariation()));
    }
}

void MainWindow::updateHistory(int option)
{

    if(history->getHistorySize() == 0){
        ui->date_time->setText("NO DATA");
        ui->currentIndex->setText(QString::number(0));
        ui->totalSize->setText(QString::number(0));
        ui->basal_data->setText("NO DATA");
        ui->bolus_data->setText("NO DATA");
        ui->insulin_data->setText("NO DATA");
        ui->correction_data->setText("NO DATA");

        // Disable the Buttons
        ui->previousButton->setDisabled(true);
        ui->nextButton->setDisabled(true);
    }
    else{
        if(option == 1){
            history->setHistoryIndex(history->getHistoryIndex() - 1);
        }

        else if(option == 2){
            history->setHistoryIndex(history->getHistoryIndex() + 1);
        }

        // Set 5 Parameters on the GUI with the Newest Entry
        ui->currentIndex->setText(QString::number(history->getHistoryIndex() + 1));
        ui->totalSize->setText(QString::number(history->getHistorySize()));
        ui->date_time->setText(history->getDateTime(history->getHistoryIndex()));
        ui->basal_data->setText(QString::number(history->getBasal(history->getHistoryIndex())));
        ui->bolus_data->setText(QString::number(history->getBolus(history->getHistoryIndex())));
        ui->insulin_data->setText(QString::number(history->getInsulin(history->getHistoryIndex())));
        ui->correction_data->setText(QString::number(history->getCorrection(history->getHistoryIndex())));


        // Enable the Buttons when Size is Big Enough
        if(history->getHistorySize() > 1){
            ui->previousButton->setDisabled(false);
            ui->nextButton->setDisabled(false);
        }
        else{
            ui->previousButton->setDisabled(true);
            ui->nextButton->setDisabled(true);
        }

        // Button by Index
        if(history->getHistoryIndex() == 0){
            ui->previousButton->setDisabled(true);
            ui->nextButton->setDisabled(false);
        }
        else if(history->getHistoryIndex() == history->getHistorySize() - 1){
            ui->previousButton->setDisabled(false);
            ui->nextButton->setDisabled(true);
        }
        else{
            ui->previousButton->setDisabled(false);
            ui->nextButton->setDisabled(false);
        }
    }

}

void MainWindow::setupProfiles() {
    currentProfile = nullptr;
    activeProfile = nullptr;
    profileState = NONE;
    timedState = NONE;

    // setup profile
    ui->ProfileListWidget->setStyleSheet(R"(
        QListWidget::item {
            border: 2px solid black; /* Outline */
            padding: 10px; /* More space */
            margin: 5px; /* Space between items */
        }

        QListWidget::item:selected {
            border: 2px solid blue;
            color: black;
        }
    )");
    model = new BasalScheduleModel(currentProfile, this);
    ui->ProfileTimedSettingsTableView->setModel(model);
    ui->BasalScheduleTableView->setModel(model);
}

QListWidgetItem* MainWindow::findItemForProfile(QListWidget* listWidget, Profile* targetProfile) {
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        Profile* storedProfile = item->data(Qt::UserRole).value<Profile*>();
        if (storedProfile == targetProfile) {
            return item;  // Found the matching item
        }
    }
    return nullptr;  // Not found
}

void MainWindow::updateBolusTable(){
    QTableWidget* table = ui->ProfileBolusSettingTableWidget;
    table->clear();
    table->setRowCount(1);
    table->setColumnCount(2);

    // Optionally, set header labels
    table->setHorizontalHeaderLabels(QStringList() << "Duration (Hrs)" << "Carbohydrates");

    QTableWidgetItem* durationItem = new QTableWidgetItem(QString::number(currentProfile->getBolusDuration()));
    QTableWidgetItem* carbItem = new QTableWidgetItem(QString(currentProfile->getCarb()? "ON" : "OFF"));
    ui->ProfileBolusSettingTableWidget->setItem(0, 0, durationItem);
    ui->ProfileBolusSettingTableWidget->setItem(0, 1, carbItem);
}
