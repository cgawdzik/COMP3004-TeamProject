#ifndef CONTROL_IQ_MANAGER_H
#define CONTROL_IQ_MANAGER_H

#include <QObject>

class ControlIQManager : public QObject {
    Q_OBJECT

public:
    explicit ControlIQManager(QObject *parent = nullptr);

public slots:
    bool handleCGM(double glucose);
    void setBasal(double rate);
    double getBasal();

signals:
    void suspendInsulin(int flag);
    void resumeInsulin();
    void administerBolus(double glucoseDeviation);

private:
    bool suspended = false;
    double basalRate;
    bool justIncreased = false;
};

#endif // CONTROL_IQ_MANAGER_H
