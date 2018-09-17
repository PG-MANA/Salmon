/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * MediaUpload クラス
 * MediaUploadに使う。INIT=>APPEND=>FINALIZEと処理していき、media_idを返す。
 */
#pragma once

#include <QObject>
#include <QByteArray>

class Twitter;

class MediaUpload : public QObject {
    Q_OBJECT
public:
    explicit MediaUpload ( const QByteArrayList &_list,const QByteArrayList &mime,Twitter *tw,QObject *parent = Q_NULLPTR );
    virtual ~MediaUpload();
    bool start();

signals:
    void finished ( const QByteArray &media_id );
    void aborted();

public slots:
    void append();
    void finalize();
    void next();
    void retry();

private:
    QByteArrayList list;
    QByteArrayList mimetype;
    QByteArray id;//=media_ids
    QByteArray media_id;//操作中のmedia_id
    Twitter *twitter;
    unsigned int counter;
};
