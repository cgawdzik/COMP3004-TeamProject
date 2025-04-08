#include "profile.h"

Profile::Profile(QObject *parent)
    : QObject(parent)
{
    name = "newProfile";
    bolusDuration = 0;
    carb = false;
    activeSchedule = nullptr;
}

void Profile::addBasalSchedule(BasalSchedule* schedule) {
    scheduleList.append(schedule);
}

bool Profile::removeBasalSchedule(int index) {
    if (index >= 0 && index < scheduleList.size()) {
        BasalSchedule* schedule = scheduleList.at(index);
        scheduleList.removeAt(index);
        delete schedule; // Free memory
        return true;
    }
    return false;
}

bool Profile::activateBasalSchedule(int index) {
    if (index >= 0 && index < scheduleList.size()) {
        activeSchedule = scheduleList.at(index);
        return true;
    }
   return false;
}

bool Profile::moveBasalSchedule(int sourceIndex, int destinationIndex) {
    int count = scheduleList.size();
    if (sourceIndex < 0 || sourceIndex >= count || destinationIndex < 0 || destinationIndex >= count)
       return false;
    if (sourceIndex == destinationIndex)
       return true;
    scheduleList.move(sourceIndex, destinationIndex);
    return true;
}
