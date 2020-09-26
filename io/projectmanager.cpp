#include "projectmanager.h"
using namespace std;
#define n QString("\n")

bool ProjectManager::writeProjectFile(std::vector<calibrationPoint_t> points,
                                      const QString& fileName,bool compatibilitymode){
    QFile caFile(fileName);
    caFile.open(QIODevice::WriteOnly | QIODevice::Text);

    if(!caFile.isOpen())
        return false;

    /* Check if current calibration points
       are compatible with vipers toolbox v2.0 */
    if(!compatibilitymode){
        bool compatible = true;
        for(calibrationPoint_t point:points)
            if(point.type!=Biquad::PEAKING)
                compatible = false;
        compatibilitymode = compatible;
    }

    QTextStream outStream(&caFile);
    if(compatibilitymode){
        outStream << "# DDCToolbox Project File, v1.0.0.0 (@ThePBone)" + n;
        outStream << n;
        for (size_t i = 0; i < points.size(); i++)
        {
            calibrationPoint_t cal = points.at(i);
            outStream << "# Calibration Point " + QString::number(i + 1) + n;
            outStream << QString::number(cal.freq) + "," + QString::number(cal.bw) + "," + QString::number(cal.gain) + n;
        }
    }else{
        outStream << "# DDCToolbox Project File, v4.0.0.0 (@ThePBone)" + n;
        outStream << n;
        for (size_t i = 0; i < points.size(); i++)
        {
            calibrationPoint_t cal = points.at(i);
            outStream << "# Calibration Point " + QString::number(i + 1) + n;
            if(cal.type==Biquad::CUSTOM){
                outStream << QString::number(cal.freq) + ",0,0," + typeToString(cal.type) + ";";
                outStream << QString::number(cal.custom441.a0,'f',16) + "," + QString::number(cal.custom441.a1,'f',16) + "," + QString::number(cal.custom441.a2,'f',16) + ",";
                outStream << QString::number(cal.custom441.b0,'f',16) + "," + QString::number(cal.custom441.b1,'f',16) + "," + QString::number(cal.custom441.b2,'f',16) + ",";
                outStream << QString::number(cal.custom48.a0,'f',16) + "," + QString::number(cal.custom48.a1,'f',16) + "," + QString::number(cal.custom48.a2,'f',16) + ",";
                outStream << QString::number(cal.custom48.b0,'f',16) + "," + QString::number(cal.custom48.b1,'f',16) + "," + QString::number(cal.custom48.b2,'f',16)  + n;
            } else
                outStream << QString::number(cal.freq) + "," + QString::number(cal.bw) + "," + QString::number(cal.gain) + "," + typeToString(cal.type) + n;
        }
    }
    outStream << n;
    outStream << "#File End" + n;
    caFile.close();
    return true;
}

bool ProjectManager::exportVDC(QString fileName, const std::list<double>& p1, const std::list<double>& p2){
    if (fileName != "" && fileName != nullptr)
    {
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        if(ext!="vdc")fileName.append(".vdc");

        QFile caFile(fileName);
        caFile.open(QIODevice::WriteOnly | QIODevice::Text);

        if(!caFile.isOpen())
            return false;

        QTextStream outStream(&caFile);
        outStream << "SR_44100:";

        std::vector<double> v1;
        for (double const &d: p1)
            v1.push_back(d);
        std::vector<double> v2;
        for (double const &d: p2)
            v2.push_back(d);

        for (size_t i = 0; i < v1.size(); i++)
        {
            outStream << qSetRealNumberPrecision(16) << v1.at(i);
            if (i != (v1.size() - 1))
                outStream << ",";
        }
        outStream << n;
        outStream << "SR_48000:";

        for (size_t i = 0; i < v2.size(); i++)
        {
            outStream << qSetRealNumberPrecision(16) << v2.at(i);
            if (i != (v2.size() - 1))
                outStream << ",";
        }

        outStream << n;
        caFile.close();
        return true;
    }
    return false;
}

