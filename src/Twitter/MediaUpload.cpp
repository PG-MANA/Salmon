/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * MediaUpload クラス
 * MediaUploadに使う。INIT=>APPEND=>FINALIZEと処理していき、media_idを返す。
 */
#include "MediaUpload.h"
#include "Twitter.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>

//MEMO:APIを叩くときはtwitterクラスを使う。(additional_owners対応したら面白いかも。)
MediaUpload::MediaUpload ( const QByteArrayList & _list, const QByteArrayList &mime,Twitter *tw,QObject *parent )
    :QObject ( parent ),list ( _list ),mimetype ( mime ),twitter ( tw ),counter ( 0 ) {
}

MediaUpload::~MediaUpload() {
}

/*
 * 引数:なし
 * 戻値:開始作業が成功したか。
 * 概要:アップロード作業を開始する。
 */
bool MediaUpload::start() {
    if ( twitter == nullptr || !list.size() || !mimetype.size() ) return false;
    connect ( twitter->media_upload_init ( QByteArray::number ( list.at ( counter ).size() ),mimetype.at ( counter ) ),&QNetworkReply::finished,this,&MediaUpload::append );
    return true;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReplyによって呼ばれる。本格的にアップロードを行う。
 */
void MediaUpload::append() {
    QNetworkReply *rep =qobject_cast<QNetworkReply*> ( sender() );
    rep->deleteLater();
    if ( rep->error() != QNetworkReply::NetworkError::NoError ) return emit aborted();
    media_id = QJsonDocument::fromJson ( rep->readAll() ).object() ["media_id_string"].toString().toUtf8();
    //とりあえず簡単に一個ずつアップロードする。(1MBで区切るようにできるとなお良い)
    connect ( twitter->media_upload_append ( media_id,list.at ( counter ),mimetype.at ( counter ) ),&QNetworkReply::finished,this,&MediaUpload::finalize );
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReplyによって呼ばれる。FINALIZE作業。
 */
void MediaUpload::finalize() {
    QNetworkReply *rep =qobject_cast<QNetworkReply*> ( sender() );
    rep->deleteLater();
    if ( rep->error() != QNetworkReply::NetworkError::NoError ) return emit aborted();
    else connect ( twitter->media_upload_finalize ( media_id ),&QNetworkReply::finished,this,&MediaUpload::next );
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:次のファイルを上げる。
 */
void MediaUpload::next() {
    QNetworkReply *rep =qobject_cast<QNetworkReply*> ( sender() );
    rep->deleteLater();
    if ( rep->error() != QNetworkReply::NetworkError::NoError ) return emit aborted();
    id += ( ( id.isEmpty() ) ?"":"," )+ QJsonDocument::fromJson ( rep->readAll() ).object() ["media_id_string"].toString().toUtf8();
    counter++;
    if ( counter >= list.count() ) emit finished ( id );
    else start();
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:再試行する。失敗したファイルを上げ直す
 */
void MediaUpload::retry() {
    start();
    return;
}

