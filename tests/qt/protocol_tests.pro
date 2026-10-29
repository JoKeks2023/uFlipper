QT = core serialport network
include(../../qflipper_common.pri)
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = uflipper_protocol_tests
INCLUDEPATH += $$PWD/../../backend $$PWD/../../plugins/protobufinterface
LIBS += -L$$BACKEND_BUILD -lbackend -L$$BACKEND_BUILD/../dfu -ldfu
SOURCES += protocol_tests.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libsodium sqlite3
LIBS += -L$$BACKEND_BUILD/.. -luflipper-core
