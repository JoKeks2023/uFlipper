#include "cli.h"

#include <QSettings>

int main(int argc, char *argv[])
{
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QCoreApplication::setApplicationName(QStringLiteral("%1-cli").arg(APP_NAME));
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName(QStringLiteral("Joris Conrad"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("jorisconrad.com"));

    Cli a(argc, argv);
    return a.exec();
}
