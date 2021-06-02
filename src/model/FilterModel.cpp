#include "FilterModel.h"

#include <QDebug>
#include <QMessageBox>
#include <QModelIndex>
#include <QUndoCommand>
#include <QWidget>

#include "model/command/EditCommand.h"

FilterModel::FilterModel(QWidget *parent) : QAbstractTableModel{parent}, m_stack(new QUndoStack(parent)) {}

int FilterModel::rowCount(const QModelIndex &) const { return m_data.count(); }

int FilterModel::columnCount(const QModelIndex &) const { return 4; }

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

QVariant FilterModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::BackgroundRole)
    {
        switch (m_data[index.row()]->IsStable())
        {
        case Biquad::UNSTABLE:
            return QVariant(QColor(255, 149, 117));
        case Biquad::PARTIALLY_STABLE:
            return QVariant(QColor(255, 210, 66));
        default:
            return {};
        }
    }

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

QVariant FilterModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    switch (section) {
    case Type: return "Type";
    case Freq: return "Frequency";
    case Bw: return "Bandwidth";
    case Gain: return "Gain";
    default: return {};
    }
}

Qt::ItemFlags FilterModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | QAbstractTableModel::flags(index);
}

void FilterModel::sort(int column, Qt::SortOrder order){
    if(m_data.empty())
        return;

    beginResetModel();
    switch (column) {
    case Type:
        std::sort(m_data.begin(), m_data.end(), Biquad::compareType);
        break;
    case Freq:
        std::sort(m_data.begin(), m_data.end(), Biquad::compareFrequency);
        break;
    case Bw:
        std::sort(m_data.begin(), m_data.end(), Biquad::compareBwOrSlope);
        break;
    case Gain:
        std::sort(m_data.begin(), m_data.end(), Biquad::compareGain);
        break;
    default:
        break;
    }

    if(order == Qt::DescendingOrder)
        std::reverse(m_data.begin(), m_data.end());

    endResetModel();
}

Biquad *FilterModel::getFilter(int row){
    return m_data[row];
}

Biquad *FilterModel::getFilterById(uint id){
    for(int i = 0; i < m_data.length(); i++){
        if(m_data[i]->GetId() == id){
            return m_data[i];
        }
    }

    return nullptr;
}

const QVector<Biquad *> &FilterModel::getFilterBank() const {
    return m_data;
}

void FilterModel::append(Biquad *filter) {
    beginInsertRows({}, m_data.count(), m_data.count());
    m_data.push_back(filter);
    endInsertRows();

    QModelIndex begin = index(m_data.count(), 0);
    QModelIndex end = index(m_data.count(), 3);
    emit dataChanged(begin, end);

    this->sort(Freq, Qt::AscendingOrder);
}

void FilterModel::appendAll(const QVector<Biquad *>& filters) {
    beginInsertRows({}, m_data.count(), m_data.count());
    for(const auto& filter : filters){
        m_data.push_back(filter);
    }
    endInsertRows();

    QModelIndex begin = index(m_data.count() - filters.count(), 0);
    QModelIndex end = index(m_data.count(), 3);
    emit dataChanged(begin, end);

    this->sort(Freq, Qt::AscendingOrder);
}

void FilterModel::appendAllDeflated(const QVector<DeflatedBiquad>& filters) {
    QVector<Biquad*> vector;
    vector.reserve(filters.count());
    for(const auto& filter : filters){
        vector.push_back(filter.inflate());
    }
    appendAll(vector);
}

bool FilterModel::remove(Biquad *filter){
    for(int i = 0; i < m_data.length(); i++){
        if(m_data[i] == filter){
            removeRows(i, 1);
            return true;
        }
    }
    return false;
}

bool FilterModel::removeById(uint32_t id){
    for(int i = 0; i < m_data.length(); i++){
        if(m_data[i]->GetId() == id){
            removeRows(i, 1);
            return true;
        }
    }
    return false;
}

