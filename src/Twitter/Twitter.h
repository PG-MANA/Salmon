/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * Twitter API接続クラス
 * 接続自体はNetworkクラスに任せる。
 */
#pragma once

#include "TwitterCore.h"
#include "../Network/Network.h"
#include "../Network/OAuth.h"
#include <QStringList>

class  QNetworkReply;
class QNetworkRequest;
class Network;

struct TwitterSetting;

class Twitter {
public:
    explicit Twitter ( const TwitterSetting* twset = nullptr );
    virtual ~Twitter();
    inline QByteArray getUserId() const {
        return user_id;
    };

    //認証関係
    QNetworkReply* request_token();
    QUrl authorize_url ( const QString &token );
    QNetworkReply* access_token ( const char *oauth_token,const char *oauth_token_secret,const char *oauth_verifier );
    void decode_access_token ( const QByteArray &data,TwitterSetting &twset );

    //ツイート関係
    QNetworkReply* tweet ( const QString &message,const QByteArray &media_id = QByteArray() /*ポインタのほうがいいかな...*/,const QByteArray &reply_id = QByteArray() );
    QNetworkReply* tweet_destroy ( const QByteArray &id );
    QNetworkReply* retweet ( const QByteArray &id );
    QNetworkReply *favorite ( const QByteArray &id );

    //取得関係
    QNetworkReply *home_timeline ( const QByteArray &since_id = QByteArray() );
    QNetworkReply *user_stream();
    QNetworkReply *filter_stream(const QByteArray &follow);

    //メディア関係
    QNetworkReply *media_upload_init ( const QByteArray &total_bytes,const QByteArray &media_type );
    QNetworkReply *media_upload_append ( const QByteArray &media_id,const QByteArray &data,const QByteArray &mime_type );
    QNetworkReply *media_upload_finalize ( const QByteArray &media_id );

    //ユーザ関係
    QNetworkReply *friends_ids ( const QByteArray &cursor = QByteArray() );

    //リスト関係
    QNetworkReply *get_lists();

protected:
    /*汎用関数。あくまでTwitterクラスから呼ぶもので他のクラス(UIなど)からはこれを呼ばず専用の関数を作る。*/
    void get ( const char *uri,QNetworkRequest &req,std::vector<OAuth::entry> &ele ); //Requestを作成するだけ
    void post ( const char *uri,QNetworkRequest &req,std::vector<OAuth::entry> &ele );

private:
    QByteArray user_id;
    OAuth *oauth;
    Network net;
};