std::vector<calibrationPoint_t> ProjectManager::readProjectFile(const QString& fileName)
{
    std::vector<calibrationPoint_t> buffer;
    if (fileName != "" && fileName != nullptr){
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return buffer;

        QTextStream in(&file);
        while (!in.atEnd()){
            auto point = readSingleLine(in.readLine().trimmed());
            if(point.type != Biquad::INVALID)
                buffer.push_back(point);
        }
        file.close();
    }
    return buffer;
}

calibrationPoint_t ProjectManager::readSingleLine(const QString& str){
    calibrationPoint_t cal;
    cal.freq = 0;
    cal.bw = 0;
    cal.gain = 0;
    cal.type = Biquad::INVALID;
    if (str != nullptr || str != "")
    {
        if ((str.length() > 0) && !str.startsWith("#"))
        {
            bool isCustomLine = false;
            QStringList strArray;
            if(str.contains(";")){
                strArray = str.split(";")[0].split(',');
                isCustomLine = true;
            }
            else strArray = str.split(',');
            //Load legacy project data
            if ((!strArray.empty()) && (strArray.length() == 3))
            {
                int result = 0;
                double num2 = 0.0;
                double num3 = 0.0;
                if ((sscanf(strArray[0].toUtf8().constData(), "%d", &result) == 1 &&
                     sscanf(strArray[1].toUtf8().constData(), "%lf", &num2) == 1) &&
                        sscanf(strArray[2].toUtf8().constData(), "%lf", &num3) == 1)
                {
                    if(result<=0||num2<0)return cal;
                    if(isnan(num2)||isnan(num3))return cal;
                    if(isinf(num2)||isinf(num3))return cal;

                    cal.type = Biquad::PEAKING;
                    cal.freq = result;
                    cal.bw = num2;
                    cal.gain = num3;
                }
            }
            //Load v3 project data
            else if ((!strArray.empty()) && (strArray.length() == 4))
            {
                if(isCustomLine){
                    QString coeffpart = str.split(";")[1];
                    customFilter_t c441 = defaultCustomFilter();
                    customFilter_t c48 = defaultCustomFilter();
                    int counter = 0;
                    for(const auto& coeff : coeffpart.split(",")){
                        switch(counter){
                        case 0:
                            c441.a0 = coeff.toDouble();
                            break;
                        case 1:
                            c441.a1 = coeff.toDouble();
                            break;
                        case 2:
                            c441.a2 = coeff.toDouble();
                            break;
                        case 3:
                            c441.b0 = coeff.toDouble();
                            break;
                        case 4:
                            c441.b1 = coeff.toDouble();
                            break;
                        case 5:
                            c441.b2 = coeff.toDouble();
                            break;
                        case 6:
                            c48.a0 = coeff.toDouble();
                            break;
                        case 7:
                            c48.a1 = coeff.toDouble();
                            break;
                        case 8:
                            c48.a2 = coeff.toDouble();
                            break;
                        case 9:
                            c48.b0 = coeff.toDouble();
                            break;
                        case 10:
                            c48.b1 = coeff.toDouble();
                            break;
                        case 11:
                            c48.b2 = coeff.toDouble();
                            break;
                        }
                        counter++;
                    }

                    cal.type = Biquad::CUSTOM;
                    cal.custom441 = c441;
                    cal.custom48 = c48;
                }
                else{
                    int result = 0;
                    double num2 = 0.0;
                    double num3 = 0.0;
                    if ((sscanf(strArray[0].toUtf8().constData(), "%d", &result) == 1 &&
                         sscanf(strArray[1].toUtf8().constData(), "%lf", &num2) == 1) &&
                            sscanf(strArray[2].toUtf8().constData(), "%lf", &num3) == 1)
                    {
                        if(result<=0||num2<0)return cal;
                        if(isnan(num2)||isnan(num3))return cal;
                        if(isinf(num2)||isinf(num3))return cal;

                        Biquad::Type filtertype = stringToType(strArray[3].trimmed());
                        cal.type = filtertype;
                        cal.freq = result;
                        cal.bw = num2;
                        cal.gain = num3;
                    }
                }
            }
        }
    }
    return cal;
}
