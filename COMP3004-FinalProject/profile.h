#ifndef PROFILE_H
#define PROFILE_H

#include <QString>
#include <QObject>

class Profile: public QObject
{
    Q_OBJECT
public:
    explicit Profile(const QString& name, double basalRate, double carbRatio,
            double correctionFactor, double targetGlucose, QObject *parent = nullptr);
    const QString& getName() const {return name;};
    double getBasalRate() const {return basalRate;};
    double getCarbRatio() const {return carbRatio;};
    double getCorrectionFactor() const {return correctionFactor;};
    double getTargetBG() const {return targetGlucose;};

private:
    QString name;
    double basalRate;
    double carbRatio;
    double correctionFactor;
    double targetGlucose;
};

#endif // PROFILE_H
