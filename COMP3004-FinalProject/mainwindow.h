#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QListWidget>

#include "cgm_simulator.h"
#include "control_iq_manager.h"
#include "bolus_manager.h"
#include "history_data.h"
#include "profile.h"
#include "glucose_graph_widget.h"
#include "basalschedulemodel.h"
#include "firstrowhighlightdelegate.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum ProfileState {CREATE, EDIT, NONE};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // For time and date
    void setupDateTime();
    void updateDateTime();
    QTimer *dateTimeTimer;

    // For battery
    int batteryLevel = 100;
    QTimer *batteryTimer;
    void setupBattery();
    void updateBattery();
    void updateStatus();

    CGMSimulator* cgmSim;
    ControlIQManager* controlIQ;
    double latestGlucose;
    BolusManager* bolusMgr;

    // Graph
    GlucoseGraphWidget *glucoseGraph;

    // Insulin on board
    double currentIOB = 0.0;
    QTimer *iobTimer;
    double insulinRemaining = 300.0;

    // For Profiles
    Profile* currentProfile;
    Profile* activeProfile;
    ProfileState profileState;
    ProfileState timedState;
    BasalScheduleModel* model;

    void setupProfiles();
    QListWidgetItem* findItemForProfile(QListWidget* listWidget, Profile* targetProfile);
    void updateBolusTable();

    // For History
    HistoryData* history;
    void updateHistory(int index);

};
#endif // MAINWINDOW_H
