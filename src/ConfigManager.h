#pragma once

#include <QObject>
#include <QJsonObject>
#include <QString>

class ConfigManager : public QObject {
    Q_OBJECT

public:
    explicit ConfigManager(const QString &configPath, QObject *parent = nullptr);
    ~ConfigManager() override;

    QString windowSize() const;
    void setWindowSize(const QString &size);

    int windowPosX() const;
    int windowPosY() const;
    void setWindowPos(int x, int y);

    QString animationMode() const;
    void setAnimationMode(const QString &mode);

    int animationPeriodMs() const;
    void setAnimationPeriodMs(int ms);

    QString socketPath() const;
    void setSocketPath(const QString &path);

    QString aiTool() const;
    void setAiTool(const QString &tool);

    int timeoutSec() const;
    void setTimeoutSec(int sec);

private:
    void load();
    void save();
    void applyDefaults();

    QString m_configPath;
    QJsonObject m_root;
};
