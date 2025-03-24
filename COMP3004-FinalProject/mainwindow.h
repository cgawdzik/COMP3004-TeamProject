#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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


};
#endif // MAINWINDOW_H
