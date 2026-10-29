#include "uireview.h"
#include "preferences.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTest>
#include <QTimer>
#include <functional>
#include <stdexcept>
#include <vector>
namespace {
QQuickItem *findItem(QQuickItem *parent, const QString &name) {
    if (parent->objectName() == name)
        return parent;
    for (auto *child : parent->childItems())
        if (auto *found = findItem(child, name))
            return found;
    return nullptr;
}
class Review : public QObject {
  public:
    Review(QQmlApplicationEngine &engine, QString directory)
        : QObject(&engine), directory_(std::move(directory)) {
        window_ = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
        if (!window_ || !QDir().mkpath(directory_))
            throw std::runtime_error("UI review cannot initialize");
        originalEasyMode_ = globalPrefs->easyMode();
        connect(&engine, &QQmlApplicationEngine::warnings, this, [this](const QList<QQmlError> &warnings) {
            for (const auto &warning : warnings) {
                const auto text = warning.toString();
                if (text.contains("TypeError") || text.contains("ReferenceError") ||
                    text.contains("Binding loop") || text.contains("Cannot assign") ||
                    text.contains("Unable to assign"))
                    errors_.append(text);
            }
        });
        steps_ = {{"nav-dashboard", "dashboard", "01_dashboard"},
                  {"nav-infrared", "infrared", "02_infrared"},
                  {"tool-easy-mode", "infrared", ""},
                  {"tool-easy-mode", "infrared", ""},
                  {"tool-tab-1", "infrared", "11_ir_controls"},
                  {"tool-tab-2", "infrared", "12_ir_library"},
                  {"tool-tab-0", "infrared", ""},
                  {"nav-nfc", "nfc", "14_nfc"},
                  {"nav-settings", "settings", "03_settings"},
                  {"easy-mode-preference", "settings", ""},
                  {"easy-mode-preference", "settings", ""},
                  {"settings-tab-1", "settings", "04_storage"},
                  {"settings-tab-2", "settings", "05_cloud"},
                  {"settings-tab-3", "settings", "06_integrations"},
                  {"settings-tab-4", "settings", "07_credits"},
                  {"nav-backups", "backups", "15_backups"},
                  {"nav-library", "library", "08_files"},
                  {"nav-screen", "screen", "09_screen"},
                  {"nav-dashboard", "dashboard", "10_compact"}};
        QTimer::singleShot(0, this, [this] { next(); });
    }
    ~Review() override {
        globalPrefs->setEasyMode(originalEasyMode_);
    }

