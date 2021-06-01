#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include "model/Biquad.h"
#include "model/DeflatedBiquad.h"

#include <QAbstractTableModel>

#define FREQ_MIN 0
#define FREQ_MAX 24000
#define BW_MIN 0
#define BW_MAX 100
#define GAIN_MIN -40
#define GAIN_MAX 40

class QUndoStack;

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
    FilterModel(QWidget * parent = {});
    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void sort(int column, Qt::SortOrder order) override;

    Biquad* getFilter(int row);
    Biquad* getFilterById(uint id);
    const QVector<Biquad*>& getFilterBank() const;

    void append(Biquad* filter);
    void appendAll(QVector<Biquad*> filters);
    void appendAllDeflated(QVector<DeflatedBiquad> filters);
    bool remove(Biquad* filter);
    bool removeById(uint32_t id);
    bool removeAllById(QVector<uint32_t> ids);
    void clear();
    void replace(QModelIndex index, DeflatedBiquad current, bool stealth = false);
    QModelIndex replaceById(uint id, DeflatedBiquad current, bool stealth = false);

    QVector<float> getMagnitudeResponseTable(int nBandCount, double dSRate);
    QVector<float> getPhaseResponseTable(int nBandCount, double dSRate);
    QVector<float> getGroupDelayTable(int nBandCount, double dSRate);

    std::list<double> exportCoeffs(double dSamplingRate, bool noA0Divide = false);

    bool getDebugMode() const;
    void setDebugMode(bool value);

signals:
    void filterEdited(DeflatedBiquad previous, DeflatedBiquad current, QModelIndex index);

private:
    bool debugMode = false;

};

#endif // FILTERMODEL_H

