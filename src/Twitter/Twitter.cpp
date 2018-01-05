/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * Twitter API接続クラス
 * 接続自体はNetworkクラスに任せる。
 * ここだけ命名規則が違うのは変数の感覚で関数を使ってたのとコードが古いのといい命名が思いつかないからである
 */
#include "Twitter.h"
#include "TwitterUrl.h"
#include "TwitterSetting.h"
#include "../Network/OAuth.h"
#include "../Network/Network.h"
#include "../Salmon.h"
#include <QtCore>
#include <QNetworkRequest>
#include <QNetworkReply>

Twitter::Twitter ( const TwitterSetting *twset ) {
    if ( twset != nullptr ) {
        oauth = new OAuth ( CONS_KEY, CONS_SEC, twset->oauth_token, twset->oauth_secret );
        user_id = twset->user_id;
    } else {
        oauth = nullptr;
    }
}

Twitter::~Twitter() {
    delete oauth;
}

/*
 * 引数:oauth_token,oauth_token_secret(使わない気が...),oauth_verifier(PINコード)
 * 戻値:受信用QNetworkReply
 * 概要:アクセストークンの取得を行う。初期設定時以外には呼ばない。
 */
QNetworkReply *Twitter::access_token ( const char *oauth_token,const char *oauth_token_secret,const char *oauth_verifier ) {
    OAuth pr_oauth ( CONS_KEY, CONS_SEC, oauth_token, oauth_token_secret );
    std::vector<OAuth::entry> ele;
    std::string str;
    QByteArray Body;
    QNetworkRequest request;

    //ヘッダー生成
    ele.push_back ( { "oauth_verifier",oauth_verifier,false } );
    pr_oauth.makeOAuthHeader ( TwitterUrl::access_token, false, ele, str );

    request.setUrl ( QUrl ( TwitterUrl::access_token ) );
    request.setRawHeader ( "Authorization",str.c_str() );
    return net.get ( request );
}

/*
 * 引数:token_key(トークンキー)
 * 戻値:ブラウザで表示するURL
 * 概要:認証ページのURLを生成する。
 */
QUrl Twitter::authorize_url ( const QString& token ) {
    return QUrl ( TwitterUrl::authorize + token );
}

/*
 * 引数:なし
 * 戻値:postしたあとのQNetworkReply
 * 概要:リクエストトークンの取得を行う。初期設定時以外には呼ばない。
 */
QNetworkReply * Twitter::request_token() {
    OAuth pr_oauth ( CONS_KEY, CONS_SEC, "", "" ); //トークン見所持
    std::vector<OAuth::entry> ele;
    std::string str;
    QNetworkRequest request;

    //ヘッダー生成
    ele.push_back ( { "oauth_callback","oob",false } );
    pr_oauth.makeOAuthHeader ( TwitterUrl::request_token, true, ele,str );
    request.setUrl ( QUrl ( TwitterUrl::request_token ) );
    request.setRawHeader ( "Authorization",str.c_str() );
    return net.post ( request,QByteArray() ); //Post内容は空
}

/*
 * 引数:data(access_token関数の実行後のレスポンス),twset(設定格納先)
 * 戻値:TwitterSettings
 * 概要:各種トークンなどのの分析。初期設定時以外には呼ばない。またOAuthクラスの作成もこの関数でされる。
 */
void Twitter::decode_access_token ( const QByteArray &data,TwitterSetting &twset ) {
    QList<QByteArray> list = data.split ( '&' );
    QByteArray  token_key = list[0].split ( '=' ) [1]; //oauth_tokenが、前に入っているという大前提の依存
    QByteArray  token_sec = list[1].split ( '=' ) [1]; //上と同じく拡張性が低い

    strcpy ( twset.oauth_token, token_key.constData() );
    strcpy ( twset.oauth_secret, token_sec.constData() );
    strcpy ( twset.user_id, list[2].split ( '=' ) [1].constData() );
    strcpy ( twset.user_name, list[3].split ( '=' ) [1].constData() );;
    oauth = new OAuth ( CONS_KEY, CONS_SEC, token_key.constData(), token_sec.constData() );
    return;
}

