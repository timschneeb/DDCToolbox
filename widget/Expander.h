#ifndef EXPANDER_H
#define EXPANDER_H


#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

class Expander : public QWidget {
    Q_OBJECT
private:
    QGridLayout mainLayout;
    QToolButton toggleButton;
    QFrame headerLine;
    QParallelAnimationGroup toggleAnimation;
    QScrollArea contentArea;
    int animationDuration{300};

signals:
    void stateChanged(bool open);

public:
    explicit Expander(const QString & title = "", const int animationDuration = 300, QWidget *parent = 0);
    void setContentLayout(QLayout & contentLayout);
    void setState(bool state);
};

#endif // EXPANDER_H
