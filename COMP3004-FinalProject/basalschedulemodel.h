#ifndef BASALSCHEDULEMODEL_H
#define BASALSCHEDULEMODEL_H

#include <QAbstractTableModel>
#include "profile.h"
#include "basalschedule.h"

class BasalScheduleModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    // The model holds a pointer to the Profile object whose schedule it will display.
    explicit BasalScheduleModel(Profile* profile, QObject* parent = nullptr)
        : QAbstractTableModel(parent), m_profile(profile) {}

    // Return number of rows equals the number of basal schedule entries
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return m_profile ? m_profile->getSchedule().size() : 0;
    }

    // Five columns: Time, Basal Rate, Carb  Ratio, Correctiion Factor, Target BG
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return 5;
    }

    // Return the data for each cell, based on the role requested.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Provide header labels for the table view.
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setProfile(Profile*);
    bool removeBasalRow(int row, const QModelIndex &parent = QModelIndex());
    void addBasalRow(BasalSchedule*, const QModelIndex& parent = QModelIndex());
    bool moveBasalRow(int prevRow, int newRow, const QModelIndex &parent = QModelIndex());

private:
    Profile* m_profile;  // Pointer to the Profile object
};

#endif // BASALSCHEDULEMODEL_H