bool FilterModel::removeAllById(const QVector<uint32_t>& ids){
    QVector<int> rows;
    for(const auto& id : ids){
        for(int i = 0; i < m_data.length(); i++){
            if(m_data[i]->GetId() == id){
                rows.append(i);
            }
        }
    }

    if(rows.count() < 1){
        qWarning() << "FilterModel::removeAllById(): No matching rows found to remove";
        return false;
    }

    std::sort(rows.begin(), rows.end(), std::greater<int>());

    beginRemoveRows({}, rows.last(), rows.first());
    for(const int& row : rows){
        delete m_data[row]; // TODO: Safe deletion without undo/redo corruption
        m_data[row] = nullptr;
        m_data.remove(row, 1);
    }
    endRemoveRows();

    QModelIndex begin = index(rows.last(), 0);
    QModelIndex end = index(rows.first(), 3);
    emit dataChanged(begin, end);
    return true;
}

void FilterModel::clear(){
    if(!m_data.empty()){
        beginResetModel();
        for(int i = 0; i < m_data.size(); i++){
            delete m_data[i];
            m_data[i] = nullptr;
        }
        m_data.clear();
        endResetModel();
    }
}

void FilterModel::replace(QModelIndex index, DeflatedBiquad current, bool stealth){
    if(current.type == FilterType::CUSTOM)
        m_data[index.row()]->RefreshFilter(current.type, current.c441, current.c48);
    else
        m_data[index.row()]->RefreshFilter(current.type, current.gain, current.freq, current.bwOrSlope);

    if(!stealth)
        emit dataChanged(index, index.siblingAtColumn(3));
}

QModelIndex FilterModel::replaceById(uint id, DeflatedBiquad current, bool stealth)
{
    QModelIndex index;
    for(int i = 0; i < m_data.length(); i++){
        if(m_data[i]->GetId() == id){
            index = this->index(i, 0);
        }
    }

    if(!index.isValid()){
        qWarning() << "FilterModel::replaceById: Index of filter id" << id << "not found";
        return index;
    }

    if(current.type == FilterType::CUSTOM)
        m_data[index.row()]->RefreshFilter(current.type, current.c441, current.c48);
    else
        m_data[index.row()]->RefreshFilter(current.type, current.gain, current.freq, current.bwOrSlope);

    if(!stealth)
        emit dataChanged(index, index.siblingAtColumn(3));

    return index;
}

QVector<float> FilterModel::getMagnitudeResponseTable(int nBandCount, double dSRate)
{
    QVector<float> vector;
    if (nBandCount <= 0)
    {
        return vector;
    }

    vector.reserve(nBandCount);
    for (int j = 0; j < nBandCount; j++)
    {
        double num3 = (dSRate / 2.0) / ((double) nBandCount);
        float val = 0.0f;
        for (auto & k : m_data)
            val += (float) k->GainAt(num3 * (j + 1.0), dSRate);
        vector.push_back(val);
    }
    return vector;
}

QVector<float> FilterModel::getPhaseResponseTable(int nBandCount, double dSRate)
{
    QVector<float> vector;
    if (nBandCount <= 0)
    {
        return vector;
    }

    vector.reserve(nBandCount);
    for (int j = 0; j < nBandCount; j++)
    {
        double num3 = (dSRate / 2.0) / ((double) nBandCount);
        float val = 0.0f;
        for (auto & k : m_data)
            val += (float) k->PhaseResponseAt(num3 * (j + 1.0), dSRate);
        vector.push_back(val);
    }
    return vector;
}

QVector<float> FilterModel::getGroupDelayTable(int nBandCount, double dSRate)
{
    QVector<float> vector;
    if (nBandCount <= 0)
    {
        return vector;
    }

    vector.reserve(nBandCount);
    for (int j = 0; j < nBandCount; j++)
    {
        double num3 = (dSRate / 2.0) / ((double) nBandCount);
        float val = 0.0f;
        for (auto & k : m_data)
            val += (float) k->GroupDelayAt(num3 * (j + 1.0), dSRate);
        vector.push_back(val);
    }
    return vector;
}

std::list<double> FilterModel::exportCoeffs(double dSamplingRate, bool noA0Divide)
{
    std::list<double> numArray;
    for (int i = 0; i < m_data.size(); i++)
    {
        std::list<double> numArray2 = m_data[i]->ExportCoeffs(dSamplingRate, noA0Divide);
        if (numArray2.empty())
        {
            qWarning() << "Export failed: Biquad::ExportCoeffs returned empty list";
            return std::list<double>();
        }
        numArray.splice(numArray.end(), numArray2);
    }

    return numArray;
}

bool FilterModel::getDebugMode() const
{
    return debugMode;
}

void FilterModel::setDebugMode(bool value)
{
    debugMode = value;
}
