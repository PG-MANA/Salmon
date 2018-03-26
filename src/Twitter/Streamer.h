/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * Twitter Stream APIクラス
 * メインスレッドとは別のスレッドで動作する。よってGUIを操作してはいけない。
 */
#pragma once

#include "TwitterCore.h"
#include <QObject>

class Twitter;
class QNetworkReply;

struct TwitterSetting;

namespace TwitterJson {
struct TweetData;
struct NotificationData;
}

class Streamer : public QObject {
    Q_OBJECT
public:
    explicit Streamer ( QObject *parent=nullptr );
    ~Streamer();

signals:
    void newTweet ( TwitterJson::TweetData *twdata );
    void deleteTweet ( const QString &id );
    void newNotification ( TwitterJson::NotificationData *nfdata );
    void abort ( unsigned int err );

public slots:
    void setTwitter ( const TwitterSetting *twset );
    void startUserStream();
    void stopUserStream();
    void readStream();
    void finishedStream();
    void startFilterStream();

protected:
    qint64 json_size;
    QString friend_ids;
    Twitter *twitter;
    QNetworkReply *reply;
};
