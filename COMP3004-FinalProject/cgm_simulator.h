#ifndef CGM_SIMULATOR_H
#define CGM_SIMULATOR_H

#include <QObject>
#include <QTimer>

class CGMSimulator : public QObject {
    Q_OBJECT

public:
    explicit CGMSimulator(QObject *parent = nullptr);
    void start();
    void stop();

signals:
    void newGlucoseReading(double value);

private slots:
    void generateReading();

private:
    QTimer timer;
    double currentValue;
};

#endif // CGM_SIMULATOR_H
