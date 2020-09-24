#include "conversionengine.h"
#include "utils/vdcimporter.h"

#include <QDebug>
#include <QRegularExpression>

ConversionEngine::ConversionEngine()
= default;

QString ConversionEngine::convertVDCtoProjectFile(const QString& inputVdc){
    QFile file(inputVdc);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";

    QTextStream in(&file);
    QByteArray ba = in.readAll().toLatin1();
    char* textString = ba.data();

    DirectForm2 **df441, **df48;
    int sosCount = DDCParser(textString, &df441, &df48);
    char *vdcprj = VDC2vdcprj(df48, 48000.0, sosCount);

    QString prj = QString::fromStdString(vdcprj);

    free(vdcprj);
    for (int i = 0; i < sosCount; i++)
    {
        free(df441[i]);
        free(df48[i]);
    }
    free(df441);
    free(df48);

    return prj;
}

std::vector<calibrationPoint_t> ConversionEngine::readParametricEQFile(const QString& path){
    std::vector<calibrationPoint_t> points;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return points;

    points = readParametricEQString(file.readAll());

    file.close();
    return points;
}

std::vector<calibrationPoint_t> ConversionEngine::readParametricEQString(QString string){
    std::vector<calibrationPoint_t> points;
    QString str;
    QTextStream in(&string);
    while (!in.atEnd())
    {
        str = in.readLine().trimmed();
        if (str != nullptr || str != "")
        {
            if ((str.length() > 0) && !str.startsWith("#"))
            {

                QRegularExpression re
                        (R"(Filter\s\d+[\s\S][^PK]*PK\s+Fc\s+(?<hz>\d+)\s*Hz\s*Gain\s*(?<gain>-?\d*.?\d+)\s*dB\s*Q\s*(?<q>-?\d*.?\d+))");
                QRegularExpressionMatch match = re.match(str);
                if(!match.hasMatch())
                    continue;

                bool freq_ok = false;
                int freq = match.captured("hz").toInt(&freq_ok);
                bool gain_ok = false;
                double gain = match.captured("gain").toDouble(&gain_ok);
                bool q_ok = false;
                double q = match.captured("q").toDouble(&q_ok);

                if(freq_ok && gain_ok && q_ok){
                    if(freq < 0)
                        continue;

                    //Convert Q to BW
                    double QQ1st = ((2*q*q)+1)/(2*q*q);
                    double QQ2nd = pow(2*QQ1st,2)/4;
                    double bw = round(1000000*log(QQ1st+sqrt(QQ2nd-1))/log(2))/1000000;

                    calibrationPoint_t cal;
                    cal.freq = freq;
                    cal.bw = bw;
                    cal.gain = gain;
                    cal.type = biquad::Type::PEAKING; //TODO: add filtertype to eapo/autoeq parser
                    points.push_back(cal);
                }
                else {
                    qWarning().noquote().nospace() << "Parsing error: invalid line -> " << str;
                    continue;
                }
            }
        }
    }
    return points;
}
