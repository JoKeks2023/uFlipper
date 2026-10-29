#pragma once
#include <QString>
class QQmlApplicationEngine;
class ApplicationBackend;
void runNfcReview(QQmlApplicationEngine &engine, ApplicationBackend &backend, const QString &directory, bool card, const QString &mode = "ppse");
