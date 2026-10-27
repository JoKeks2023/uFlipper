#include "uireview.h"
#include "workbench/workbenchdata.h"
#include "application.h"
#include "workbench/devicetelemetry.h"

#include <QDebug>
#include <QTimer>
#include <QLocale>
#include <QSysInfo>
#include <QDateTime>
#include <QTranslator>
#include <QQmlContext>
#include <QQuickWindow>
#include <QFontDatabase>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QtQuickControls2/QQuickStyle>

#include "updateregistry.h"
#include "deviceregistry.h"
#include "screencanvas.h"
#include "preferences.h"
#include "backenderror.h"
#include "inputevent.h"
#include "logger.h"

Q_LOGGING_CATEGORY(LOG_APP, "APP")

Application::Application(int &argc, char **argv):
    QtSingleApplication(QStringLiteral(APP_NAME), argc, argv),
    m_updateRegistry(QString()),
    m_isDeveloperMode(QGuiApplication::queryKeyboardModifiers() & Qt::KeyboardModifier::AltModifier),
    m_updateStatus(UpdateStatus::NoUpdates)
{
    if(isRunning()) {
        sendMessage(QStringLiteral("IT'S ME."));
        std::exit(0);
        return;
    }

    initCommandOptions();
    initConnections();
    initLogger();
    initQmlTypes();
    initTranslations();
    initImports();
    initStyles();
    initFonts();
    initGUI();
    if(!m_uiReviewDirectory.isEmpty()) QTimer::singleShot(7000, this, [this] { runUiReview(m_engine, m_uiReviewDirectory); });

    qCInfo(LOG_APP).noquote() << APP_NAME << "version" << APP_VERSION << "commit"
                              << APP_COMMIT << QDateTime::fromSecsSinceEpoch(APP_TIMESTAMP).toString(Qt::ISODate);

    qCInfo(LOG_APP).noquote() << "OS info:" << QSysInfo::prettyProductName() << QSysInfo::productVersion() << QSysInfo::kernelVersion() << "Qt" << qVersion();

    if(m_isDeveloperMode) {
        qCCritical(LOG_APP) << "Developer mode is enabled! Please be careful.";
    }
}

Application::~Application()
{
    qCInfo(LOG_APP).noquote() << APP_NAME << "exited";
}

ApplicationUpdater *Application::updater()
{
    return &m_updater;
}

const QString Application::commitNumber()
{
    return APP_COMMIT;
}

bool Application::isDeveloperMode() const
{
    return m_isDeveloperMode;
}

Application::UpdateStatus Application::updateStatus() const
{
    return m_updateStatus;
}

void Application::selfUpdate()
{
    qCInfo(LOG_APP) << "Application updates are disabled until a uFlipper update channel is configured.";
}

void Application::checkForUpdates()
{
    setUpdateStatus(UpdateStatus::NoUpdates);
    qCInfo(LOG_APP) << "No uFlipper application update channel is configured.";
}

void Application::onMessageReceived()
{
    qCDebug(LOG_APP) << "Another instance was prevented from running";
}

void Application::onLatestVersionChanged()
{
    if(m_updateRegistry.state() == ApplicationUpdateRegistry::State::Ready && m_updater.canUpdate(m_updateRegistry.latestVersion())) {
        setUpdateStatus(UpdateStatus::CanUpdate);
    } else {
        setUpdateStatus(UpdateStatus::NoUpdates);
    }
}

void Application::onCurrentDeviceChanged()
{
    if(m_fileDialog.isOpen()) {
        m_fileDialog.close();
    }
}

void Application::initConnections()
{
    connect(this, &QtSingleApplication::messageReceived, this, &Application::onMessageReceived);
    connect(&m_updateRegistry, &Flipper::UpdateRegistry::latestVersionChanged, this, &Application::onLatestVersionChanged);
    connect(&m_backend, &ApplicationBackend::currentDeviceChanged, this, &Application::onCurrentDeviceChanged);
}

void Application::initCommandOptions()
{
    QCommandLineParser parser;

    const auto developerModeOption = QCommandLineOption({QStringLiteral("x"), QStringLiteral("developer-mode")}, QStringLiteral("Enable developer mode."));
    const auto usbLogLevelOption = QCommandLineOption({QStringLiteral("u"), QStringLiteral("usb-log-level")}, QStringLiteral("Set USB backend log level, 0 - none"), QStringLiteral("0"));

    parser.addOption(developerModeOption);
    parser.addOption(usbLogLevelOption);
    const QCommandLineOption smokeTest(QStringLiteral("smoke-test"), QStringLiteral("Exit after a native startup test."));
    parser.addOption(smokeTest);
    const QCommandLineOption uiReview(QStringLiteral("review-ui"), QStringLiteral("Exercise native UI navigation and save screenshots"), QStringLiteral("directory"));
    parser.addOption(uiReview);
    parser.addVersionOption();
    parser.addHelpOption();

    parser.process(*this);

    if(parser.isSet(smokeTest)) QTimer::singleShot(3000, this, [] { QCoreApplication::quit(); });
    m_uiReviewDirectory = parser.value(uiReview);
    if(parser.isSet(smokeTest) && !m_uiReviewDirectory.isEmpty()) parser.showHelp(2);
    m_isDeveloperMode |= parser.isSet(developerModeOption);

    if(parser.isSet(usbLogLevelOption)) {
        bool canConvert;
        const auto value = parser.value(usbLogLevelOption).toInt(&canConvert);
        if(!canConvert) {
            qCDebug(LOG_APP) << "USB log level has to be a non-negative number";
        } else {
            m_backend.deviceRegistry()->setBackendLogLevel(value);
        }
    }
}

