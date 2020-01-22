#include "conversionengine.h"
#include "vdcimporter.h"

ConversionEngine::ConversionEngine()
{
}

QString ConversionEngine::convertVDCtoProjectFile(QString inputVdc){
    QFile file(inputVdc);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";

    QTextStream in(&file);
    QByteArray ba = in.readAll().toLatin1();
    char* textString = ba.data();

    DirectForm2 **df441, **df48;
    int sosCount = DDCParser(textString, &df441, &df48);
    char *vdcprj = VDC2vdcprj(df48, 48000.0, sosCount);

    free(vdcprj);
    for (int i = 0; i < sosCount; i++)
    {
        free(df441[i]);
        free(df48[i]);
    }
    free(df441);
    free(df48);

    return QString::fromStdString(vdcprj);
}

std::vector<calibrationPoint_t> ConversionEngine::readParametricEQFile(QString path){
    std::vector<calibrationPoint_t> points;
    QString str;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return points;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        str = in.readLine().trimmed();
        if (str != nullptr || str != "")
        {
            if ((str.length() > 0) && !str.startsWith("#"))
            {
                QString strPart2 = str.split(':')[1].trimmed();
                QStringList lineParts = strPart2.split(" ");
                /**
                  [0] "ON"
                  [1] "PK"
                  [2] "Fc"
                  [3] <Freq,INT>
                  [4] "Hz"
                  [5] "Gain"
                  [6] <Gain,FLOAT>
                  [7] "dB"
                  [8] "Q"
                  [9] <Q-Value,FLOAT>
                                      **/
                if ((!lineParts.empty()) && (lineParts.length() == 10))
                {
                    int freq = lineParts[3].toInt();
                    double gain = lineParts[6].toDouble();
                    double q = lineParts[9].toDouble();
                    if(freq < 0)continue;

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
            }
        }
    }
    file.close();
    return points;
}
