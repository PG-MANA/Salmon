/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * メインウィンドウ クラス
 * 主役である画面の表示を請け負っている。
 */
#pragma once

#include <QMainWindow>
#include "../Twitter/TwitterCore.h"
#include "../Network/Network.h"

class Twitter;
class Streamer;
class TweetInfo;
class QCloseEvent;
class QKeyEvent;
class QVBoxLayout;
class QHBoxLayout;
class QBoxLayout;
class QPushButton;
class QPlainTextEdit;
class QThread;
class QString;
class QByteArray;
class QScrollArea;
class QSystemTrayIcon;

struct TwitterSetting;

namespace TwitterJson {
struct TweetData;
struct NotificationData;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow();
    virtual ~MainWindow();
    bool init ( const char *SettingFile );

public slots:
    void show();
    void tweet();
    void tweetWithMedia ( const QByteArray &id );
    void addMedia();
    void showTweet ( TwitterJson::TweetData *twdata );
    void removeTweet ( const QString &id );
    void showNotification ( TwitterJson::NotificationData *nfdata );
    void showTimeLine();
    void updateTimeLine();
    void showAbout();
    void contentAction ( TwitterJson::TweetData *twdata,unsigned int act );
    void finishedTweet();
    void finishedRequest();
    void abortedTimeLine ( unsigned int error );
    void setAlwayTop ( bool checked );
    void changeStatusStream ( bool checked );

protected:
    virtual void closeEvent ( QCloseEvent *event ) override;
    virtual void keyPressEvent ( QKeyEvent *qkey ) override;
    virtual bool eventFilter ( QObject *obj,QEvent *event ) override;
    void retweet ( TwitterJson::TweetData *twdata );
    void favorite ( TwitterJson::TweetData *twdata );
    void deleteTweet ( TwitterJson::TweetData *twdata );
    Network net;

private:
    void createMenus();
    void createTimeLine();
    void createTweetBox();

    void authorize_twitter();
    bool addMediaByClipboard();
    inline void setEnabledTweet ( bool enable );
    void processTweet(QString &text);

    QByteArray pin_dialog();
    Twitter *twitter;//将来複数持てるかも
    TwitterSetting *twset;
    QVBoxLayout *main_layout;
    QScrollArea *info_scroll_area;//ツイートに画像などを添付する時表示されるもの
    QPushButton *tweet_button;
    TweetInfo *tweet_info;
    QPlainTextEdit *tweet_editer;
    QBoxLayout *timeline_layout;
    Streamer *timeline_streamer;
    QThread *timeline_thread;
    QAction *stream_status;
    QSystemTrayIcon *tray_info;//これだとMainWindowが複数できたときにそれごとにトレイに追加されるのでstaticで管理するか、Salmon.cppが管理する必要が出てくるかもしれない。ただし、show()=>showMessage()=>hide()であたかもメッセージだけ表示された感じになる。これもヒープ上に作るのが世の常らしい(QtドキュメントもHeap上に作ってる。)。
    unsigned int MAX_TWDATA =64;//最高ツイートデータ保持数。これを超えると画面・メモリから消される。 TODO:設定から読んで可変にすべき。
};
