# Exercise the production encoder/decoder without building the desktop app.
include(../../plugins/flipperproto0/flipperproto0.pro)
TEMPLATE = app
TARGET = uflipper_storage_read_tests
DESTDIR = $$OUT_PWD
CONFIG -= plugin app_bundle
CONFIG += console
LIBS -= -L$$OUT_PWD/../../3rdparty/ -l3rdparty
INCLUDEPATH += $$PWD/../../backend $$PWD/../../plugins/flipperproto0
SOURCES += storage_read_tests.cpp \
    $$PWD/../../backend/abstractoperation.cpp \
    $$PWD/../../backend/failable.cpp \
    $$PWD/../../backend/flipperzero/rpc/abstractprotobufoperation.cpp \
    $$PWD/../../backend/flipperzero/rpc/storagereadoperation.cpp \
    $$PWD/../../3rdparty/nanopb/pb_common.c \
    $$PWD/../../3rdparty/nanopb/pb_decode.c \
    $$PWD/../../3rdparty/nanopb/pb_encode.c
HEADERS += $$PWD/../../backend/abstractoperation.h \
    $$PWD/../../backend/flipperzero/rpc/abstractprotobufoperation.h \
    $$PWD/../../backend/flipperzero/rpc/storagereadoperation.h
