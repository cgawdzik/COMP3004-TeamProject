#include "cgm_simulator.h"
#include <QRandomGenerator>

CGMSimulator::CGMSimulator(QObject *parent)
    : QObject(parent), currentValue(6.0) {
    connect(&timer, &QTimer::timeout, this, &CGMSimulator::generateReading);
    timer.setInterval(5000); // 5 seconds
}

void CGMSimulator::start() {
    timer.start();
}

void CGMSimulator::stop() {
    timer.stop();
}

void CGMSimulator::generateReading() {
    double delta = QRandomGenerator::global()->generateDouble() * 0.6 - 0.3;
    currentValue = qBound(2.5, currentValue + delta, 15.0);
    emit newGlucoseReading(currentValue);
}
