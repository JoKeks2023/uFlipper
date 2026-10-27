QT = core serialport network
include(../../qflipper_common.pri)
CONFIG += console c++17 link_pkgconfig
CONFIG -= app_bundle
TARGET = uFlipper-probe
DESTDIR = $$OUT_PWD/../..
INCLUDEPATH += $$PWD/../../backend $$PWD/../core $$PWD/../../plugins/protobufinterface
LIBS += -L$$OUT_PWD/../../backend -lbackend -L$$OUT_PWD/../../dfu -ldfu -L$$OUT_PWD/../.. -luflipper-core
PKGCONFIG += libsodium sqlite3
SOURCES += main.cpp
macx: target.path = $$DESTDIR/uFlipper.app/Contents/MacOS
unix:!macx: target.path = $$PREFIX/bin
INSTALLS += target
