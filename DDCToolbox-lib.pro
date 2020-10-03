TARGET = DDCToolbox
TEMPLATE = lib
CONFIG += staticlib
DEFINES += DDCTOOLBOX_PLUGIN

CONFIG += c++17

include($$PWD/DDCToolbox.pri)

unix {
    target.path = /usr/lib
}
else: error("Static linking only available on Linux systems")

!isEmpty(target.path): INSTALLS += target
