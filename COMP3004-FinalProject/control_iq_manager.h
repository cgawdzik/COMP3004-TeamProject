#ifndef CONTROL_IQ_MANAGER_H
#define CONTROL_IQ_MANAGER_H

#include <QObject>

class ControlIQManager : public QObject {
    Q_OBJECT

public:
    explicit ControlIQManager(QObject *parent = nullptr);

public slots:
    void handleCGM(double glucose);

signals:
    void suspendInsulin();
    void resumeInsulin();

private:
    bool suspended = false;
};

#endif // CONTROL_IQ_MANAGER_H
