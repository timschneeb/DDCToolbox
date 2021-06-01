#include "FilterModel.h"

#include "model/command/EditCommand.h"

bool FilterModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    beginRemoveRows({}, row, row + count - 1);
    for(int i = row; i < count; i++){
        delete m_data[i];
        m_data[i] = nullptr;
    }
    m_data.remove(row, count);
    endRemoveRows();

    QModelIndex begin = index(row, 0);
    QModelIndex end = index(row, 3);
    emit dataChanged(begin, end);
    return true;
}

bool FilterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (!checkIndex(index))
            return false;

        DeflatedBiquad backup = DeflatedBiquad(m_data[index.row()]);

        switch (index.column()) {
        case 0:
            m_data[index.row()]->SetFilterType(FilterType(value.toString()));
            break;
        case 1:
            if(value.toInt() < FREQ_MIN || value.toInt() > FREQ_MAX){
                QMessageBox::warning(qobject_cast<QWidget*>(parent()), tr("Warning"),
                                     tr("Frequency value '%1' is out of range (%2 ~ %3)")
                                     .arg(value.toInt()).arg(FREQ_MIN).arg(FREQ_MAX));
                return false;
            }

            m_data[index.row()]->SetFrequency(value.toInt());
            break;
        case 2:
            if(value.toDouble() < BW_MIN || value.toDouble() > BW_MAX){
                QMessageBox::warning(qobject_cast<QWidget*>(parent()), tr("Warning"),
                                     tr("Bandwidth value '%1' is out of range (%2 ~ %3)")
                                     .arg(value.toDouble()).arg(BW_MIN).arg(BW_MAX));
                return false;
            }

            m_data[index.row()]->SetBandwidthOrSlope(value.toDouble());
            break;
        case 3:
            if(value.toDouble() < GAIN_MIN || value.toDouble() > GAIN_MAX){
                QMessageBox::warning(qobject_cast<QWidget*>(parent()), tr("Warning"),
                                     tr("Gain value '%1' is out of range (%2 ~ %3)")
                                     .arg(value.toDouble()).arg(GAIN_MIN).arg(GAIN_MAX));
                return false;
            }

            m_data[index.row()]->SetGain(value.toDouble());
            break;
        };

        /* No change */
        if(backup == DeflatedBiquad(m_data[index.row()]))
            return false;

        emit filterEdited(backup, DeflatedBiquad(m_data[index.row()]), index);
        return true;
    }
    return false;
}

QVariant FilterModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        const auto & filter = m_data[index.row()];
        switch (index.column()) {
        case Type: return filter->GetFilterType().operator QString();
        case Freq: return filter->GetFrequency();
        case Bw:   return filter->GetBandwithOrSlope();
        case Gain: return filter->GetGain();
        default: return {};
        };
    }

    return {};
}

bool FilterModel::getDebugMode() const
{
    return debugMode;
}

void FilterModel::setDebugMode(bool value)
{
    debugMode = value;
}
