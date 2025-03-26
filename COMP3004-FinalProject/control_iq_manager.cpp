#include "control_iq_manager.h"

ControlIQManager::ControlIQManager(QObject *parent)
    : QObject(parent) {}

void ControlIQManager::handleCGM(double glucose) {
    if (glucose < 3.9 && !suspended) {
        suspended = true;
        emit suspendInsulin();
    } else if (glucose >= 4.5 && suspended) {
        suspended = false;
        emit resumeInsulin();
    }
}
