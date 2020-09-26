#ifndef STABILITYREPORT_H
#define STABILITYREPORT_H

#include <QDialog>
#include <QTableWidget>
#include <ddccontext.h>

namespace Ui {
class StabilityReport;
}

class StabilityReport : public QDialog
{
    Q_OBJECT

public:
    explicit StabilityReport(const DDCContext& ctx, QWidget *parent = nullptr);
    ~StabilityReport();

    bool isReportPositive();
private:
    Ui::StabilityReport *ui;
    int unstableCount = 0;

    void addIssue(int stability, QString location, QString description);
};

#endif // STABILITYREPORT_H
