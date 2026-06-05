#pragma once

#include <QObject>
#include <QString>

void playSound(const QString &filePath, QObject *errorContext = nullptr);
