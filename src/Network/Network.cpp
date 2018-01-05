/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * ネットワーククラス
 * Windows版で言うWinHTTP.cppかな。
 */
#include "Network.h"
#include "../Salmon.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>

Network::Network() {
}

Network::~Network() {
}

/*
 * 引数:url
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたurlにGETリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::get ( const QUrl &url ) {
    QNetworkRequest req;
    req.setUrl ( url );
    return get ( req );
}

/*
 * 引数:req(QNetworkRequestで、最低URLは設定しておく)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたQNetworkRequestを使ってGETリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::get ( QNetworkRequest &req ) {
    req.setHeader ( QNetworkRequest::UserAgentHeader, getUserAgent() );
    return  qnet.get ( req );
}

/*
 * 引数:url,data(本体、POSTするデータ)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたurlとdataを使ってPOSTリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::post ( const QUrl &url,const QByteArray &data ) {
    QNetworkRequest req;
    req.setUrl ( url );
    return post ( req,data );
}

/*
 * 引数:req(QNetworkRequestで、最低URLは設定しておく),data(本体、POSTするデータ)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたQNetworkRequestlとdataを使ってPOSTリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::post ( QNetworkRequest &req,const QByteArray &data ) {
    req.setHeader ( QNetworkRequest::UserAgentHeader, getUserAgent() );
    //req.setRawHeader("Content-Length",data.size());
    req.setHeader ( QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded" );
    return  qnet.post ( req,data );
}

/*
 * 引数:url,data(本体、POSTするデータ)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたurlとdataを使ってPOSTリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::upload ( const QUrl &url,const QList<QByteArrayList> &data ) {
    QNetworkRequest req;
    req.setUrl ( url );
    return upload ( req,data );
}

/*
 * 引数:req(QNetworkRequestで、最低URLは設定しておく),data(本体、POSTするデータ)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたQNetworkRequestlとdataを使ってmultipart/form-data形式のPOSTリクエストを送る。
 * data=>QByteArrayList(0:title,1:mime_type,2:data)
 */
QNetworkReply *Network::upload ( QNetworkRequest &req,const QList<QByteArrayList> &data ) {
    req.setHeader ( QNetworkRequest::UserAgentHeader, getUserAgent() );
    //req.setRawHeader("Content-Length",data.size());
    //req.setHeader ( QNetworkRequest::ContentTypeHeader,"multipart/form-data");

    //multipart/form-data形式へ変換
    QHttpMultiPart *multiformPart = new QHttpMultiPart ( QHttpMultiPart::FormDataType );

    for ( const QByteArrayList &entry :data ) {
        if ( entry.size() !=3 ) continue; //無効
        QHttpPart dataPart;
        dataPart.setHeader ( QNetworkRequest::ContentDispositionHeader,QVariant ( "form-data; name=\""+entry.at ( 0 )+"\"" ) );
        //dataPart.setHeader(QNetworkRequest::ContentTypeHeader,QVariant(entry.at(1)));
        dataPart.setBody ( entry.at ( 2 ) );
        multiformPart->append ( dataPart );
    }
    QNetworkReply *rep = qnet.post ( req,multiformPart );
    multiformPart->setParent ( rep );
    return rep;
}

/*
 * 引数:なし
 * 戻値:ユーザーエージェント(QByteArray)
 * 概要:マクロ定数を使用するのを避けるために作られたメンバ
 */
QByteArray Network::getUserAgent() const {
    return QByteArray ( USER_AGENT );
}
