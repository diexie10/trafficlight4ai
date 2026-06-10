#pragma once

#include <QObject>
#include <QString>

inline constexpr char kDefaultYellowSound[] = ":/effects/yellow.ogg";
inline constexpr char kDefaultGreenSound[] = ":/effects/green.ogg";

void playSound(const QString &filePath, QObject *errorContext = nullptr);
