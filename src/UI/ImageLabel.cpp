/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * ImageLabel クラス
 * これを応用すれば画像や動画のラベルも作れる。
 */
#include "ImageLabel.h"
#include "TweetContent.h"
#include <QHash>
#include<QNetworkReply>
#include <QMouseEvent>

QHash<QString,QPixmap> ImageLabel::images;

ImageLabel::ImageLabel ( const unsigned int _sizex,const unsigned int _sizey,const unsigned int _index,TweetContent *_parent_content,QWidget *parent, Qt::WindowFlags f )
    :QLabel ( parent,f ),parent_content ( _parent_content ),index ( _index ),sizex ( _sizex ),sizey ( _sizey ) {
}

/*
 * 引数:url(画像の名前)
 * 戻値:キャッシュから設定できた場合true、できなかった場合はfalse
 * 概要:渡されたURLでキャッシュを探し見つかった場合はそこから設定する。
 */
bool ImageLabel::setPixmapByName ( const QString &_url ) {
    url = _url;
    const QHash<QString,QPixmap>::iterator image = images.find ( _url );
    if ( image == images.end() ) {
        if ( images.size() >256 ) images.clear(); //ここらで一回全部消してメモリの使用を減らす
        return false;
    }
    setPixmap ( image.value() );
    return true;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReplyのfinishedによって呼ばれる。画像の設定、キャッシュへ登録を行う。
 */
void ImageLabel::setPixmapByNetwork() {
    QNetworkReply *rep = qobject_cast<QNetworkReply*> ( sender() );
    QPixmap p;

    rep->deleteLater();//returnしたあとに削除される
    if ( rep->error() != QNetworkReply::NoError || !p.loadFromData ( rep->readAll() ) ) return;
    if ( sizex&&sizey ) {
        p = p.scaled ( sizex,sizey,Qt::KeepAspectRatio ); //縮小
        images[url] = p;
    }
    setPixmap ( p );
    return;
}

/*
 * 引数:なし
 * 戻値:TweetContent (親Widget)
 * 概要:親のTweetContentを返す
 */
TweetContent *ImageLabel::getParentContent() {
    return parent_content;
}

/*
 * 引数:TweetContent (新しい親Widget)
 * 戻値:なし
 * 概要:TweetContent を更新するときに使う。
 */
void ImageLabel::setParentContent ( TweetContent *_parent_content ) {
    parent_content = _parent_content;
    return;
}

/*
 * 引数:sizex,sizey(新しいsize)
 * 戻値:なし
 * 概要:縮小サイズを更新するときに使う。
 */
void ImageLabel::setSize ( unsigned int _sizex,unsigned int _sizey ) {
    sizex = _sizex;
    sizey = _sizey;
    return;
}

/*
 * 引数:sizex,sizey(結果受け取り用)
 * 戻値:なし
 * 概要:縮小サイズを取得するときに使う。
 */
void ImageLabel::getSize ( unsigned int &_sizex,unsigned int &_sizey ) {
    _sizex = sizex;
    _sizey = sizey;
    return;
}

/*
 * 引数:なし
 * 戻値:index(画像が何番目か)
 * 概要:Layoutなどで画像が何番目かを返す。
 */
unsigned int ImageLabel::getIndex() {
    return index;
}

/*
 * 引数:index(画像が何番目か)
 * 戻値:なし
 * 概要:Layoutなどで画像が何番目かを変更する。
 */
void ImageLabel::setIndex ( unsigned int i ) {
    index = i;
    return;
}


/*
 * 引数:event
 * 戻値:なし
 * 概要:クリックされたときにシグナルを発信する。
 * 備考:ダブルクリックはmouseDoubleClickEventになる。
 */
void ImageLabel::mousePressEvent ( QMouseEvent* event ) {
    switch ( event->button() ) {
    case Qt::LeftButton:
        emit clicked ( ( parent_content ) ?parent_content->getTweetData() :nullptr,index );
        break;
    case Qt::RightButton:
        emit rightClicked ( ( parent_content ) ?parent_content->getTweetData() :nullptr,index );
        break;
    default:
        event->ignore();
        return;
    }
    event->accept();
    return;
}
