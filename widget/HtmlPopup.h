#ifndef TEXTPOPUP_H
#define TEXTPOPUP_H

#include <QDialog>
#include <QString>

namespace Ui {
class TextPopup;
}

class HtmlPopup : public QDialog
{
    Q_OBJECT

public:
    explicit HtmlPopup(const QString&,QWidget *parent = nullptr);
    ~HtmlPopup();

private:
    Ui::TextPopup *ui;
};

#endif // TEXTPOPUP_H
