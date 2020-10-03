TARGET = DDCToolbox
TEMPLATE = lib
CONFIG += staticlib
DEFINES += DDCTOOLBOX_PLUGIN

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

include($$PWD/DDCToolbox.pri)

unix {
    target.path = /usr/lib
}
else: error("Static linking only available on Linux systems")

!isEmpty(target.path): INSTALLS += target
