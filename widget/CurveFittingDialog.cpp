#include "CurveFittingDialog.h"
#include "CurveFittingWorkerDialog.h"
#include "ui_CurveFittingDialog.h"

#include "Expander.h"
#include "utils/CSVParser.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

#include <utils/QInt64Validator.h>

CurveFittingDialog::CurveFittingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveFittingDialog)
{
    ui->setupUi(this);

    // Connect signals
    connect(ui->fileSelection, &QAbstractButton::clicked, this, &CurveFittingDialog::selectFile);
    connect(ui->projectLink, &QAbstractButton::clicked, this, &CurveFittingDialog::visitProject);

    // Rearrange layout and insert expander
    auto * anyLayout = new QVBoxLayout();
    anyLayout->setContentsMargins(6, 0, 0, 0);
    anyLayout->addWidget(ui->widget);
    Expander* spoiler = new Expander("Advanced options", 300, this);
    spoiler->setContentLayout(*anyLayout);
    this->layout()->addWidget(spoiler);
    this->layout()->addWidget(ui->buttonBox);

    // Prepare seed
    ui->adv_random_seed->setValidator(new QInt64Validator(0, INT64_MAX, ui->adv_random_seed));
    ui->adv_random_seed->setText(QString::number((rand() << 16) | rand()));

    // Setup UI
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setText("Calculate");
    ui->status_panel->setVisible(false);
}

CurveFittingDialog::~CurveFittingDialog()
{
    delete ui;
}

void CurveFittingDialog::visitProject()
{
    // TODO: Update project link
    QDesktopServices::openUrl(QUrl("https://github.com/james34602/"));
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

    std::ifstream file(path.toStdString());
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
    this->hide();
    auto worker = new CurveFittingWorkerDialog(freq, gain, this);

    // Launch worker dialog and halt until finished or cancelled
    worker->exec();

    // TODO: Exchange results

    worker->deleteLater();

    /* These vectors must not be cleared or modified while the worker is active.
     * The internal C array of QVector<float> is directly referenced  */
    freq.clear();
    gain.clear();

    QDialog::accept();
}
