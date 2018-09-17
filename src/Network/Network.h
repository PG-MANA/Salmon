/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * ネットワーククラス
 * 通信を受け持つだけでTwitterに特化はしてない。
 * (特にQObjectを継承する必要がなさそう)
*/
#pragma once

#include <QNetworkAccessManager>

class QNetworkRequest;
class QNetworkReply;
class QUrl;
class QByteArray;

class Network {
public:
    Network();
    ~Network();
    QNetworkReply *get ( const QUrl &url );
    QNetworkReply *get ( QNetworkRequest &req );
    QNetworkReply *post ( const QUrl &url,const QByteArray &data );
    QNetworkReply *post ( QNetworkRequest &req,const QByteArray &data );
    QNetworkReply *upload ( const QUrl &url,const QList<QByteArrayList> &data );
    QNetworkReply *upload ( QNetworkRequest &req,const QList<QByteArrayList> &data );
private:
    QNetworkAccessManager qnet;
    inline QByteArray getUserAgent() const;
};
