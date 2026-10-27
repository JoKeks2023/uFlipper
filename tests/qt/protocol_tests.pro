QT = core serialport network
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = uflipper_protocol_tests
INCLUDEPATH += $$PWD/../../backend $$PWD/../../plugins/protobufinterface
LIBS += -L$$BACKEND_BUILD -lbackend
SOURCES += protocol_tests.cpp
