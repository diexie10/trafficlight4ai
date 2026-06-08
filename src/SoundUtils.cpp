#include "SoundUtils.h"
#include <QApplication>
#include <QFile>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMessageBox>
#include <QUrl>

void playSound(const QString &filePath, QObject *errorContext)
{
    if (!filePath.isEmpty() && QFile::exists(filePath)) {
        auto *player = new QMediaPlayer();
        auto *audioOutput = new QAudioOutput(player);
        player->setAudioOutput(audioOutput);
        player->setSource(QUrl::fromLocalFile(filePath));

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
