# Exercise the production encoder/decoder without building the desktop app.
QT = core
TEMPLATE = app
TARGET = uflipper_storage_read_tests
CONFIG += console c++17
CONFIG -= app_bundle
DEFINES += PB_ENABLE_MALLOC
INCLUDEPATH += $$PWD/../../backend $$PWD/../../plugins/flipperproto0 \
    $$PWD/../../plugins/protobufinterface $$PWD/../../3rdparty/nanopb
SOURCES += $$files($$PWD/../../plugins/flipperproto0/*.cpp) \
    $$files($$PWD/../../plugins/flipperproto0/messages/*.c)
HEADERS += $$files($$PWD/../../plugins/flipperproto0/*.h)
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 13.0
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
