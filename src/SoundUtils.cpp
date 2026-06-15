#include "SoundUtils.h"
#include <QApplication>
#include <QFile>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMessageBox>
#include <QPointer>
#include <QTimer>
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
        QTimer::singleShot(5000, player, &QObject::deleteLater);
        auto *audioOutput = new QAudioOutput(player);
        player->setAudioOutput(audioOutput);
        player->setSource(url);

        QObject::connect(player, &QMediaPlayer::errorOccurred,
                         player, [player, errorContext, filePath]
                         (QMediaPlayer::Error, const QString &) {
            QPointer<QMediaPlayer> playerGuard(player);
            QPointer<QWidget> ctxGuard(qobject_cast<QWidget *>(errorContext));
            if (ctxGuard) {
                QMessageBox::warning(ctxGuard,
                    QObject::tr("Audio Error"),
                    QObject::tr("Invalid audio file: %1").arg(filePath));
            }
            if (playerGuard)
                playerGuard->deleteLater();
        });

        QObject::connect(player, &QMediaPlayer::playbackStateChanged,
                         player, [player](QMediaPlayer::PlaybackState state) {
            QPointer<QMediaPlayer> playerGuard(player);
            if (state == QMediaPlayer::StoppedState) {
                if (playerGuard)
                    playerGuard->deleteLater();
            }
        });

        player->play();
    } else {
        QApplication::beep();
    }
}
