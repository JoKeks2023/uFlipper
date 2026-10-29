QT += quick serialport widgets quickcontrols2 svg testlib

include(../qflipper_common.pri)

TARGET = $$NAME
DESTDIR = $$OUT_PWD/..

CONFIG += c++17

SOURCES += backupreview.cpp nfcreview.cpp uireview.cpp \
        application.cpp \
        applicationupdater.cpp \
        applicationupdateregistry.cpp \
        main.cpp \
        qtsingleapplication/qtlocalpeer.cpp \
        qtsingleapplication/qtlockedfile.cpp \
        qtsingleapplication/qtlockedfile_unix.cpp \
        qtsingleapplication/qtlockedfile_win.cpp \
        qtsingleapplication/qtsingleapplication.cpp \
        qtsingleapplication/qtsinglecoreapplication.cpp \
        screencanvas.cpp \
        systemfiledialog.cpp

RESOURCES += qml.qrc

TRANSLATIONS += \
    translations/en_US.ts

CONFIG += lrelease
CONFIG += embed_translations

# Generated translations must exist before any parallel RCC invocation.
load(lrelease)
rcc.depends += $$QM_FILES

QML_IMPORT_PATH += $$PWD/imports

unix:!macx {
    QTPLUGIN += qxdgdesktopportal
    QTPLUGIN.platforms += qxcb qwayland-egl qwayland-generic
}

win32:!win32-g++ {
    PRE_TARGETDEPS += \
        $$OUT_PWD/../backend/backend.lib \
        $$OUT_PWD/../dfu/dfu.lib

} else:unix|win32-g++ {
    PRE_TARGETDEPS += \
        $$OUT_PWD/../backend/libbackend.a \
        $$OUT_PWD/../dfu/libdfu.a

    contains(CONFIG, static): PRE_TARGETDEPS += \
        $$OUT_PWD/../plugins/libflipperproto0.a \
        $$OUT_PWD/../3rdparty/lib3rdparty.a
}

unix|win32 {
    LIBS += \
        -L$$OUT_PWD/../backend/ -lbackend \
        -L$$OUT_PWD/../dfu/ -ldfu

    contains(CONFIG, static): LIBS += \
        -L$$OUT_PWD/../plugins/ -lflipperproto0 \
        -L$$OUT_PWD/../3rdparty/ -l3rdparty
}

win32 {
    equals(HAS_VERSION, 0) {
        RC_SUFFIX = -rc

        contains(GIT_VERSION, .*$${RC_SUFFIX}.*) {
            # Remove -rc suffix as it isn't allowed in Windows manifest
            TOKENS = $$split(GIT_VERSION, -)
            VERSION = $$first(TOKENS)
        } else {
            VERSION = $$GIT_VERSION
        }

    } else: VERSION = 0.0.0
}

macx {
    ICON = assets/icons/qFlipper.icns
    QMAKE_INFO_PLIST = $$PWD/../installer-assets/macos/Info.plist
} else:win32: RC_ICONS = assets/icons/qFlipper.ico

INCLUDEPATH += \
    $$PWD/../dfu \
    $$PWD/../backend

DEPENDPATH += \
    $$PWD/../dfu \
    $$PWD/../backend

HEADERS += backupreview.h nfcreview.h \
    application.h \
    applicationupdater.h \
    applicationupdateregistry.h \
    qtsingleapplication/qtlocalpeer.h \
    qtsingleapplication/qtlockedfile.h \
    qtsingleapplication/qtsingleapplication.h \
    qtsingleapplication/qtsinglecoreapplication.h \
    screencanvas.h \
    systemfiledialog.h

DISTFILES +=

unix:!macx {
    target.path = $$PREFIX/bin

    desktopfiles.files = $$PWD/../installer-assets/appimage/uFlipper.desktop
    desktopfiles.path = $$PREFIX/share/applications

    iconfiles.files = $$PWD/assets/icons/qFlipper.png
    iconfiles.path = $$PREFIX/share/icons/hicolor/512x512/apps

    udevfiles.files = $$PWD/../installer-assets/udev/42-flipperzero.rules
    udevfiles.path = $$PREFIX/lib/udev/rules.d

    INSTALLS += target desktopfiles iconfiles udevfiles

} else:win32 {
    target.path = $$DESTDIR/$$NAME
    INSTALLS += target
}

CONFIG += link_pkgconfig
PKGCONFIG += libsodium sqlite3
LIBS += -L$$OUT_PWD/../ -luflipper-core
