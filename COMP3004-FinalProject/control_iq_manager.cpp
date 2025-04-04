#include "control_iq_manager.h"

ControlIQManager::ControlIQManager(QObject *parent)
    : QObject(parent), basalRate(0) {}

bool ControlIQManager::handleCGM(double glucose) {
    // Glucose levels below 3.9 are detected
    if (glucose < 3.9 && !suspended) {
        suspended = true;
        emit suspendInsulin(0);
    } else if (glucose >= 4.5 && !suspended) { // Increase insulin if target glucose greatly exceeded
        //replace 4.5 with target glucose
        //increase insulin rate
    }
    return suspended;
}

void ControlIQManager::setBasal(double rate) {
   //Basal delivery suspended
   if (rate <= 0) {
       suspended = true;
       emit suspendInsulin(1);
   } else { //Start insulin with rate within proper bounds
       suspended = false;
       basalRate = qBound(0.5, rate, 2.0);
       emit resumeInsulin();
   }
}

double ControlIQManager::getBasal() {
   return basalRate;
}
