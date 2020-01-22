#ifndef CONVERSIONENGINE_H
#define CONVERSIONENGINE_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include "filtertypes.h"

class ConversionEngine : public QObject
{
    Q_OBJECT
public:
    ConversionEngine();
    static QString convertVDCtoProjectFile(QString inputVdc);
    static std::vector<calibrationPoint_t> readParametricEQFile(QString path);
};

#endif // CONVERSIONENGINE_H
