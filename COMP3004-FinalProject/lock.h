#ifndef LOCK_H
#define LOCK_H

#include <QDialog>

class Lock : public QObject {
    Q_OBJECT

public:
    explicit Lock(QObject *parent = nullptr);
    bool isLocked() { return locked; }

public slots:
    bool checkPin(int pin);
    bool createPin();

private:
    bool locked = true;
    int pin = 1234;

};

#endif // LOCK_H
