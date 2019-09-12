#ifndef TEXTPOPUP_H
#define TEXTPOPUP_H

#include <QDialog>
#include <QString>

namespace Ui {
class TextPopup;
}

class TextPopup : public QDialog
{
    Q_OBJECT

public:
    explicit TextPopup(QString,QWidget *parent = nullptr);
    ~TextPopup();

private:
    Ui::TextPopup *ui;
};

#endif // TEXTPOPUP_H