/*
 * 引数:なし
 * 戻値:getしたあとのQNetworkReply
 * 概要:HomeTimeを取得。API制限に気をつける。
 */
QNetworkReply *Twitter::home_timeline() {
    QNetworkRequest req;
    std::vector<OAuth::entry> ele;
    QUrl qurl ( TwitterUrl::home_timeline );

    qurl.setQuery ( QUrlQuery ( "tweet_mode=extended" ) );
    req.setUrl ( qurl );
    ele.push_back ( OAuth::entry {"tweet_mode","extended",true} );
    get ( TwitterUrl::home_timeline,req,ele );
    return net.get ( req );
}

/*
 * 引数:なし
 * 戻値:getしたあとのQNetworkReply
 * 概要:UserStream、いわゆるタイムラインのストリーム。delimited=lengthがつく。
 */
QNetworkReply *Twitter::user_stream() {
    QNetworkRequest req;
    std::vector<OAuth::entry> ele;
    QUrl qurl ( TwitterUrl::user_stream );

    qurl.setQuery ( QUrlQuery ( "delimited=length" ) );
    req.setUrl ( qurl );
    ele.push_back ( OAuth::entry {"delimited","length",true} );
    get ( TwitterUrl::user_stream,req,ele );
    return  net.get ( req );
}

/*
 * 引数:message(ツイート内容)
 * 戻値:結果取得用のQNetworkReply
 * 概要:messageをツイートする。
 */
QNetworkReply *Twitter::tweet ( const QString &message,const QByteArray &media_id/*カンマ区切りで複数行けそう?*/,const QByteArray &reply_id ) {
    QByteArray body;
    std::vector<OAuth::entry> ele;
    QNetworkRequest req;

    if ( !media_id.isEmpty() ) ele.push_back ( {"media_ids",media_id.constData(),true} );
    if ( !reply_id.isEmpty() ) ele.push_back ( {"in_reply_to_status_id",reply_id.constData(),true} );
    body = QUrl::toPercentEncoding ( message );
    ele.push_back ( { "status", body.constData(), true } );
    post ( TwitterUrl::statuse_update,req,ele );
    req.setUrl ( QUrl ( TwitterUrl::statuse_update ) );

    body.prepend ( "status=" ); //作ったので書き換えて良い
    if ( !media_id.isEmpty() ) body.append ( "&media_ids=" ).append ( media_id );
    if ( !reply_id.isEmpty() ) body.append ( "&in_reply_to_status_id=" ).append ( reply_id );
    //送信
    return net.post ( req,body );
}

/*
 * 引数:id(削除をするID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:idを削除する。自分の発言かどうかのチェックはしてない。
 */
QNetworkReply *Twitter::tweet_destroy ( const QByteArray &id ) {
    QString  uri ( TwitterUrl::statuse_destroy + id + ".json" );
    std::vector<OAuth::entry> ele;
    QNetworkRequest req;

    post ( uri.toUtf8().constData(),req,ele );
    req.setUrl ( uri );
    //送信
    return net.post ( req,QByteArray() );
}

/*
 * 引数:id(リツイートをするID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:idをリツイートをする。
 */
QNetworkReply *Twitter::retweet ( const QByteArray &id ) {
    QString  uri ( TwitterUrl::statuse_retweet + id + ".json" );
    std::vector<OAuth::entry> ele;
    QNetworkRequest req;

    post ( uri.toUtf8().constData(),req,ele );
    req.setUrl ( uri );
    //送信
    return net.post ( req,QByteArray() );
}

/*
 * 引数:id(お気に入りに登録するツイートのID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:idをお気に入りに登録する。
 */
QNetworkReply *Twitter::favorite ( const QByteArray &id ) {
    //変数宣言
    std::vector<OAuth::entry> ele;
    QNetworkRequest req;

    ele.push_back ( {"id",id.constData(),true} );
    post ( TwitterUrl::favorite_create,req,ele );
    req.setUrl ( QUrl ( TwitterUrl::favorite_create ) );
    //送信
    return net.post ( req,QByteArray ( "id=" ) + id );
}

