#include <QCoreApplication>
#include <QtTest>

#include "model/DeflatedBiquad.h"

class TestBiquadCalculation : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void test_calcBiquad();
    void test_calcBiquad_data();

private:
    Biquad* biquad = nullptr;

};

Q_DECLARE_METATYPE(DeflatedBiquad)
Q_DECLARE_METATYPE(QList<double>)

void TestBiquadCalculation::initTestCase()
{
    qRegisterMetaType<DeflatedBiquad>();
    qRegisterMetaType<QList<double>>();
}

void TestBiquadCalculation::cleanup()
{
    delete biquad;
    biquad = nullptr;
}

void TestBiquadCalculation::test_calcBiquad()
{
    QFETCH(DeflatedBiquad, input);
    QFETCH(QList<double>, output);

    biquad = input.inflate();
    auto coeffs = biquad->ExportCoeffs(44100);

    int i = 0;
    for(const auto& coeff : coeffs){
        QVERIFY(i < output.count());
        QVERIFY(qFuzzyCompare(coeff, output.at(i)));
        i++;
    }
}

void TestBiquadCalculation::test_calcBiquad_data()
{
    QTest::addColumn<DeflatedBiquad>("input");
    QTest::addColumn<QList<double>>("output"); // <- 44100Hz

    QTest::newRow("Peaking filter check")       << DeflatedBiquad(FilterType::PEAKING, 1240, 1.45, -4.56)
                                                << QList<double>({0.9561071060260367,-1.7572839907970152,0.8289625757343658,1.7572839907970152,-0.7850696817604025});
    QTest::newRow("Low pass filter check")      << DeflatedBiquad(FilterType::LOW_PASS, 9000, 0.45, 0)
                                                << QList<double>({0.2977572006541998,0.5955144013083996,0.2977572006541998,0.4736458659823215,-0.6646746685991206});
    QTest::newRow("High pass filter check")     << DeflatedBiquad(FilterType::HIGH_PASS, 90, 2.30, 0)
                                                << QList<double>({0.9887477446743959,-1.9774954893487917,0.9887477446743959,1.9774141997484305,-0.9775767789491530});
    /*QTest::newRow("Low shelf filter check")     << DeflatedBiquad(FilterType::LOW_SHELF, 600, 1.2, -8.6)
                                                << QList<double>({0.9728265357506073,-1.8621456495630835,0.8934761363109759,1.8586297754446239,-0.8698185461800426});
    QTest::newRow("High shelf filter check")    << DeflatedBiquad(FilterType::HIGH_SHELF, 10000, 0.25, 10.25)
                                                << QList<double>({1.8671584960883918,-0.6537004429099003,-0.2917940221189908,-0.1183855241289121,0.1967214930694112});*/
    QTest::newRow("Band pass 1 filter check")   << DeflatedBiquad(FilterType::BAND_PASS1, 170, 0.9, 0)
                                                << QList<double>({0.006857562605174583,0,-0.006857562605174583,1.984178818660435,-0.9847609719885009});
    QTest::newRow("Band pass 2 filter check")   << DeflatedBiquad(FilterType::BAND_PASS2, 500, 3, 0)
                                                << QList<double>({0.08103234195894421,0,-0.08103234195894421,1.833273671900652,-0.8379353160821116});
    QTest::newRow("Notch filter check")         << DeflatedBiquad(FilterType::NOTCH, 400, 1.75, 0)
                                                << QList<double>({0.9645753400208199,-1.926018684417751,0.9645753400208199,1.926018684417751,-0.9291506800416399});
    QTest::newRow("All pass filter check")      << DeflatedBiquad(FilterType::ALL_PASS, 6320, 0.65, 0)
                                                << QList<double>({0.6595865913867148,-1.031033822075308,1,1.031033822075308,-0.6595865913867148});
    QTest::newRow("Unity gain filter check")    << DeflatedBiquad(FilterType::UNITY_GAIN, 10, 0, 5)
                                                << QList<double>({1.778279410038923,0,0,0,0});
    QTest::newRow("1P High pass filter check")  << DeflatedBiquad(FilterType::ONEPOLE_HIGHPASS, 1000, 0, 0)
                                                << QList<double>({0.9655920584451845,-0.9655920584451845,0,0.9311841168903688,0});
    QTest::newRow("1P Low pass filter check")   << DeflatedBiquad(FilterType::ONEPOLE_LOWPASS, 17000, 0, 0)
                                                << QList<double>({0.4090794059162734,0.4090794059162734,0,0.1818411881674533,0});
    QTest::newRow("Custom filter check")        << DeflatedBiquad(FilterType::CUSTOM, CustomFilter(2.473215902450587,-1.234464129264916,-0.4732159024505866,1.485586269581991,-1.234464129264916,0.5144137304180085),
                                                                                      CustomFilter(2.473215902450587,-1.234464129264916,-0.4732159024505866,1.485586269581991,-1.234464129264916,0.5144137304180085))
                                                << QList<double>({0.6006698679682584,-0.4991331844671332,0.2079938633373259,0.4991331844671332,0.1913362686944154});
}

QTEST_MAIN(TestBiquadCalculation)

#include "tst_biquad_operation.moc"
