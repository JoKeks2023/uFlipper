#include "powersnapshot.h"
#include <cmath>
namespace UFlipper {
QVariantMap powerSnapshot(const QHash<QByteArray, QByteArray> &properties) {
    QVariantMap raw, values;
    for (auto it = properties.cbegin(); it != properties.cend(); ++it)
        raw.insert(QString::fromUtf8(it.key()), QString::fromUtf8(it.value()));
    const bool supported = properties.value("format.major") == "2";
    struct Metric {
        const char *key;
        const char *unit;
        double min, max;
    };
    const Metric fields[] = {{"charge.level", "%", 0, 100},
                             {"battery.health", "%", 0, 100},
                             {"charge.voltage.limit", "mV", 0, 10000},
                             {"battery.voltage", "mV", 0, 10000},
                             {"battery.current", "mA", -10000, 10000},
                             {"battery.temp", "°C", -60, 120},
                             {"capacity.remain", "mAh", 0, 100000},
                             {"capacity.full", "mAh", 0, 100000},
                             {"capacity.design", "mAh", 0, 100000}};
    for (const auto &field : fields) {
        bool parsed = false;
        double value = properties.value(field.key).toDouble(&parsed);
        bool available =
            supported && parsed && std::isfinite(value) && value >= field.min && value <= field.max;
        QVariantMap metric{{"available", available}, {"unit", QString::fromUtf8(field.unit)}};
        if (available)
            metric.insert("value", value);
        values.insert(QString::fromUtf8(field.key), metric);
    }
    const QByteArray state = properties.value("charge.state");
    bool known = supported && (state == "charging" || state == "charged" || state == "discharging");
    QVariantMap charge{{"available", known}};
    if (known)
        charge.insert("value", QString::fromUtf8(state));
    values.insert("charge.state", charge);
    return {{"formatSupported", supported}, {"raw", raw}, {"metrics", values}};
}
} // namespace UFlipper
