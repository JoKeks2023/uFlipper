QT = core network
TEMPLATE = app
CONFIG += console c++17 link_pkgconfig
CONFIG -= app_bundle
TARGET = uflipper_nfc_session_tests
PKGCONFIG += libsodium sqlite3
INCLUDEPATH += $$PWD/../../backend
SOURCES += nfc_session_tests.cpp $$PWD/../../backend/workbench/nfcsession.cpp
HEADERS += $$PWD/../../backend/workbench/nfcsession.h
LIBS += -L$$CORE_BUILD -luflipper-core
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 13.0

SOURCES += $$PWD/../../backend/workbench/viccbridge.cpp
HEADERS += $$PWD/../../backend/workbench/viccbridge.h
