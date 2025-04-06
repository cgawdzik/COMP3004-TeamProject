#ifndef HISTORY_DATA_H
#define HISTORY_DATA_H
#include <QObject>
#include <QList>

class HistoryData : public QObject
{
Q_OBJECT
public:
    explicit HistoryData(QObject *parent = nullptr);
    int getHistorySize();
    int getHistoryIndex();
    void setHistorySize(int size);
    void setHistoryIndex(int index);

    void addEntryBasal(double data);
    void addEntryBolus(double data);
    void addEntryInsulin(double data);
    void addEntryCorrection(double data);

    double getBasal(int index);
    double getBolus(int index);
    double getInsulin(int index);
    double getCorrection(int index);


private:
    QList<double> list_basal;
    QList<double> list_bolus;
    QList<double> list_insulin;
    QList<double> list_correction;
    int history_size = 0;
    int historyIndex = 0;
};


#endif // HISTORY_DATA_H
