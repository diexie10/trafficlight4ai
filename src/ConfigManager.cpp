#include "ConfigManager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <unistd.h>
#include <algorithm>

static const QStringList kValidSizes = {"small", "medium", "large"};
static const QStringList kValidModes = {"breathing", "classic"};

static QString defaultSocketPath()
{
    QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (!runtimeDir.isEmpty())
        return runtimeDir + "/trafficlight4ai.sock";
    return QString("/tmp/trafficlight4ai-%1.sock").arg(getuid());
}

ConfigManager::ConfigManager(const QString &configPath, QObject *parent)
    : QObject(parent), m_configPath(configPath)
{
    load();
}

ConfigManager::~ConfigManager()
{
    save();
}

void ConfigManager::applyDefaults()
{
    QJsonObject window;
    window["size"] = "small";
    window["posX"] = 20;
    window["posY"] = 20;

    QJsonObject animation;
    animation["mode"] = "breathing";
    animation["periodMs"] = 1000;

    QJsonObject socket;
    socket["path"] = defaultSocketPath();

    m_root["window"] = window;
    m_root["animation"] = animation;
    m_root["socket"] = socket;
    m_root["aiTool"] = "codex";
    m_root["timeoutSec"] = 300;
}

void ConfigManager::load()
{
    QFile file(m_configPath);
    if (!file.exists()) {
        applyDefaults();
        save();
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        applyDefaults();
        save();
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        applyDefaults();
        save();
        return;
    }

    // Apply defaults first, then overlay loaded values to fill missing keys
    applyDefaults();
    const QJsonObject loaded = doc.object();
    for (auto it = loaded.begin(); it != loaded.end(); ++it)
        m_root[it.key()] = it.value();

    normalize();
}

void ConfigManager::save()
{
    QDir dir = QFileInfo(m_configPath).absoluteDir();
    if (!dir.exists())
        dir.mkpath(".");

    QFile file(m_configPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(m_root).toJson(QJsonDocument::Indented));
        file.close();
    }
}

QString ConfigManager::windowSize() const
{
    return m_root["window"].toObject()["size"].toString("small");
}

void ConfigManager::setWindowSize(const QString &size)
{
    if (!kValidSizes.contains(size))
        return;
    QJsonObject window = m_root["window"].toObject();
    window["size"] = size;
    m_root["window"] = window;
    save();
}

int ConfigManager::windowPosX() const
{
    return m_root["window"].toObject()["posX"].toInt(20);
}

int ConfigManager::windowPosY() const
{
    return m_root["window"].toObject()["posY"].toInt(20);
}

void ConfigManager::setWindowPos(int x, int y)
{
    QJsonObject window = m_root["window"].toObject();
    window["posX"] = x;
    window["posY"] = y;
    m_root["window"] = window;
    save();
}

QString ConfigManager::animationMode() const
{
    return m_root["animation"].toObject()["mode"].toString("breathing");
}

void ConfigManager::setAnimationMode(const QString &mode)
{
    if (!kValidModes.contains(mode))
        return;
    QJsonObject animation = m_root["animation"].toObject();
    animation["mode"] = mode;
    m_root["animation"] = animation;
    save();
}

int ConfigManager::animationPeriodMs() const
{
    return m_root["animation"].toObject()["periodMs"].toInt(1000);
}

void ConfigManager::setAnimationPeriodMs(int ms)
{
    ms = std::clamp(ms, 200, 5000);
    QJsonObject animation = m_root["animation"].toObject();
    animation["periodMs"] = ms;
    m_root["animation"] = animation;
    save();
}

QString ConfigManager::socketPath() const
{
    const QByteArray envPath = qgetenv("TL4AI_SOCKET");
    if (!envPath.isEmpty())
        return QString::fromLocal8Bit(envPath);
    return m_root["socket"].toObject()["path"].toString(defaultSocketPath());
}

void ConfigManager::setSocketPath(const QString &path)
{
    if (path.isEmpty())
        return;
    QJsonObject socket = m_root["socket"].toObject();
    socket["path"] = path;
    m_root["socket"] = socket;
    save();
}

QString ConfigManager::aiTool() const
{
    return m_root["aiTool"].toString("codex");
}

void ConfigManager::setAiTool(const QString &tool)
{
    m_root["aiTool"] = tool;
    save();
}

int ConfigManager::timeoutSec() const
{
    return m_root["timeoutSec"].toInt(300);
}

void ConfigManager::setTimeoutSec(int sec)
{
    if (sec != 0)
        sec = std::clamp(sec, 30, 3600);
    m_root["timeoutSec"] = sec;
    save();
}

void ConfigManager::normalize()
{
    // Validate window.size
    QJsonObject window = m_root["window"].toObject();
    if (!kValidSizes.contains(window["size"].toString()))
        window["size"] = "small";
    m_root["window"] = window;

    // Validate animation.mode and animation.periodMs
    QJsonObject animation = m_root["animation"].toObject();
    if (!kValidModes.contains(animation["mode"].toString()))
        animation["mode"] = "breathing";
    int periodMs = animation["periodMs"].toInt(1000);
    animation["periodMs"] = std::clamp(periodMs, 200, 5000);
    m_root["animation"] = animation;

    // Validate timeoutSec
    int timeout = m_root["timeoutSec"].toInt(300);
    if (timeout != 0)
        timeout = std::clamp(timeout, 30, 3600);
    m_root["timeoutSec"] = timeout;

    // Validate socket.path
    QJsonObject socket = m_root["socket"].toObject();
    if (socket["path"].toString().isEmpty())
        socket["path"] = defaultSocketPath();
    m_root["socket"] = socket;

    save();
}
