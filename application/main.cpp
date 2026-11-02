#include "application.h"
#include "mcp/stdiorelay.h"
#include "mcp/localgateway.h"
#include <cstring>

#include <QSettings>

int main(int argc, char *argv[])
{
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName(QStringLiteral("Joris Conrad"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("jorisconrad.com"));

    if (argc == 2 && std::strcmp(argv[1], "--mcp") == 0) {
        QCoreApplication relay(argc, argv);
        return Mcp::runStdioRelay(Mcp::localDirectory());
    }

#if QT_VERSION < 0x060000
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);

    Application app(argc, argv);
    return app.exec();
}
