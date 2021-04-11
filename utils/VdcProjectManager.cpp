#include "VdcProjectManager.h"

#include "utils/VdcImporter.h"
#include "model/FilterModel.h"

#include <QTextStream>
#include <QFileInfo>
#include <QRegularExpression>

#include <cassert>
#include <cmath>

#define n QString("\n")

VdcProjectManager::VdcProjectManager()
{
}

void VdcProjectManager::projectModified()
{
    m_projectUnsaved = true;
    emit projectMetaChanged();
}

bool VdcProjectManager::saveProject(const QString& fileName){
    assert(m_model);

    if(!writeProject(fileName, m_model->getFilterBank())){
        return false;
    }

    m_currentProject = fileName;
    m_projectUnsaved = false;
    emit projectMetaChanged();
    return true;
}

bool VdcProjectManager::exportProject(QString fileName, const std::list<double>& p1, const std::list<double>& p2){
    if (fileName.isEmpty())
        return false;

    if(QFileInfo(fileName).suffix() != "vdc")
        fileName.append(".vdc");

    QFile caFile(fileName);
    caFile.open(QIODevice::WriteOnly | QIODevice::Text);

    if(!caFile.isOpen())
        return false;

    QTextStream outStream(&caFile);
    QString buffer;

#define WRITE_VDC(input,sr) \
    buffer += "SR_"#sr":"; \
    for(const auto& coeff : input) \
        buffer += QString::number(coeff, 'f', 16) + ","; \
    buffer.chop(1); \
    buffer += n;

    WRITE_VDC(p1,44100);
    WRITE_VDC(p2,48000);

#undef WRITE_VDC

    outStream << buffer;
    caFile.close();
    return true;
}

bool VdcProjectManager::loadProject(const QString &fileName)
{
    closeProject();

    auto biquads = readProject(fileName);
    m_model->clear();

    m_model->appendAllDeflated(biquads);

    m_currentProject = fileName;
    m_projectUnsaved = false;
    emit projectMetaChanged();

    return !biquads.empty();
}

void VdcProjectManager::closeProject()
{
    m_model->clear();
    m_currentProject.clear();
    m_projectUnsaved = false;

    emit projectClosed();
    emit projectMetaChanged();
}

bool VdcProjectManager::writeProject(const QString& fileName, QVector<Biquad *> bank)
{
    QVector<DeflatedBiquad> biquads;
    for(const auto& b : bank)
        biquads.append(b);
    return writeProject(fileName, biquads);
}

bool VdcProjectManager::writeProject(const QString& fileName, QVector<DeflatedBiquad> bank)
{
    QString name = fileName;
    if(QFileInfo(fileName).suffix() != "vdcprj")
        name.append(".vdcprj");

    QFile caFile(name);
    caFile.open(QIODevice::WriteOnly | QIODevice::Text);

    if(!caFile.isOpen())
        return false;

    /* Check if current calibration points
       are compatible with vipers toolbox v2.0 */
    bool legacy = true;
    for(const auto& biquad : bank)
        if(biquad.type != FilterType::PEAKING)
            legacy = false;

    QTextStream outStream(&caFile);
    if(legacy){
        outStream << "# DDCToolbox Project File, v1.0.0.0 (Legacy mode)" + n;
        outStream << n;
        for (int i = 0; i < bank.size(); i++)
        {
            DeflatedBiquad biquad = bank.at(i);
            outStream << "# Calibration Point " + QString::number(i + 1) + n;
            outStream << QString::number(biquad.freq) + "," +
                         QString::number(biquad.bwOrSlope) + "," +
                         QString::number(biquad.gain) + n;
        }
    }
    else
    {
        outStream << "# DDCToolbox Project File, v4.0.0.0" + n;
        outStream << n;
        for (int i = 0; i < bank.size(); i++)
        {
            DeflatedBiquad biquad = bank.at(i);
            outStream << "# Calibration Point " + QString::number(i + 1) + n;
            if(biquad.type == FilterType::CUSTOM){
#define WRITE_CUSTOM(sr,name) QString::number(biquad.c##sr.name,'f',16)
                outStream << "1,0,0," + (QString)biquad.type + ";";
                outStream << WRITE_CUSTOM(441,a0) + "," + WRITE_CUSTOM(441,a1) + "," + WRITE_CUSTOM(441,a2) + ",";
                outStream << WRITE_CUSTOM(441,b0) + "," + WRITE_CUSTOM(441,b1) + "," + WRITE_CUSTOM(441,b2) + ",";
                outStream << WRITE_CUSTOM(48,a0)  + "," + WRITE_CUSTOM(48,a1)  + "," + WRITE_CUSTOM(48,a2) + ",";
                outStream << WRITE_CUSTOM(48,b0)  + "," + WRITE_CUSTOM(48,b1)  + "," + WRITE_CUSTOM(48,b2) + n;
#undef WRITE_CUSTOM
            }
            else
                outStream << QString::number(biquad.freq) + "," +
                             QString::number(biquad.bwOrSlope) + "," +
                             QString::number(biquad.gain) + "," +
                             (QString)biquad.type + n;
        }
    }
    outStream << n;
    outStream << "#File End" + n;
    caFile.close();
    return true;
}