  private:
    struct Step {
        const char *button;
        const char *page;
        const char *image;
    };
    void click(const QString &name) {
        auto *button = findItem(window_->contentItem(), name);
        if (!button || !button->isVisible() || !button->isEnabled())
            throw std::runtime_error(("UI button unavailable: " + name).toStdString());
        const auto point = button->mapToScene(QPointF(button->width() / 2, button->height() / 2));
        QTest::mouseClick(window_, Qt::LeftButton, Qt::NoModifier, point.toPoint(), 30);
        if (name == "easy-mode-preference") {
            QTest::qWait(200);
            if (dropdownChanges_ == 0 &&
                !window_->grabWindow().save(directory_ + "/13_settings_dropdown.png"))
                throw std::runtime_error("Cannot capture settings dropdown");
            QTest::keyClick(window_, globalPrefs->easyMode() ? Qt::Key_End : Qt::Key_Home);
            QTest::keyClick(window_, Qt::Key_Return);
            ++dropdownChanges_;
        }
    }
    void next() {
        try {
            if (index_ == steps_.size()) {
                complete();
                return;
            }
            const auto &step = steps_[index_];
            if (index_ + 1 == steps_.size()) {
                window_->setWidth(1060);
                window_->setHeight(680);
            }
            click(QString::fromLatin1(step.button));
            QTimer::singleShot(450, this, [this] { check(); });
        } catch (const std::exception &error) {
            fail(QString::fromUtf8(error.what()));
        }
    }
    void check() {
        try {
            const auto &step = steps_[index_];
            auto *shell = findItem(window_->contentItem(), "workbench");
            if (!shell || shell->property("currentPage").toString() != QString::fromLatin1(step.page))
                throw std::runtime_error(
                    (QString("Click %1: expected %2, actual %3")
                         .arg(step.button, step.page,
                              shell ? shell->property("currentPage").toString() : "missing shell"))
                        .toStdString());
            if (index_ == 2 && globalPrefs->easyMode() == originalEasyMode_)
                throw std::runtime_error("Easy Mode click did not update preference");
            if (index_ == 3 && globalPrefs->easyMode() != originalEasyMode_)
                throw std::runtime_error("Easy Mode preference did not restore");
            const auto buttonName = QString::fromLatin1(step.button);
            if (buttonName == "easy-mode-preference") {
                const bool expected = dropdownChanges_ == 1 ? !originalEasyMode_ : originalEasyMode_;
                if (globalPrefs->easyMode() != expected)
                    throw std::runtime_error("Settings dropdown did not persist expected Easy Mode value");
            }
            if (buttonName.startsWith("tool-tab-")) {
                auto *tool = findItem(window_->contentItem(), "toolPage");
                if (!tool || tool->property("currentTab").toInt() != buttonName.mid(9).toInt())
                    throw std::runtime_error("Tool tab click did not reach expected tab");
            }
            if (buttonName.startsWith("settings-tab-")) {
                auto *settings = findItem(window_->contentItem(), "settingsPage");
                if (!settings || settings->property("tab").toInt() != buttonName.mid(13).toInt())
                    throw std::runtime_error("Settings tab click did not reach expected tab");
            }
            if (*step.image) {
                const auto image = window_->grabWindow();
                if (image.isNull() || !image.save(directory_ + "/" + step.image + ".png"))
                    throw std::runtime_error("Native screenshot failed");
            }
            ++index_;
            next();
        } catch (const std::exception &error) {
            fail(QString::fromUtf8(error.what()));
        }
    }
    void complete() {
        for (const auto &text : parent()->property("uiReviewErrors").toStringList()) {
            if (text.contains("TypeError") || text.contains("ReferenceError") ||
                text.contains("Binding loop") || text.contains("Cannot assign") ||
                text.contains("Unable to assign"))
                errors_.append(text);
        }
        if (!errors_.isEmpty()) {
            fail(errors_.join("\n"));
            return;
        }
        QFile result(directory_ + "/result.json");
        if (!result.open(QIODevice::WriteOnly)) {
            fail("Cannot save UI result");
            return;
        }
        if (dropdownChanges_ != 2) {
            fail("Settings dropdown roundtrip was not exercised");
            return;
        }
        int screenshots = QFile::exists(directory_ + "/13_settings_dropdown.png") ? 1 : 0;
        for (const auto &step : steps_)
            if (*step.image)
                ++screenshots;
        result.write(QJsonDocument(QJsonObject{{"navigationClicks", int(steps_.size())},
                                               {"easyModeRoundtrip", true},
                                               {"settingsDropdownRoundtrip", true},
                                               {"screenshots", screenshots},
                                               {"compactWidth", 1060},
                                               {"compactHeight", 680},
                                               {"qmlErrors", 0},
                                               {"success", true}})
                         .toJson());
        QCoreApplication::exit(0);
    }
    void fail(const QString &message) {
        if (window_)
            window_->grabWindow().save(directory_ + "/failure.png");
        globalPrefs->setEasyMode(originalEasyMode_);
        QFile result(directory_ + "/failure.txt");
        if (result.open(QIODevice::WriteOnly))
            result.write(message.toUtf8());
        qCritical().noquote() << "UI review failed:" << message;
        QCoreApplication::exit(2);
    }
    QString directory_;
    QQuickWindow *window_;
    std::vector<Step> steps_;
    size_t index_ = 0;
    int dropdownChanges_ = 0;
    bool originalEasyMode_ = true;
    QStringList errors_;
};
} // namespace
void runUiReview(QQmlApplicationEngine &engine, const QString &directory) {
    try {
        new Review(engine, directory);
    } catch (const std::exception &error) {
        qCritical() << error.what();
        QCoreApplication::exit(2);
    }
}
