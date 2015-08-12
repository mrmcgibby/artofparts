TEMPLATE = app

QT += qml quick widgets multimedia

SOURCES += main.cpp \
    balance.cpp \
    gason.cpp \
    balancemodel.cpp \
    box.cpp \
    optimizer.cpp

RESOURCES += qml.qrc

CONFIG += c++11

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    gason.h \
    balancemodel.h \
    box.h \
    optimizer.h

DISTFILES +=
