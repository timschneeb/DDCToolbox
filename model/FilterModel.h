#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include "model/Biquad.h"
#include "model/DeflatedBiquad.h"

#include <QMessageBox>
#include <QModelIndex>
#include <QUndoCommand>
#include <QWidget>
#include <QDebug>

#define FREQ_MIN 0
#define FREQ_MAX 24000
#define BW_MIN 0
#define BW_MAX 100
#define GAIN_MIN -40
#define GAIN_MAX 40

class FilterModel : public QAbstractTableModel {
    Q_OBJECT

    QVector<Biquad*> m_data = QVector<Biquad*>();
    QUndoStack *m_stack;

    enum Section : int {
        Type = 0,
        Freq = 1,
        Bw   = 2,
        Gain = 3
    };

public:
    FilterModel(QWidget * parent = {}) : QAbstractTableModel{parent}, m_stack(new QUndoStack(parent)) {}
    int rowCount(const QModelIndex & = QModelIndex()) const override { return m_data.count(); }
    int columnCount(const QModelIndex & = QModelIndex()) const override { return 4; }

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
        switch (section) {
        case Type: return "Type";
        case Freq: return "Frequency";
        case Bw: return "Bandwidth/Slope";
        case Gain: return "Gain";
        default: return {};
        }
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return  Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | QAbstractTableModel::flags(index);
    }

    void sort(int column, Qt::SortOrder order) override{
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

    Biquad* getFilter(int row){
        return m_data[row];
    }

    const QVector<Biquad*>& getFilterBank() const {
        return m_data;
    }

    void append(Biquad* filter) {
        beginInsertRows({}, m_data.count(), m_data.count());
        m_data.push_back(filter);
        endInsertRows();

        QModelIndex begin = index(m_data.count(), 0);
        QModelIndex end = index(m_data.count(), 3);
        emit dataChanged(begin, end);

        this->sort(Freq, Qt::AscendingOrder);
    }

    bool remove(Biquad* filter){
        for(int i = 0; i < m_data.length(); i++){
            if(m_data[i] == filter){
                removeRows(i, 1);
                return true;
            }
        }
        return false;
    }

    bool removeById(uint32_t id){
        for(int i = 0; i < m_data.length(); i++){
            if(m_data[i]->GetId() == id){
                removeRows(i, 1);
                return true;
            }
        }
        return false;
    }

    void clear(){
        if(!m_data.empty()){
            beginResetModel();
            m_data.clear();
            endResetModel();
        }
    }

    void replace(QModelIndex index, DeflatedBiquad current, bool stealth = false){
        if(current.type == FilterType::CUSTOM)
            m_data[index.row()]->RefreshFilter(current.type, current.c441, current.c48);
        else
            m_data[index.row()]->RefreshFilter(current.type, current.gain, current.freq, current.bwOrSlope);

        if(!stealth)
            emit dataChanged(index, index);
    }


    QVector<float> getMagnitudeResponseTable(int nBandCount, double dSRate)
    {
        QVector<float> vector;
        if (nBandCount <= 0)
        {
            return vector;
        }

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
    QVector<float> getPhaseResponseTable(int nBandCount, double dSRate)
    {
        QVector<float> vector;
        if (nBandCount <= 0)
        {
            return vector;
        }

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

    QVector<float> getGroupDelayTable(int nBandCount, double dSRate)
    {
        QVector<float> vector;
        if (nBandCount <= 0)
        {
            return vector;
        }

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

    std::list<double> exportCoeffs(double dSamplingRate)
    {
        std::list<double> numArray;
        for (int i = 0; i < m_data.size(); i++)
        {
            std::list<double> numArray2 = m_data[i]->ExportCoeffs(dSamplingRate);
            if (numArray2.empty())
            {
                qWarning() << "Export failed: Biquad::ExportCoeffs returned empty list";
                return std::list<double>();
            }
            numArray.splice(numArray.end(), numArray2);
        }

        return numArray;
    }

signals:
    void filterEdited(DeflatedBiquad previous, DeflatedBiquad current, QModelIndex index);

};

#endif // FILTERMODEL_H