/*
 * 引数:total_bytes(総通信量),media_type(MIMETYPE)
 * 戻値:結果取得用のQNetworkReply
 * 概要:media_uploadの初期化を行う。
 */
QNetworkReply *Twitter::media_upload_init ( const QByteArray &total_bytes,const QByteArray &media_type ) {
    std::vector<OAuth::entry> ele;
    QNetworkRequest req;

    ele.push_back ( {"command","INIT",true} );
    ele.push_back ( {"total_bytes",total_bytes.constData(),true} );
    ele.push_back ( {"media_type",media_type.constData(),true} );
    post ( TwitterUrl::media_upload,req,ele );
    req.setUrl ( QUrl ( TwitterUrl::media_upload ) );
    //送信
    return net.post ( req,QByteArray ( "command=INIT&total_bytes=" )+total_bytes+QByteArray ( "&media_type=" )+media_type );
}

/*
 * 引数:id(media_id),data(アップロードするデータ),mime_type(dataのMIME-TYPE)
 * 戻値:結果取得用のQNetworkReply
 * 概要:media_uploadのアップロードを行う。postdataはQHttpMultiPartなどを使いmultipart/form-data形式にすること。
 */
QNetworkReply *Twitter::media_upload_append ( const QByteArray &media_id,const QByteArray &data,const QByteArray &mime_type ) {
    std::vector<OAuth::entry> ele;
    QNetworkRequest req;
    QList<QByteArrayList> upload_data;
    //multipart/form-dataのときOAuthシグネチャ作成にオプションコマンド入れない
    post ( TwitterUrl::media_upload,req,ele );
    req.setUrl ( QUrl ( TwitterUrl::media_upload ) );
    //アップロードリスト作成
    upload_data.push_back ( QByteArrayList ( {"command","","APPEND"} ) );
    upload_data.push_back ( QByteArrayList ( {"media_id","",media_id} ) );
    upload_data.push_back ( QByteArrayList ( {"segment_index","","0"} ) );
    upload_data.push_back ( QByteArrayList ( {"media",mime_type,data} ) );
    //送信
    return net.upload ( req,upload_data );
}

/*
 * 引数:id(media_id)
 * 戻値:結果取得用のQNetworkReply
 * 概要:media_uploadのFINALIZEを行う。
 */
QNetworkReply *Twitter::media_upload_finalize ( const QByteArray &media_id ) {
    std::vector<OAuth::entry> ele;
    QNetworkRequest req;

    ele.push_back ( {"command","FINALIZE",true} );
    ele.push_back ( {"media_id",media_id.constData(),true} );
    post ( TwitterUrl::media_upload,req,ele );
    req.setUrl ( QUrl ( TwitterUrl::media_upload ) );
    //送信
    return net.post ( req,QByteArray ( "command=FINALIZE&media_id=" ) + media_id );
}


/*
 * 引数:uri,req(URLなどをセットしたもの),ele(OAuthで使うもの)
 * 戻値:なし。
 * 概要:OAuthヘッダを作成。送信はしない。
 */
void Twitter::get ( const char *uri,QNetworkRequest &req,std::vector<OAuth::entry> &ele ) {
    std::string str;
    oauth->makeOAuthHeader ( uri, false, ele, str );
    req.setRawHeader ( "Authorization",str.c_str() );
    return;
}

/*
 * 引数:uri,req(URLなどをセットしたもの),ele(OAuthで使うもの)
 * 戻値:なし。
 * 概要:OAuthヘッダを作成。送信はしない。
 */
void Twitter::post ( const char *uri,QNetworkRequest &req,std::vector<OAuth::entry> &ele ) {
    std::string str;
    oauth->makeOAuthHeader ( uri, true, ele, str );
    req.setRawHeader ( "Authorization",str.c_str() );
    return;
}