void Application::initLogger()
{
    qInstallMessageHandler(Logger::messageOutput);
}

void Application::initStyles()
{
#ifdef Q_OS_LINUX
    setWindowIcon(QIcon(":/assets/icons/qFlipper.ico"));
#endif

    QQuickWindow::setDefaultAlphaBuffer(true);
    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);

#if QT_VERSION < 0x060000
    QQuickStyle::setStyle(":/styles/DefaultAmber");
#else
    QQuickStyle::setStyle("DefaultAmber");
#endif
}

void Application::initTranslations()
{
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();

    for (const QString &locale : uiLanguages) {
        if (translator.load(":/i18n/" + QLocale(locale).name())) {
            installTranslator(&translator);
            break;
        }
    }
}

void Application::initQmlTypes()
{
    qmlRegisterType<ScreenCanvas>("QFlipper", 1, 0, "ScreenCanvas");

    qmlRegisterUncreatableType<BackendError>("QFlipper", 1, 0, "BackendError", QStringLiteral("This class is only a enum container"));
    qmlRegisterUncreatableType<InputEvent>("QFlipper", 1, 0, "InputEvent", QStringLiteral("This class is only a enum container"));
    qmlRegisterUncreatableType<WorkbenchData>("QFlipper", 1, 0, "WorkbenchData", QStringLiteral("Workbench data is owned by the backend"));
    qmlRegisterUncreatableType<DeviceTelemetry>("QFlipper", 1, 0, "DeviceTelemetry", QStringLiteral("Device telemetry is owned by the backend"));
    qmlRegisterUncreatableType<ApplicationBackend>("QFlipper", 1, 0, "ApplicationBackend", QStringLiteral("This class is meant to be created from c++"));
    qmlRegisterUncreatableType<ApplicationUpdater>("QFlipper", 1, 0, "ApplicationUpdater", QStringLiteral("This class is meant to be created from c++"));

    qmlRegisterSingletonInstance("QFlipper", 1, 0, "Logger", globalLogger);
    qmlRegisterSingletonInstance("QFlipper", 1, 0, "Preferences", globalPrefs);
    qmlRegisterSingletonInstance("QFlipper", 1, 0, "Backend", &m_backend);
    qmlRegisterSingletonInstance("QFlipper", 1, 0, "App", this);
    qmlRegisterSingletonInstance("QFlipper", 1, 0, "SystemFileDialog", &m_fileDialog);
}

void Application::initImports()
{
    m_engine.addImportPath(":/imports");
    m_engine.addImportPath(":/styles");
}

void Application::initFonts()
{
    QFontDatabase::addApplicationFont(":/assets/fonts/haxrcorp-4089.ttf");
    QFontDatabase::addApplicationFont(":/assets/fonts/Born2bSportyV2.ttf");
    QFontDatabase::addApplicationFont(":/assets/fonts/ProggySquare.ttf");
    QFontDatabase::addApplicationFont(":/assets/fonts/ShareTech-Regular.ttf");
    QFontDatabase::addApplicationFont(":/assets/fonts/ShareTechMono-Regular.ttf");
}

void Application::initGUI()
{
    const QUrl url(QStringLiteral("qrc:/main.qml"));

    const auto onObjectCreated = [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            std::exit(-1);
        }
    };

    connect(&m_engine, &QQmlApplicationEngine::objectCreated, this, onObjectCreated, Qt::QueuedConnection);
    m_engine.rootContext()->setContextProperty("qVersionMajor", QT_VERSION_MAJOR);
    m_engine.rootContext()->setContextProperty("qVersionMinor", QT_VERSION_MINOR);
    m_engine.rootContext()->setContextProperty("qVersionPatch", QT_VERSION_PATCH);
    m_engine.load(url);
}

void Application::setUpdateStatus(UpdateStatus newUpdateStatus)
{
    if(newUpdateStatus == m_updateStatus) {
        return;
    }

    m_updateStatus = newUpdateStatus;
    emit updateStatusChanged();
}