QVector<DeflatedBiquad> VdcProjectManager::readProject(const QString& fileName)
{
    QVector<DeflatedBiquad> buffer;
    if (!fileName.isEmpty()){
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return buffer;

        QTextStream in(&file);
        while (!in.atEnd()){
            auto point = parseProjectLine(in.readLine());
            if(point.type != FilterType::INVALID)
                buffer.push_back(point);
        }
        file.close();
    }
    return buffer;
}

DeflatedBiquad VdcProjectManager::parseProjectLine(QString str){

    str = str.trimmed();
    if (str.isEmpty() || str.startsWith("#"))
        return DeflatedBiquad();

    bool isCustomLine = false;
    QStringList csvData;

    /* Check for custom filter data and fill csvData */
    if(str.contains(";") &&
            str.split(";")[0].trimmed().endsWith("Custom", Qt::CaseInsensitive)){
        csvData = str.split(";")[0].split(',');
        isCustomLine = true;
    }
    else{
        csvData = str.split(',');
    }

    if(csvData.empty())
        return DeflatedBiquad();


    if(!isCustomLine){
        DeflatedBiquad biquad;

        if(csvData.length() < 3)
            return DeflatedBiquad();

        bool freqOk, bwOk, gainOk;
        biquad.freq = csvData[0].toInt(&freqOk);
        biquad.bwOrSlope = csvData[1].toDouble(&bwOk);
        biquad.gain = csvData[2].toDouble(&gainOk);

        if(!freqOk || !bwOk || !gainOk)
            return DeflatedBiquad();

        if(biquad.freq < FREQ_MIN || biquad.freq > FREQ_MAX)
            return DeflatedBiquad();
        if(biquad.bwOrSlope < BW_MIN || biquad.bwOrSlope > BW_MAX)
            return DeflatedBiquad();
        if(biquad.gain < GAIN_MIN || biquad.gain > GAIN_MAX)
            return DeflatedBiquad();

        /* Legacy project data (v1) */
        if(csvData.length() == 3)
            biquad.type = FilterType::PEAKING;
        /* Project data (v4) */
        else if(csvData.length() == 4)
            biquad.type = FilterType(csvData[3].trimmed());

        return biquad;
    }
    else {
        CustomFilter c441 = CustomFilter();
        CustomFilter c48 = CustomFilter();
        int counter = 0;
        for(const auto& coeff : str.split(";")[1].split(",")){
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

        if(counter != 12){
            qWarning() << "Invalid custom coefficient count";
        }

        return DeflatedBiquad(FilterType::CUSTOM, c441, c48);
    }
}

bool VdcProjectManager::loadVdc(const QString &fileName)
{
    closeProject();

    auto biquads = readVdc(fileName);
    m_model->clear();
    m_model->appendAllDeflated(biquads);

    m_projectUnsaved = false;
    emit projectMetaChanged();

    return !biquads.empty();
}

QVector<DeflatedBiquad> VdcProjectManager::readVdc(const QString& inputVdc){
    QVector<DeflatedBiquad> biquads;

    QFile file(inputVdc);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return biquads;

    QTextStream in(&file);
    QByteArray ba = in.readAll().toLatin1();
    char* textString = ba.data();

    DirectForm2 **df441, **df48;
    int sosCount = DDCParser(textString, &df441, &df48);
    char *vdcprj = VDC2vdcprj(df48, 48000.0, sosCount);

    QString proj = QString::fromStdString(vdcprj);

    free(vdcprj);
    for (int i = 0; i < sosCount; i++)
    {
        free(df441[i]);
        free(df48[i]);
    }
    free(df441);
    free(df48);

    QTextStream projIn(&proj);
    while (!projIn.atEnd()){
        auto point = parseProjectLine(projIn.readLine());
        if(point.type != FilterType::INVALID)
            biquads.push_back(point);
    }

    return biquads;
}

bool VdcProjectManager::loadParametricEq(const QString &fileName)
{
    closeProject();

    auto biquads = readParametricEq(fileName);
    m_model->clear();
    m_model->appendAllDeflated(biquads);

    m_projectUnsaved = false;
    emit projectMetaChanged();

    return !biquads.empty();
}

bool VdcProjectManager::loadParametricEqString(QString string)
{
    closeProject();
    m_model->clear();

    QVector<DeflatedBiquad> biquads;

    QTextStream in(&string);
    while (!in.atEnd())
    {
        auto biquad = parseParametricEqLine(in.readLine().trimmed());
        if(biquad.type != FilterType::INVALID)
            biquads.append(biquad);
    }

    m_model->appendAllDeflated(biquads);

    m_projectUnsaved = false;
    emit projectMetaChanged();

    return m_model->rowCount() > 0;
}

QVector<DeflatedBiquad> VdcProjectManager::readParametricEq(const QString& path){
    QVector<DeflatedBiquad> biquads;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return biquads;

    QTextStream in(file.readAll());
    while (!in.atEnd())
    {
        auto biquad = parseParametricEqLine(in.readLine().trimmed());
        if(biquad.type != FilterType::INVALID)
            biquads += biquad;
    }

    file.close();
    return biquads;
}

DeflatedBiquad VdcProjectManager::parseParametricEqLine(QString str){
    if (str.isEmpty() || str.startsWith("#"))
        return DeflatedBiquad();

    QRegularExpression re
            (R"(Filter\s\d+[\s\S][^PK]*PK\s+Fc\s+(?<hz>\d+)\s*Hz\s*Gain\s*(?<gain>-?\d*.?\d+)\s*dB\s*Q\s*(?<q>-?\d*.?\d+))");
    QRegularExpressionMatch match = re.match(str);
    if(!match.hasMatch())
        return DeflatedBiquad();

    bool freqOk, gainOk, qOk;
    int freq = match.captured("hz").toInt(&freqOk);
    double gain = match.captured("gain").toDouble(&gainOk);
    double q = match.captured("q").toDouble(&qOk);

    if(!freqOk || !gainOk || !qOk)
        return DeflatedBiquad();

    /* Convert Q to BW */
    double QQ1st = ((2*q*q)+1)/(2*q*q);
    double QQ2nd = pow(2*QQ1st,2)/4;
    double bw = round(1000000*log(QQ1st+sqrt(QQ2nd-1))/log(2))/1000000;

    if(freq < FREQ_MIN || freq > FREQ_MAX)
        return DeflatedBiquad();
    if(bw < BW_MIN || bw > BW_MAX)
        return DeflatedBiquad();
    if(gain < GAIN_MIN || gain > GAIN_MAX)
        return DeflatedBiquad();

    return DeflatedBiquad(FilterType::PEAKING, freq, bw, gain);
}
