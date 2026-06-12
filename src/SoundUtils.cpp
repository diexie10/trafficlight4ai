#include "SoundUtils.h"
#include <QApplication>
#include <QFile>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMessageBox>
#include <QUrl>

QUrl soundUrlForPath(const QString &filePath)
{
    if (filePath.isEmpty())
        return {};

    if (filePath.startsWith(QLatin1String(":/")))
        return QUrl("qrc" + filePath);

    if (QFile::exists(filePath))
        return QUrl::fromLocalFile(filePath);

    return {};
}

void playSound(const QString &filePath, QObject *errorContext)
{
    const QUrl url = soundUrlForPath(filePath);
    if (url.isValid()) {
        auto *player = new QMediaPlayer();
        auto *audioOutput = new QAudioOutput(player);
        player->setAudioOutput(audioOutput);
        player->setSource(url);

        QObject::connect(player, &QMediaPlayer::errorOccurred,
                         player, [player, errorContext, filePath]
                         (QMediaPlayer::Error, const QString &) {
            if (errorContext) {
                auto *widget = qobject_cast<QWidget *>(errorContext);
                QMessageBox::warning(widget,
                    QObject::tr("Audio Error"),
                    QObject::tr("Invalid audio file: %1").arg(filePath));
            }
            player->deleteLater();
        });

        QObject::connect(player, &QMediaPlayer::playbackStateChanged,
                         player, [player](QMediaPlayer::PlaybackState state) {
            if (state == QMediaPlayer::StoppedState) {
                player->deleteLater();
            }
        });

        player->play();
    } else {
        QApplication::beep();
    }
}
