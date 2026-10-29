#pragma once
#include <QString>
class QQmlApplicationEngine;
class ApplicationBackend;
void runBackupReview(QQmlApplicationEngine &, ApplicationBackend &, const QString &directory, bool cancel);
