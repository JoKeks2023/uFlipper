#pragma once
#include <QByteArray>
#include <QHash>
#include <QVariantMap>
namespace UFlipper {
// Keep raw fields alongside values with explicit units. Unknown fields are never guessed.
QVariantMap powerSnapshot(const QHash<QByteArray, QByteArray> &properties);
} // namespace UFlipper
