#include "lock.h"

Lock::Lock(QObject *parent)
{

}

bool Lock::createPin() {








    return false;
}

bool Lock::checkPin(int pinAttempt) {
    if (pin == pinAttempt) {
        locked = false;
        return true;
    }
    return false;
}
