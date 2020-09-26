#include "StabilityReport.h"
#include "ui_StabilityReport.h"

#include "utils/filtertypes.h"
#include "mainwindow.h"

#include <cassert>

StabilityReport::StabilityReport(const DDCContext& ctx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StabilityReport)
{
    ui->setupUi(this);
    this->setModal(true);

    MainWindow* ui_ctx = dynamic_cast<MainWindow*>(parent);

    for (auto point : ctx.GetFilterBank()){
        int id               = point.first;
        Biquad* filter       = point.second;
        QString filtertype   = typeToString(filter->GetFilterType());

        if(filter != nullptr){
            QString issue;

            Biquad::Stability stability = filter->IsStable();
            if(stability == Biquad::UNSTABLE)
                issue = QString("Pole outside the unit circle");
            else if(stability == Biquad::PARTIALLY_STABLE)
                issue = QString("Pole approaches the unit circle");
            else
                continue;

            unstableCount++;

            int row = ui_ctx != nullptr ? ui_ctx->getRowById(id) + 1 : 0;
            addIssue(stability,
                     QString("%1 at %2Hz (row %3)").arg(filtertype).arg(round(filter->GetFrequency())).arg(row),
                     issue);
        }
    }

    if(unstableCount < 1)
        ui->summary->setText("All filters appear to be stable");
    else
        ui->summary->setText(QString("%1 issue(s) found. Please fix the problems listed below.").arg(unstableCount));

    ui->issueTable->resizeColumnsToContents();
}

StabilityReport::~StabilityReport()
{
    delete ui;
}

bool StabilityReport::isReportPositive(){
    return unstableCount < 1;
}

void StabilityReport::addIssue(int stable, QString location, QString description){

    if(stable == Biquad::STABLE)
        return;

    auto tw = ui->issueTable;
    tw->insertRow(tw->rowCount());

#define ADD_ITEM(index, value) \
    QTableWidgetItem *c##index = new QTableWidgetItem(); \
    c##index->setText(value); \
    tw->setItem(tw->rowCount()-1, index, c##index);
#define ADD_ICON(index, path) \
    QTableWidgetItem *c##index = new QTableWidgetItem(); \
    QLabel *lbl = new QLabel; \
    lbl->setPixmap(QPixmap(path)); \
    lbl->setAlignment(Qt::AlignCenter); \
    tw->setItem(tw->rowCount()-1, index, c##index); \
    tw->setCellWidget(tw->rowCount()-1, index, lbl);

    ADD_ICON(0, stable == Biquad::UNSTABLE ? ":/img/critical.svg" : ":/img/warning.svg");
    ADD_ITEM(1, location);
    ADD_ITEM(2, description);

#undef ADD_ITEM
#undef ADD_ICON
}
