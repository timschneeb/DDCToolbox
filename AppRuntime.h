#ifndef APPRUNTIME_H
#define APPRUNTIME_H

#include "VdcEditorWindow.h"

#include <QApplication>

class AppRuntime : public QApplication
{
public:
    AppRuntime(int &argc, char **argv);
    bool event(QEvent *event);

private:
    VdcEditorWindow* _window = nullptr;
};

#endif // APPRUNTIME_H
