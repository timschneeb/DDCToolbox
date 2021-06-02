#ifndef STABILITYREPORT_H
#define STABILITYREPORT_H

#include <QDialog>

class FilterModel;

namespace Ui {
class StabilityReport;
}

class StabilityReport : public QDialog
{
    Q_OBJECT

public:
    explicit StabilityReport(FilterModel* model, QWidget *parent = nullptr);
    ~StabilityReport();

    bool hasIssue() const;
    bool hasCriticalIssue() const;
private:
    Ui::StabilityReport *ui;
    int unstableCount = 0;
    int criticalUnstableCount = 0;

    void addIssue(int stability, const QString& location, const QString& description);
};

#endif // STABILITYREPORT_H
