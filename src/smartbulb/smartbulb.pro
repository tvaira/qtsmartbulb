TARGET = QtSmartBulb

CONFIG += c++11

QT = core bluetooth

QT += core-private

PUBLIC_HEADERS += qsmartbulbglobal.h qsmartbulb.h

PRIVATE_HEADERS += 

SOURCES += qsmartbulb.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

LIBS += -lssl -ldl -lcrypto

load(qt_module)
