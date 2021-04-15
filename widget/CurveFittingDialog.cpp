#include "CurveFittingDialog.h"
#include "ui_CurveFittingDialog.h"

#include "Expander.h"
#include "utils/CSVParser.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

CurveFittingDialog::CurveFittingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveFittingDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);

    connect(ui->fileSelection, &QAbstractButton::clicked, this, &CurveFittingDialog::selectFile);
    connect(ui->projectLink, &QAbstractButton::clicked, this, &CurveFittingDialog::visitProject);

    ui->status_panel->setVisible(false);

    auto * anyLayout = new QVBoxLayout();
    anyLayout->addWidget(ui->widget);
    Expander* spoiler = new Expander("Advanced options", 300, this);
    spoiler->setContentLayout(*anyLayout);
    this->layout()->addWidget(spoiler);
    this->layout()->addWidget(ui->buttonBox);
}

CurveFittingDialog::~CurveFittingDialog()
{
    delete ui;
}


void CurveFittingDialog::selectFile()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open CSV dataset"),
                                                "", tr("CSV dataset (*.csv)"));
    if(file.isEmpty())
        return;

    ui->filePath->setText(file);
    parseCsv();
}

void CurveFittingDialog::visitProject()
{
    // TODO: Update project link
    QDesktopServices::openUrl(QUrl("https://github.com/james34602/"));
}

void CurveFittingDialog::parseCsv(){
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    freq.clear();
    gain.clear();

    QString path = ui->filePath->text();

    if(!QFile::exists(path)){
        QMessageBox::warning(this, "CSV parser", "Selected file does not exist. Please choose another one.");
        setStatus(false, "File not found");
        return;
    }

    std::ifstream file(path.toStdWString());
    for(auto& row: CSVRange(file))
    {
        float freq_val, gain_val;

        if(row.size() < 1){
            // Row does not contain enough columns, skip it
            continue;
        }

        try {
            // Check if field contains numeric content, otherwise skip row
            freq_val = std::stod(std::string(row[0]));
            gain_val = std::stod(std::string(row[1]));
        } catch (std::invalid_argument &ex) {
            // Row does not contain expected data, skip it
            continue;
        }

        freq.push_back(freq_val);
        gain.push_back(gain_val);
    }

    if(freq.size() < 1 && gain.size() < 1){
        setStatus(false, "No valid rows found");
        return;
    }

    setStatus(true, QString::number(freq.size()) + " rows loaded");
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
}

void CurveFittingDialog::setStatus(bool success, QString text){
    ui->status_panel->setVisible(true);

    if(success)
        ui->status_img->setPixmap(QPixmap(":/img/success.svg"));
    else
        ui->status_img->setPixmap(QPixmap(":/img/critical.svg"));

    ui->status_label->setText(text);
}

void CurveFittingDialog::accept()
{
    // TODO
    QDialog::accept();
}
