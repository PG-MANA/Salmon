/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * Streamer.cpp
 * 役目は、受信した文字をJSONパースして必要事項を詰め込んで、メインスレッドに投げることだけ。
 */
#include "Streamer.h"
#include "Twitter.h"
#include "TwitterJson.h"
#include "../Salmon.h" // ENABLE_NEW_STREAM
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

Streamer::Streamer ( QObject *parent )
    :QObject ( parent ),json_size ( 0 ),twitter ( nullptr ),reply ( nullptr ) {
}

Streamer::~Streamer() {
    stopUserStream();
    delete twitter;
}

/*
 * 引数:twset(設定を詰めておく)
 * 戻値:なし
 * 概要:Twitterクラス生成。動作するスレッドでよぶ。(QMetaObject::invokeMethodを使ってQt::ConnectionTypeはQt::BlockingQueuedConnection)
 */
void Streamer::setTwitter ( const TwitterSetting *twset ) {
    twitter = new Twitter ( twset );
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:user_streamを開始する。reply->closeするか、deleteするまで永遠と動く。
 */
#if ENABLE_NEW_STREAM
void Streamer::startUserStream() {
    if ( twitter == nullptr ) return emit abort ( TwitterCore::BadPointer );
    if ( reply != nullptr && reply->isRunning() ) return;
    reply = twitter->friends_ids();
    if ( reply->error() !=QNetworkReply::NoError ) {
        delete reply;
        reply = nullptr;
        return emit abort ( TwitterCore::CannotConnect );
    }
    connect ( reply,&QNetworkReply::finished,this,&Streamer::startFilterStream );
    return;
}
#else
void Streamer::startUserStream() {
    if ( twitter == nullptr ) return emit abort ( TwitterCore::BadPointer );
    if ( reply != nullptr && reply->isRunning() ) return;
    reply = twitter->user_stream();
    if ( reply->error() !=QNetworkReply::NoError ) {
        delete reply;
        reply = nullptr;
        return emit abort ( TwitterCore::CannotConnect );
    }
    connect ( reply,&QNetworkReply::readyRead,this,&Streamer::readStream ); //qnet->getの前にconnectしたい(ただQtのサンプルを見る限り間違った実装ではなさそう)
    connect ( reply,&QNetworkReply::finished,this,&Streamer::finishedStream );
    return;
}
#endif

/*
 * 引数:なし
 * 戻値:なし
 * 概要:friends_idsをもとにfilter_streamを開始する。reply->closeするか、deleteするまで永遠と動く。暫定処理
 */
void Streamer::startFilterStream() {
    if ( reply->error() !=QNetworkReply::NoError ) {
        delete reply;
        reply = nullptr;
        return emit abort ( TwitterCore::CannotConnect );
    }
    friend_ids.reserve ( reply->size() );
    QJsonArray &&friends = QJsonDocument::fromJson ( reply->readAll() ).object() ["ids"].toArray();

    for ( const auto &v: friends ) {
        friend_ids += v.toString() + ",";
    }
    friend_ids += twitter->getUserId();
    reply = twitter->filter_stream ( friend_ids.toUtf8() );
    if ( reply->error() !=QNetworkReply::NoError ) {
        delete reply;
        reply = nullptr;
        return emit abort ( TwitterCore::CannotConnect );
    }
    connect ( reply,&QNetworkReply::readyRead,this,&Streamer::readStream ); //qnet->getの前にconnectしたい(ただQtのサンプルを見る限り間違った実装ではなさそう)
    connect ( reply,&QNetworkReply::finished,this,&Streamer::finishedStream );
    return;
}


/*
 * 引数:なし
 * 戻値:なし
 * 概要:user_streamを停止する。スレッドは削除されない。なおこのときfinishedシグナルが出される。
 */
void Streamer::stopUserStream() {
    if ( reply != nullptr ) {
        reply->close();
        delete reply;
        reply = nullptr;
    }
    json_size = 0;
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReply::readyReadのslot。読み込んでTweetDataに格納しNewTweetシグナルを発生させる。
 */
void Streamer::readStream() {
    if ( json_size == 0 ) {
        //サイズ取得
        int max = 22/*longlongの桁数+2(\r\n)*/, size, s, e;
        QByteArray temp ( reply->peek ( max ) );
        for ( s = 0,size = temp.size(); size > s&&! ( temp.at ( s ) >= '0'&&temp.at ( s ) <= '9' ) ; s++ ); //数字が来るまで飛ばす
        for ( e = s; size > e&&temp.at ( e ) != '\r'; e++ );
        if ( e == size ) {
            reply->read ( s ); //if(s==e)\r\nなどが溜まっていた。 else 数字が切れてる or サイズがでかすぎる
            return;
        }
        json_size = temp.mid ( s,e-s ).toLongLong();
        reply->read ( e+2 ); //読み捨て(\rのあとの\nも)
    }
    if ( reply->bytesAvailable() < json_size ) return;
    QJsonObject &&json = QJsonDocument::fromJson ( reply->read ( json_size ) ).object();
    json_size = 0;
    if ( json.isEmpty() ) return; //デコードエラー

    TwitterJson::TweetData *twdata = new TwitterJson::TweetData ( json,twitter->getUserId() );
    if ( !twdata->isEmpty() ) {
#if ENABLE_NEW_STREAM
        if(!friend_ids.contains(twdata->user_info.id)){//エグいにも程がある
            return delete twdata;
        }
#endif
        return emit newTweet ( twdata );
    } else {
        delete twdata;
    }
    TwitterJson::NotificationData *nfdata = new TwitterJson::NotificationData ( json,twitter->getUserId() );
    if ( !nfdata->isEmpty() ) {
        return emit newNotification ( nfdata );
    } else {
        delete nfdata;
    }
    if ( QString &&id = TwitterJson::getDeletedTweetId ( json ); !id.isEmpty() ) return emit deleteTweet ( id );
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReply::finishedのslot。まずストリームを閉じて、シグナルを送出する。
 */
void Streamer::finishedStream() {
    if ( reply ) {
        QNetworkReply::NetworkError error = reply->error();
        if ( reply->isRunning() ) stopUserStream();
        if ( error != QNetworkReply::OperationCanceledError ) emit abort ( TwitterCore::NetworkError );
    }
    return;
}
