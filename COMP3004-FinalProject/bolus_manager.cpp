#include "bolus_manager.h"
#include <QDebug>

BolusManager::BolusManager()
    : lastDeliveredBolus(0.0) {}

double BolusManager::calculateSuggestedBolus(double bg, double carbs) {
    double correction = (bg - targetBG) / correctionFactor;
    double meal = carbs / carbRatio;
    return correction + meal;
}

void BolusManager::deliverBolus(double units) {
    lastDeliveredBolus = units;
    qDebug() << "Delivered bolus:" << units << "units";
}
