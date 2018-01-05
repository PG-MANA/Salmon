/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * TweetContent クラス
 * タイムラインのツイート一つ一つの(HTMLで言う)divみたいなもの
 */
#pragma once

#include <QFrame>
#include "../Twitter/TwitterCore.h"
#include "../Network/Network.h"

class QMouseEvent;
class QMenu;
namespace TwitterJson {
struct TweetData;
}

class TweetContent : public QFrame {
    Q_OBJECT
public:
    enum Mode { //主にメニューの表示内容の制御
        Normal,
        Master,//ツイートの削除などを表示
        Reply = 1 << 2,
        Info = 1 << 3,
        Simple = 1 << 4,
    };

    explicit TweetContent ( TwitterJson::TweetData *_twdata = nullptr,Mode _mode = Mode::Normal,QWidget *_root_widget = Q_NULLPTR,QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
    virtual ~TweetContent();
    void setTweetData ( TwitterJson::TweetData *_twdata );
    TwitterJson::TweetData *getTweetData();

signals:
    void action ( TwitterJson::TweetData *ori,unsigned int act );

private slots:
    void triggeredAction();
    void openUrl();
    void showPicture ( TwitterJson::TweetData *twdata,unsigned int index );
    void openWindow();

private:
    virtual void mousePressEvent ( QMouseEvent* event ) override;
    void drawTweet();
    void createActions();
    Mode mode;
    Network net;
    QWidget *root_widget;
    QMenu *popup;
    TwitterJson::TweetData *twdata;
};
