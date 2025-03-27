#include "profile.h"

Profile::Profile(const QString& name, double basalRate, double carbRatio, double correctionFactor, double targetGlucose, QObject *parent)
    : name(name), basalRate(basalRate), carbRatio(carbRatio), correctionFactor(correctionFactor), targetGlucose(targetGlucose), QObject(parent)
{

}
