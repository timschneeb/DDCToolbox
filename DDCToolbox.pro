#-------------------------------------------------
#
# Project created by QtCreator 2019-09-08T00:07:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DDCToolbox
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        Biquad.cpp \
        dialog/StabilityReport.cpp \
        utils/autoeqclient.cpp \
        ddccontext.cpp \
        dialog/addpoint.cpp \
        dialog/autoeqselector.cpp \
        dialog/calc.cpp \
        dialog/customfilterdialog.cpp \
        dialog/overlaymsgproxy.cpp \
        dialog/qmessageoverlay.cpp \
        dialog/shiftfreq.cpp \
        dialog/textpopup.cpp \
        io/conversionengine.cpp \
        io/projectmanager.cpp \
        item/customfilteritem.cpp \
        item/detaillistitem.cpp \
        utils/loghelper.cpp \
        main.cpp \
        mainwindow.cpp \
        plot/frequencyplot.cpp \
        plot/qcustomplot.cpp \
        utils/tableproxy.cpp \
        utils/undocmd.cpp

HEADERS += \
        Biquad.h \
        dialog/StabilityReport.h \
        utils/autoeqclient.h \
        ddccontext.h \
        utils/delegate.h \
        dialog/addpoint.h \
        dialog/autoeqselector.h \
        dialog/calc.h \
        dialog/customfilterdialog.h \
        dialog/overlaymsgproxy.h \
        dialog/qmessageoverlay.h \
        dialog/shiftfreq.h \
        dialog/textpopup.h \
        utils/filtertypes.h \
        io/conversionengine.h \
        io/projectmanager.h \
        item/customfilterfactory.h \
        item/customfilteritem.h \
        item/detaillistitem.h \
        item/detaillistitem_delegate.h \
        utils/loghelper.h \
        mainwindow.h \
        plot/frequencyplot.h \
        plot/qcustomplot.h \
        utils/tableproxy.h \
        utils/undocmd.h \
        utils/vdcimporter.h

FORMS += \
        dialog/StabilityReport.ui \
        dialog/addpoint.ui \
        dialog/autoeqselector.ui \
        dialog/calc.ui \
        dialog/customfilterdialog.ui \
        dialog/shiftfreq.ui \
        dialog/textpopup.ui \
        item/configitem.ui \
        item/customfilteritem.ui \
        mainwindow.ui

!contains(CONFIG, wasm) {
    include ($$PWD/3rdparty/WebLoader/WebLoader.pri)
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    RC_ICONS = img/icon.ico
}

contains(CONFIG, wasm) {
   DEFINES += IS_WASM
}
unix {
   QMAKE_CXXFLAGS += -Wno-unused-variable
}

RESOURCES += \
    resources.qrc
