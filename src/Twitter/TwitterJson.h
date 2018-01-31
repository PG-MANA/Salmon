/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * Twitter JSON解析クラス
 * TwitterのAPIで得られるJSONを各種解析する。
 * TwitterJsonネームスペースで囲んでる
 */
#pragma once

#include <QStringList>
#include <QDateTime>

class QJsonObject;
class QJsonArray;

namespace TwitterJson {
/*
 * 各ツイートを扱いやすいように整理した構造体
 * new TweetData()と初期化すべきだが、一応初期値を入れた。
 */

//ユーザー情報
struct UserInfo {
    QByteArray id;
    QString screen_name;//スクリーンネーム(ABC)
    QString user_name;
    QString user_icon_url;//ユーザアイコン

    explicit UserInfo ( const QJsonObject &json );
    UserInfo() {};
};

//画像・動画情報
struct MediaInfo {
    QString url;//t.co形式のurl
    //QString display_url;//pic.twitter.comの形のやつ(使ってないので休止中)
    QString video_url;//これが空でない場合はdirect_linksにはサムネイルが入ってる
    QStringList direct_links;

    explicit MediaInfo ( const QJsonArray &json );
    MediaInfo() {};
};

//URLリスト
struct UrlInfo {
    QStringList url;//t.co形式のurl
    QStringList display_url;//見た目
    /*QStringList expanded_url;//完全なURL*/
    //同じat(i)で参照できるようにする。

    explicit UrlInfo ( const QJsonArray &json );
    UrlInfo() {};
};

//Retweet情報
struct RetweetedStatus {
    QByteArray id;
    UserInfo user_info;

    explicit RetweetedStatus ( const QJsonObject &json );
    RetweetedStatus() {};
};

struct TweetData {
    QByteArray id;//QStringよりも扱いやすく、数字しか入らないはずだから。
    QDateTime date;
    QString via;//クライアント名
    QString text;
    UserInfo user_info;

    //オプション項目(サイズ削減のため基本ポインタとなる)
    //メンバを追加したらTweetData(other)にも処理を追加すること
    RetweetedStatus *retweeted_status =nullptr;
    MediaInfo *media_info = nullptr;
    UrlInfo *url_info = nullptr;
    TweetData *quoted_status = nullptr;

    char flag = 0 ;// 1bit目: RTなら1 2bit目:自分のツイートか 3bit目 : リプかどうか 4bit目 : DMかどうか 5bit目:5:鍵アカかどうか 6bit目:公式(確認済み)かどうか(RTの場合はツイート主が公式かどうか) 7bit目:possibly_sensitiveが設定されてるかどうか。
    char flag2 = 0;// 1bit目 : 1ならRT済み 2bit目 : 1ならfav済み

    //メソッド
    explicit TweetData ( QJsonObject &tweet,const QByteArray &myid/*自分のユーザID*/ );
    explicit TweetData ( const TweetData &other );
    /*virtual*/ ~TweetData();
    void setTextWithDecode ( QString &text );
    void setTextByJson ( const QJsonObject &json );
    void setDateByString ( const QString &created_at );
    bool isEmpty();
    bool isMytweet();
    /*inline*/ QByteArray &getOriginalId();
    /*inline*/ UserInfo &getOriginalUserInfo();
};

/*
 * 通知データを利用しやすいようにしたもの
 *
 */
enum Event { //使いそうなやつだけ
    NoEvent,
    Delete,
    Favorite,
    Unfavorite,
    QuotedTweet,//引用ツイート
    Follow,
    Unfollow,
    ListCreated,
    ListDestroyed,
    ListUpdated,
    ListMemberAdded,
    ListMemberRemoved
};

struct NotificationData {
    Event event;
    UserInfo target;
    UserInfo source;
    TweetData *target_object_tweet = nullptr;//Tweetの場合はこれ、それ以外の場合はnullptr

    explicit NotificationData ( const QJsonObject &json,const QByteArray &myid );
    /*virtual*/ ~NotificationData();
    bool isEmpty();
};

//その他関数
QString getDeletedTweetId ( const QJsonObject &json );

};
