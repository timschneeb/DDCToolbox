QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += \
    $$PWD/VdcEditorWindow.h \
    $$PWD/item/CustomFilterListItem.h \
    $$PWD/item/DetailListItem.h \
    $$PWD/model/Biquad.h \
    $$PWD/model/CurveFittingOptions.h \
    $$PWD/model/DeflatedBiquad.h \
    $$PWD/model/FilterModel.h \
    $$PWD/model/FilterType.h \
    $$PWD/model/FilterViewDelegate.h \
    $$PWD/model/command/AddCommand.h \
    $$PWD/model/command/EditCommand.h \
    $$PWD/model/command/InvertCommand.h \
    $$PWD/model/command/RemoveCommand.h \
    $$PWD/model/command/ShiftCommand.h \
    $$PWD/platform/OSXHtmlSizingPatch.h \
    $$PWD/plot/FrequencyPlot.h \
    $$PWD/plot/QCustomPlot.h \
    $$PWD/utils/AutoEqClient.h \
    $$PWD/utils/BitFlags.h \
    $$PWD/utils/CSVParser.h \
    $$PWD/utils/CurveFittingUtils.h \
    $$PWD/utils/CurveFittingWorker.h \
    $$PWD/utils/QInt64Validator.h \
    $$PWD/utils/SoftwareUpdateManager.h \
    $$PWD/utils/VdcImporter.h \
    $$PWD/utils/VdcProjectManager.h \
    $$PWD/widget/AddPointDialog.h \
    $$PWD/widget/AutoEqSelector.h \
    $$PWD/widget/BwCalculator.h \
    $$PWD/widget/CsvExportDialog.h \
    $$PWD/widget/CurveFittingDialog.h \
    $$PWD/widget/CurveFittingWorkerDialog.h \
    $$PWD/widget/CustomFilterDialog.h \
    $$PWD/widget/EapoExportDialog.h \
    $$PWD/widget/Expander.h \
    $$PWD/widget/HtmlPopup.h \
    $$PWD/widget/ModalOverlayMsgProxy.h \
    $$PWD/widget/ProxyStyle.h \
    $$PWD/widget/QMessageOverlay.h \
    $$PWD/widget/QTintableProgressBar.h \
    $$PWD/widget/ShiftFrequencyDialog.h \
    $$PWD/widget/StabilityReport.h

SOURCES += \
    $$PWD/VdcEditorWindow.cpp \
    $$PWD/item/CustomFilterListItem.cpp \
    $$PWD/item/DetailListItem.cpp \
    $$PWD/model/Biquad.cpp \
    $$PWD/model/DeflatedBiquad.cpp \
    $$PWD/model/FilterModel.cpp \
    $$PWD/plot/FrequencyPlot.cpp \
    $$PWD/plot/QCustomPlot.cpp \
    $$PWD/utils/AutoEqClient.cpp \
    $$PWD/utils/CurveFittingWorker.cpp \
    $$PWD/utils/SoftwareUpdateManager.cpp \
    $$PWD/utils/VdcProjectManager.cpp \
    $$PWD/widget/AddPointDialog.cpp \
    $$PWD/widget/AutoEqSelector.cpp \
    $$PWD/widget/BwCalculator.cpp \
    $$PWD/widget/CsvExportDialog.cpp \
    $$PWD/widget/CurveFittingDialog.cpp \
    $$PWD/widget/CurveFittingWorkerDialog.cpp \
    $$PWD/widget/CustomFilterDialog.cpp \
    $$PWD/widget/EapoExportDialog.cpp \
    $$PWD/widget/Expander.cpp \
    $$PWD/widget/HtmlPopup.cpp \
    $$PWD/widget/ModalOverlayMsgProxy.cpp \
    $$PWD/widget/QMessageOverlay.cpp \
    $$PWD/widget/ShiftFrequencyDialog.cpp \
    $$PWD/widget/StabilityReport.cpp

FORMS += \
    $$PWD/VdcEditorWindow.ui \
    $$PWD/item/CustomFilterListItem.ui \
    $$PWD/item/DetailListItem.ui \
    $$PWD/widget/AddPointDialog.ui \
    $$PWD/widget/AutoEqSelector.ui \
    $$PWD/widget/BwCalculator.ui \
    $$PWD/widget/CsvExportDialog.ui \
    $$PWD/widget/CurveFittingDialog.ui \
    $$PWD/widget/CurveFittingWorkerDialog.ui \
    $$PWD/widget/CustomFilterDialog.ui \
    $$PWD/widget/EapoExportDialog.ui \
    $$PWD/widget/ShiftFrequencyDialog.ui \
    $$PWD/widget/StabilityReport.ui \
    $$PWD/widget/TextPopup.ui

INCLUDEPATH += $$PWD

include ($$PWD/3rdparty/WebLoader/WebLoader.pri)
include ($$PWD/3rdparty/QSimpleUpdater/QSimpleUpdater.pri)
include ($$PWD/3rdparty/libMultivariateOpt.pri)
include ($$PWD/3rdparty/makeid/makeid.pri)

RESOURCES += \
    $$PWD/../res/ddceditor_resources.qrc
