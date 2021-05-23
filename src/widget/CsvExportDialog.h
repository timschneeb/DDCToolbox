#ifndef CSVEXPORTDIALOG_H
#define CSVEXPORTDIALOG_H

#include <QDialog>

namespace Ui {
class CsvExportDialog;
}

class CsvExportDialog : public QDialog
{
    Q_OBJECT
public:
    enum Delimiter
    {
        SComma = 0,
        SSemicolon = 1,
        SSpace = 2,
        STab = 3
    };
    enum Format
    {
        F_B0B1B2A0A1A2 = 0,
        F_A0A1A2B0B1B2 = 1
    };
    enum NumericFormat
    {
        NFloat = 0,
        NScientific = 1
    };


public:
    explicit CsvExportDialog(QWidget *parent = nullptr);
    ~CsvExportDialog();

    Delimiter delimiter() const;
    Format format() const;
    NumericFormat numFormat() const;
    int samplerate() const;
    bool includeHeader() const;

private:
    Ui::CsvExportDialog *ui;
};

#endif // CSVEXPORTDIALOG_H
