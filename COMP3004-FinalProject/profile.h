#ifndef PROFILE_H
#define PROFILE_H

#include <QString>
#include <QObject>
#include <QVector>
#include "basalschedule.h"

class Profile: public QObject
{
    Q_OBJECT
public:
    explicit Profile(QObject *parent = nullptr);


    // getters
    const QString& getName() const {return name;};
    const QVector<BasalSchedule*>& getSchedule() const {return scheduleList;};
    int getBolusDuration() const {return bolusDuration;};
    bool getCarb() const {return carb;};
    BasalSchedule* getActiveSchedule() const {return activeSchedule;};

    // setters
    void setName(const QString& name) {this->name = name;};
    void setBolusDuration(int time) {bolusDuration = time;};
    void setCarb(bool set) {carb = set;};
    void addBasalSchedule(BasalSchedule* schedule);
    bool removeBasalSchedule(int index);
    bool activateBasalSchedule(int index);
    bool moveBasalSchedule(int sourceIndex, int destinationIndex);


private:
    QString name;
    QVector<BasalSchedule*> scheduleList;
    BasalSchedule* activeSchedule;
    int bolusDuration;
    bool carb;
};

#endif // PROFILE_H
