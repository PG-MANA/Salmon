/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * VideoPlayer クラス
 * 動画をを再生する。Phonon依存している
 */
#include <QWidget>
#include "../Network/Network.h"
#include "../Twitter/Twitter.h"
#include "../Twitter/TwitterJson.h"

#if __has_include("phonon/VideoPlayer")
namespace Phonon {
class VideoPlayer;
}
#else
#define NO_PHONON
class QMediaPlayer;
#endif

class QVBoxLayout;
class QPushButton;

class VideoPlayer : public QWidget {
    Q_OBJECT
public:
    explicit VideoPlayer ( TwitterJson::TweetData *twdata,QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );

public slots:
    void show();
private:
    void createButtons ( QVBoxLayout *main_layout );
    TwitterJson::MediaInfo media_data;
#ifdef NO_PHONON
    QMediaPlayer *video_player;
#else
    Phonon::VideoPlayer *video_player;
#endif
};
