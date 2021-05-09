INCLUDEPATH += $$PWD/src

QT += network xml

HEADERS += $$PWD/src/HttpMultiPart_p.h \
	   $$PWD/src/NetworkQueue_p.h \
	   $$PWD/src/NetworkRequest.h \
	   $$PWD/src/NetworkRequestPrivate_p.h \
	   $$PWD/src/WebLoader_p.h \
	   $$PWD/src/WebRequest_p.h \
	   $$PWD/src/NetworkRequestLoader.h

SOURCES += $$PWD/src/HttpMultiPart_p.cpp \
	   $$PWD/src/WebLoader_p.cpp \
	   $$PWD/src/WebRequest_p.cpp \
	   $$PWD/src/NetworkQueue_p.cpp \
	   $$PWD/src/NetworkRequest.cpp
