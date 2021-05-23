#ifndef SAMPLERATECHOOSEDIALOG_H
#define SAMPLERATECHOOSEDIALOG_H

#include <QDialog>

namespace Ui {
class SamplerateChooseDialog;
}

class SamplerateChooseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SamplerateChooseDialog(QWidget *parent = nullptr);
    ~SamplerateChooseDialog();

    int getResult();

private:
    Ui::SamplerateChooseDialog *ui;
};

#endif // SAMPLERATECHOOSEDIALOG_H
