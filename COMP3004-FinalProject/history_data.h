#ifndef HISTORY_DATA_H
#define HISTORY_DATA_H
#include <QObject>
#include <QList>
#include <QString>

class HistoryData : public QObject
{
Q_OBJECT
public:
    explicit HistoryData(QObject *parent = nullptr);
    int getHistorySize();
    int getHistoryIndex();
    void setHistorySize(int size);
    void setHistoryIndex(int index);

    void addEntryDateTime(QString data);
    void addEntryBasal(double data);
    void addEntryBolus(double data);
    void addEntryInsulin(double data);
    void addEntryCorrection(double data);
    void addEntryCarbs(double data);
    void addEntryGlucose(double data);

    QString getDateTime(int index);
    double getBasal(int index);
    double getBolus(int index);
    double getInsulin(int index);
    double getCorrection(int index);
    double getCarbs(int index);
    double getGlucose(int index);

    QString getLastDateTime();
    double getLastBolus();
    bool isBasalEmpty();

    double getBasalVariation();
    double getCarbsVariation();
    double getGlucoseVariation();


private:
    QList<QString> list_datetime;
    QList<double> list_basal;
    QList<double> list_bolus;
    QList<double> list_insulin;
    QList<double> list_correction;
    QList<double> list_carbs;
    QList<double> list_glucose;
    int history_size = 0;
    int historyIndex = 0;
};


#endif // HISTORY_DATA_H
