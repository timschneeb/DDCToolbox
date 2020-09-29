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

CONFIG += c++17

HEADERS += \
    VdcEditorWindow.h \
    item/CustomFilterListItem.h \
    item/DetailListItem.h \
    model/Biquad.h \
    model/DeflatedBiquad.h \
    model/FilterModel.h \
    model/FilterType.h \
    model/FilterViewDelegate.h \
    model/command/AddCommand.h \
    model/command/EditCommand.h \
    model/command/InvertCommand.h \
    model/command/RemoveCommand.h \
    model/command/ShiftCommand.h \
    plot/FrequencyPlot.h \
    plot/QCustomPlot.h \
    utils/AutoEqClient.h \
    utils/BitFlags.h \
    utils/ProjectManager.h \
    utils/VdcImporter.h \
    widget/AddPointDialog.h \
    widget/AutoEqSelector.h \
    widget/BwCalculator.h \
    widget/CustomFilterDialog.h \
    widget/HtmlPopup.h \
    widget/OverlayMsgProxy.h \
    widget/QMessageOverlay.h \
    widget/ShiftFrequencyDialog.h \
    widget/StabilityReport.h

SOURCES += \
    VdcEditorWindow.cpp \
    item/CustomFilterListItem.cpp \
    item/DetailListItem.cpp \
    main.cpp \
    model/Biquad.cpp \
    model/FilterModel.cpp \
    plot/FrequencyPlot.cpp \
    plot/QCustomPlot.cpp \
    utils/AutoEqClient.cpp \
    utils/ProjectManager.cpp \
    widget/AddPointDialog.cpp \
    widget/AutoEqSelector.cpp \
    widget/BwCalculator.cpp \
    widget/CustomFilterDialog.cpp \
    widget/HtmlPopup.cpp \
    widget/OverlayMsgProxy.cpp \
    widget/QMessageOverlay.cpp \
    widget/ShiftFrequencyDialog.cpp \
    widget/StabilityReport.cpp

FORMS += \
        VdcEditorWindow.ui \
        item/CustomFilterListItem.ui \
        item/DetailListItem.ui \
        widget/AddPointDialog.ui \
        widget/AutoEqSelector.ui \
        widget/BwCalculator.ui \
        widget/CustomFilterDialog.ui \
        widget/ShiftFrequencyDialog.ui \
        widget/StabilityReport.ui \
        widget/TextPopup.ui

include ($$PWD/3rdparty/WebLoader/WebLoader.pri)


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    RC_ICONS = img/icon.ico
}

unix {
   QMAKE_CXXFLAGS += -Wno-unused-variable
}

RESOURCES += \
    ddceditor_resources.qrc
