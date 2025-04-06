#include "profile.h"

Profile::Profile(QObject *parent)
    : QObject(parent)
{
    name = "newProfile";
    bolusDuration = 0;
    carb = false;
}

void Profile::addBasalSchedule(BasalSchedule* schedule) {
    scheduleList.append(schedule);
}

void Profile::removeBasalSchedule(int index) {
    if (index >= 0 && index < scheduleList.size()) {
        BasalSchedule* schedule = scheduleList.at(index);
        scheduleList.removeAt(index);
        delete schedule; // Free memory
    }
}
