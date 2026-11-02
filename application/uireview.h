#pragma once
#include <QString>
class QQmlApplicationEngine;
class ApplicationBackend;
void runUiReview(QQmlApplicationEngine &engine, ApplicationBackend &backend, const QString &directory);
