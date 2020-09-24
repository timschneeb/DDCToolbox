#ifndef CONVERSIONENGINE_H
#define CONVERSIONENGINE_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include "utils/filtertypes.h"

class ConversionEngine : public QObject
{
    Q_OBJECT
public:
    ConversionEngine();
    static QString convertVDCtoProjectFile(const QString& inputVdc);
    static std::vector<calibrationPoint_t> readParametricEQFile(const QString& path);
    static std::vector<calibrationPoint_t> readParametricEQString(QString string);
};

#endif // CONVERSIONENGINE_H
