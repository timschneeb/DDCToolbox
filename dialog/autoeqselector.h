#ifndef AUTOEQSELECTOR_H
#define AUTOEQSELECTOR_H

#include <QDialog>
#include <QThread>

#include "utils/autoeqclient.h"
#include "overlaymsgproxy.h"

namespace Ui {
class AutoEQSelector;
}

class AutoEQSelector : public QDialog
{
    Q_OBJECT

public:
    explicit AutoEQSelector(QWidget *parent = nullptr);
    ~AutoEQSelector();
    HeadphoneMeasurement getSelection();
protected:
    void appendToList(QueryResult result);
    void showEvent(QShowEvent *event);
private slots:
    void updateDetails();
    void doQuery();
private:
    Ui::AutoEQSelector *ui;
    QSize imgSizeCache;
    HeadphoneMeasurement hpCache;
    OverlayMsgProxy* waitScreen;
};

#endif // AUTOEQSELECTOR_H
