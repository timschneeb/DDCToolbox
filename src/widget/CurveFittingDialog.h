#ifndef CURVEFITTINGDIALOG_H
#define CURVEFITTINGDIALOG_H

#include "model/DeflatedBiquad.h"
#include "model/CurveFittingOptions.h"

#include <QDialog>

class Expander;

namespace Ui {
class CurveFittingDialog;
}

class CurveFittingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CurveFittingDialog(QWidget *parent = nullptr);
    ~CurveFittingDialog();

    QVector<DeflatedBiquad> getResults() const;

protected slots:
    void accept();

private slots:
    void parseCsv();
    void updateSupportedProperties(int index);
    void setWindowExpanded(bool b);
    void updatePreviewPlot();

    void contextMenuRequest(QPoint pos);
    void moveLegend();

private:
    void selectFile();
    void visitProject();
    void setStatus(bool success, const QString& text);

    Ui::CurveFittingDialog *ui;
    Expander* advanced_rng = nullptr;
    Expander* fgrid = nullptr;
    Expander* algo_de = nullptr;
    Expander* algo_chio = nullptr;
    Expander* algo_sgd = nullptr;

    QVector<double> freq;
    QVector<double> gain;

    QVector<DeflatedBiquad> results;
    DoubleRange calculateYAxisRange(bool exportDataset, double** freq = nullptr, double** gain = nullptr);
};

#endif // CURVEFITTINGDIALOG_H
