#ifndef CURVEFITTINGDIALOG_H
#define CURVEFITTINGDIALOG_H

#include "CurveFittingWorkerDialog.h"
#include "Expander.h"
#include "model/DeflatedBiquad.h"

#include <QDialog>

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

private:
    void selectFile();
    void visitProject();
    void setStatus(bool success, QString text);

    Ui::CurveFittingDialog *ui;
    Expander* opt_boundary_constraints = nullptr;
    Expander* advanced_rng = nullptr;

    QVector<double> freq;
    QVector<double> gain;

    QVector<DeflatedBiquad> results;
};

#endif // CURVEFITTINGDIALOG_H
