#include "basalschedulemodel.h"

QVariant BasalScheduleModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || !m_profile)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const QVector<BasalSchedule*>& scheduleList = m_profile->getSchedule();
        if (index.row() >= scheduleList.size())
            return QVariant();

        // Get the basal schedule at the given row.
        const BasalSchedule* entry = scheduleList.at(index.row());
        if (!entry)
            return QVariant();

        // For column 0 return time; for column 1 return rate.
        if (index.column() == 0)
            return entry->getTime().toString("hh:mm");
        else if (index.column() == 1)
            return entry->getBasalRate();
        else if (index.column() == 2)
            return entry->getCarbRatio();
        else  if (index.column() == 3)
            return entry->getCorrFactor();
        else if (index.column() == 4)
            return entry->getTargetBG();
    }
    return QVariant();
}

// Provide header labels for the table view.
QVariant BasalScheduleModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section == 0)
            return QString("Time");
        else if (section == 1)
            return QString("Basal Rate");
        else if (section == 2)
            return QString("Carb Ratio");
        else  if (section == 3)
            return QString("Correction Factor");
        else if (section == 4)
            return QString("Target BG");
    }
    return QVariant();
}

void BasalScheduleModel::setProfile(Profile* newProfile) {
    beginResetModel();
    m_profile = newProfile;
    endResetModel();
}

bool BasalScheduleModel::removeBasalRow(int row, const QModelIndex &parent)
{
    if (row < 0 || row >= m_profile->getSchedule().size())
        return false;

    beginRemoveRows(parent, row, row);
    m_profile->removeBasalSchedule(row);  // This removes the item from the QVector in your Profile.
    endRemoveRows();
    return true;
}

void BasalScheduleModel::addBasalRow(BasalSchedule* schedule, const QModelIndex &parent) {
    int row = m_profile->getSchedule().size()-1;;
    beginInsertRows(parent, row, row);
    m_profile->addBasalSchedule(schedule);
    endInsertRows();
}

bool BasalScheduleModel::moveBasalRow(int prevRow, int newRow, const QModelIndex &parent) {
    if (!m_profile) return false;

    const QVector<BasalSchedule*>& scheduleList = m_profile->getSchedule();
    int count = scheduleList.size();
    if (prevRow < 0 || prevRow >= count || newRow < 0 || newRow >= count)
       return false;
    if (prevRow == newRow)
       return true;  // No movement needed.

    // When moving downward (i.e. destinationRow > sourceRow), the Qt docs require that you
    // specify destinationRow+1 for beginMoveRows() because the row is removed first.
    int beginDestination = (newRow > prevRow) ? newRow + 1 : newRow;

    if (!beginMoveRows(parent, prevRow, newRow, parent, beginDestination))
       return false;

    // Use QVector's move() function to relocate the element.
    m_profile->moveBasalSchedule(prevRow, newRow);

    endMoveRows();
    return true;
}
