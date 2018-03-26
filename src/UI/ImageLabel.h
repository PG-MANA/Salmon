/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * ImageLabel クラス
 * Salmonで画像の表示するためのキャッシュ機能などをつけたQLabel
 */
#pragma once

#include <QLabel>
#include <QHash>

class QMouseEvent;
class TweetContent;

namespace TwitterJson {
struct TweetData;
}

class ImageLabel : public QLabel {
    Q_OBJECT
public:
    explicit ImageLabel ( const unsigned int _sizex = 0,const unsigned int _sizey = 0,const unsigned int _index = 0,TweetContent *_parent_content = Q_NULLPTR,QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
    bool setPixmapByName ( const QString &name );

    TweetContent *getParentContent();
    void setParentContent ( TweetContent *_parent_content );
    void getSize ( unsigned int &_sizex,unsigned int &_sizey );
    void setSize ( unsigned int _sizex,unsigned int _sizey );
    unsigned int getIndex();
    void setIndex ( unsigned int index );

signals:
    void clicked ( TwitterJson::TweetData *twdata,unsigned int index );
    void rightClicked ( TwitterJson::TweetData *twdata,unsigned int index );
public slots:
    void setPixmapByNetwork();//QNetworkReplyのfinishedと接続する。
protected:
    void mousePressEvent ( QMouseEvent* event ) override;
    static QHash<QString,QPixmap> images;//C++でいう、unordered_map

    QString url;
    TweetContent *parent_content;//親(TweetDataを引っ張り出すため)
    unsigned int index;//何番目か(0から始まる)
    unsigned int sizex,sizey;//縮小サイズ(0なら縮小しない)
};

