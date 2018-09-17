/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * Twitter JSON解析クラス
 * TwitterのAPIで得られるJSONを各種解析する。
 */
#include "TwitterJson.h"
#include "../Salmon.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QVector>

namespace TwitterJson {

UserInfo::UserInfo ( const QJsonObject &json ) {
    if ( json.isEmpty() ) return;
    id = json["id_str"].toString().toLatin1().constData();
    screen_name = json["screen_name"].toString();//スクリーンネーム
    user_name = json["name"].toString();//ユーザー名
    user_icon_url= json["profile_image_url_https"].toString();//アイコン
}

UrlInfo::UrlInfo ( const QJsonArray& json ) {
    if ( json.isEmpty() ) return;
    for ( const QJsonValue &url_element_value : json ) {
        const QJsonObject &&url_element = url_element_value.toObject();
        url << url_element["url"].toString();
        display_url<<url_element["display_url"].toString();
    }
}

MediaInfo::MediaInfo ( const QJsonArray &json ) {
    //備考:ダイレクトメッセージでは画像の取得にはOAuth認証が必要。
    if ( json.isEmpty() ) return;
    QJsonObject &&first = json.constBegin()->toObject();
    url = first["url"].toString();

    if ( first["type"].toString() == "video" || first["type"].toString() == "animated_gif" ) {
        direct_links <<  first["media_url_https"].toString();
        for ( QJsonValue &&variant : first["video_info"].toObject() ["variants"].toArray() ) {
            if ( variant.toObject() ["content_type"].toString() == "video/mp4" ) {
                video_url = variant.toObject() ["url"].toString();
                break;//variantsはbitrateやHLS形式が入っており環境ごとに選べるがとりあえず適当にmp4を持ってくる。(Webmがないときがある...)
            }
        }
    } else {
        //画像のリンクを拾っていく。
        //display_url = first["display_url"].toString();
        for ( const QJsonValue &media : json ) {
            if ( media.toObject() ["type"].toString() != "photo" ) continue;
            direct_links  << media.toObject() ["media_url_https"].toString();
        }
    }
}

RetweetedStatus::RetweetedStatus ( const QJsonObject& json ) {
    if ( json.isEmpty() ) return;
    id = json["id_str"].toString().toLatin1().constData();
    user_info = UserInfo ( json["user"].toObject() );
}

TweetData::TweetData ( QJsonObject& tweet,const QByteArray &myid ) {
    if ( tweet.find ( "id" ) == tweet.end() ) return; //IDがないものは処理できないとみなす
    id =  tweet["id_str"].toString().toLatin1().constData();
    via = tweet["source"].toString();//クライアント名
    user_info = UserInfo ( tweet["user"].toObject() );
    if ( myid == user_info.id ) flag |= 2;

    QJsonValue retweet_value = tweet["retweeted_status"];
    if ( retweet_value != QJsonValue::Null/*QJsonValue::Undefinedでないようだ...*/ ) {
        flag |= 1;//RTフラグを立てる
        QJsonObject retweet = retweet_value.toObject();
        retweeted_status = new RetweetedStatus ( retweet );
        setTextByJson ( retweet );
        setDateByString ( retweet["created_at"].toString() );
        if ( retweet["user"].toObject() ["verified"].toBool() ) flag |= 0x20; //公式(確認済み)アカウントかどうか
        if ( retweet["possibly_sensitive"].toBool() ) flag |= 0x40; //気分を害するツイートかどうか(tweet直下のpossiblyはリツイートした人の設定が反映される。)
    } else {
        setTextByJson ( tweet );
        setDateByString ( tweet["created_at"].toString() );
        if ( tweet["user"].toObject() ["protected"].toBool() ) flag |= 0x10; //鍵アカウントかどうか
        if ( tweet["user"].toObject() ["verified"].toBool() ) flag |= 0x20; //公式(確認済み)アカウントかどうか
        if ( tweet["possibly_sensitive"].toBool() ) flag |= 0x40; //気分を害するツイートかどうか
    }
    //引用ツイート
    if ( QJsonValue quote_value = tweet["quoted_status"]; quote_value != QJsonValue::Null ) {
        QJsonObject tmp = quote_value.toObject();
        quoted_status = new TweetData ( tmp,myid );
    }
    //自分に関する情報を取得
    if ( tweet["retweeted"].toBool() ) flag2 |= 1;
    if ( tweet["favorited"].toBool() ) flag2 |= 2;

    //entitiesの場所を探る
    QJsonValue &&entities = tweet["extended_tweet"].toObject() ["entities"];
    if ( entities == QJsonValue::Null &&retweet_value != QJsonValue::Null ) entities = retweet_value.toObject() ["extended_tweet"].toObject() ["entities"];

    //media解析
    QJsonArray &&media_json = entities.toObject() ["media"].toArray();
    if ( media_json.isEmpty() ) media_json = tweet["extended_entities"].toObject() ["media"].toArray(); //retweetでもtopのextended_entitiesに入ってる。また複数の画像はextended_entitiesの中になる
    if ( !media_json.isEmpty() ) {
        media_info = new MediaInfo ( media_json );
        if ( media_info->url.length() ) text.remove ( media_info->url );
        else delete media_info;
    }

    //URL解析
    QJsonArray &&url_json = entities.toObject() ["urls"].toArray();
    if ( url_json.isEmpty() ) {
        if ( retweet_value != QJsonValue::Null ) url_json = retweet_value.toObject() ["entities"].toObject() ["urls"].toArray();
        else url_json = tweet["entities"].toObject() ["urls"].toArray();
    }
    if ( url_json.size() ) { /*必ず配列になってる*/
        url_info= new UrlInfo ( url_json );
        for ( unsigned int cnt = 0,size = url_info->url.size(); cnt < size; cnt++ )
            text.replace ( url_info->url[cnt],url_info->display_url[cnt] );
    }
}

TweetData::TweetData ( const TwitterJson::TweetData& other ) {
    *this = other;//**コピーコンストラクタを実装するときはこの一文をどうにかすること**
    //オプション機能のコピー
    if ( other.retweeted_status ) {
        retweeted_status = new RetweetedStatus;
        *retweeted_status = *other.retweeted_status;
    }
    if ( other.quoted_status ) {
        quoted_status = new TweetData ( *other.quoted_status );
    }
    if ( other.media_info ) {
        media_info = new MediaInfo;
        *media_info = *other.media_info;
    }
    if ( other.url_info ) {
        url_info = new UrlInfo;
        *url_info = *other.url_info;
    }
}


TweetData::~TweetData() {
    //今のところは消すだけ(deleteはnullptrでも安全らしい)
    delete retweeted_status;
    delete quoted_status;
    delete media_info;
    delete url_info;
}

/*
 * 引数:tweet(エンコードされた文字)
 * 戻値:なし
 * 概要:エンティティを戻してtextに格納する。
 */
void TweetData::setTextWithDecode ( QString &tweet ) {
    text = tweet.replace ( "&gt;",">" ).replace ( "&lt;","<" ).replace ( "&amp;","&" );
    return;
}

/*
 * 引数:json(Twitter APIのObject)
 * 戻値: なし
 * 概要:ツイートを様々なところから探し出し格納。
 */
void TweetData::setTextByJson ( const QJsonObject& json ) {
    QString tweet;//QStringはバッファを共有しているのでムーブセマンティクスは不要
    if ( tweet =  json["full_text"].toString(),!tweet.isEmpty() ); //すること無い
    else if ( tweet = json["extended_tweet"].toObject() ["full_text"].toString(),!tweet.isEmpty() );
    else tweet = json["text"].toString();
    return setTextWithDecode ( tweet );
}

/*
 * 引数:created_at(Twitter APIのcreated_at)
 * 戻値:なし
 * 概要:Twitterの投げてくる時間の文字列をRFC2822形式に直しQDateにしてdateに格納。
 */
void TweetData::setDateByString ( const QString& created_at ) {
    //created_atの構成は"Wed Aug 27 13:08:45 +0000 2008"
    QStringList &&temp = created_at.split ( ' ' ); //section使うよりメモリを喰うが一発で終わる
    date = QDateTime::fromString ( temp.at ( 0 ) + ',' + temp.at ( 2 ) + ' '+ temp.at ( 1 ) + ' ' + temp.at ( 5 ) + ' ' + temp.at ( 3 ) + ' ' + temp.at ( 4 ) /*ゴリ押し*/,Qt::RFC2822Date ).toLocalTime();
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:中身が空かどうかを返す
 */
bool TweetData::isEmpty() {
    return id.isEmpty();
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:自分のツイートかどうかを返す
 */
bool TweetData::isMytweet() {
    return flag & ( 1 << 1 );
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:リツートかどうかを調べて元のツイートのidを返す
 */
QByteArray & TweetData::getOriginalId() {
    return retweeted_status?retweeted_status->id:id;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:リツートかどうかを調べて元のツイートのUserInfoを返す
 */
UserInfo & TweetData::getOriginalUserInfo() {
    return retweeted_status?retweeted_status->user_info:user_info;
}

NotificationData::NotificationData ( const QJsonObject& json,const QByteArray &myid ) {
    QString &&event_name = json["event"].toString();

    //ここの文字列を定数化すべきかな
    if ( event_name == "favorite" ) event = Event::Favorite;
    else if ( event_name == "unfavorite" ) event = Event::Unfavorite;
    else if ( event_name == "quoted_tweet" ) event = Event::QuotedTweet;
    else if ( event_name == "follow" ) event = Event::Follow;
    else if ( event_name == "unfollow" ) event = Event::Unfollow;
    else if ( event_name == "list_created" ) event = Event::ListCreated;
    else if ( event_name == "list_destroyed" ) event = Event::ListDestroyed;
    else if ( event_name == "list_member_added" ) event = Event::ListMemberAdded;
    else if ( event_name == "list_member_removed" ) event = Event::ListMemberRemoved;
    else {
        event = Event::NoEvent;
        return;//今のところいらない
    }

    target = UserInfo ( json["target"].toObject() );
    source = UserInfo ( json["source"].toObject() );
    if ( myid == source.id ) {
        event = Event::NoEvent;
        return;
    }
    QJsonObject &&tweet = json["target_object"].toObject();
    target_object_tweet = new TweetData ( tweet,myid ); //Listは処理できない(削除はデストラクタに任せる)
    //今の所Listの場合は何もしない
}

NotificationData::~NotificationData() {
    delete target_object_tweet;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:中身が空かどうかを返す
 */
bool NotificationData::isEmpty() {
    return ( event == Event::NoEvent );
}

/*
 * 引数:json(Twitter APIによって返されるJSON)
 * 戻値:削除されたTweetID
 * 概要:jsonが削除メッセージだった場合、該当ツイートのIdを返す。
 */
QString getDeletedTweetId ( const QJsonObject& json ) {
    return json["delete"].toObject() ["status"].toObject() ["id_str"].toString();
}

/*
 * 引数:json(Twitter APIによって返されるJSON[配列])
 * 戻値:(1個目はList ID、２個目は表示名)の配列
 * 概要:jsonを解析して、ListIDと名前を返す。
 */
QVector<QPair<QByteArray,QString>> getListInfo ( const QJsonArray &json ) {
    QVector<QPair<QByteArray,QString>> res;
    for ( int cnt = 0,len = json.size(); cnt < len; cnt++ ) {
        res.append ( QPair<QByteArray,QString> ( json[cnt].toObject() ["id_str"].toString().toUtf8(),json[cnt].toObject() ["name"].toString() ) );
    }
    return res;
}
}
