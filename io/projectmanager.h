#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H
#include "utils/filtertypes.h"
#include "item/customfilterfactory.h"

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <cmath>

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    static bool writeProjectFile(std::vector<calibrationPoint_t> points,
                          QString fileName,bool compatibilitymode);
    static bool exportVDC(QString fileName, std::list<double> p1, std::list<double> p2);
    static std::vector<calibrationPoint_t> readProjectFile(QString fileName);
    static calibrationPoint_t readSingleLine(QString str);
};

#endif // PROJECTMANAGER_H
