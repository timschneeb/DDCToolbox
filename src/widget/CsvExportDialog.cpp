#include "CsvExportDialog.h"
#include "ui_CsvExportDialog.h"

CsvExportDialog::CsvExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CsvExportDialog)
{
    ui->setupUi(this);
}

CsvExportDialog::~CsvExportDialog()
{
    delete ui;
}

CsvExportDialog::Delimiter CsvExportDialog::delimiter() const
{
    return (Delimiter) ui->delimiter->currentIndex();
}

CsvExportDialog::Format CsvExportDialog::format() const
{
    return (Format) ui->format->currentIndex();
}

CsvExportDialog::NumericFormat CsvExportDialog::numFormat() const
{
    return (NumericFormat) ui->numericFormat->currentIndex();
}

int CsvExportDialog::samplerate() const
{
    return ui->samplerate->value();
}

bool CsvExportDialog::includeHeader() const
{
    return ui->includeHeader->currentIndex() == 0;
}
