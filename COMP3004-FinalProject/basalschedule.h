#ifndef BASALSCHEDULE_H
#define BASALSCHEDULE_H

#include <QTime>
#include <QDebug>
#include <QString>

class BasalSchedule
{
public:
    BasalSchedule(QTime time, double basal, double carb, double corr, double target):
        time(time), basalRate(basal), carbRatio(carb), correctionFactor(corr), targetGlucose(target){};
    const QTime& getTime() const {return time;};
    double getBasalRate() const {return basalRate;};
    double getCarbRatio() const {return carbRatio;};
    double getCorrFactor() const {return correctionFactor;};
    double getTargetBG() const {return targetGlucose;};


private:
    QTime time;
    double basalRate;
    double carbRatio;
    double correctionFactor;
    double targetGlucose;
};

#endif // BASALSCHEDULE_H
