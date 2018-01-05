/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * TweetInfo クラス
 * MainWindowのしたにある画像添付したりリプライしたりするときに出てくるやつ。
 */
#include "TweetInfo.h"
#include "TweetContent.h"
#include "../Twitter/TwitterJson.h"
#include "MainWindow.h"
#include "ImageLabel.h"
#include <QtWidgets>

TweetInfo::TweetInfo ( MainWindow *parent_window, QWidget *parent , Qt::WindowFlags f ) : QWidget ( parent, f ),win ( parent_window ) {
    main_layout = new QVBoxLayout ( this );
    media_layout = new QHBoxLayout;
    QLabel *info_text_label = new QLabel;
    info_text_label->setPixmap ( QPixmap ( ":/add.png" ) );
    info_text_label->setVisible ( false );
    media_layout->addWidget ( info_text_label );
    media_layout->addStretch();//こっちの方が見た目がいいかな
    main_layout->addLayout ( media_layout );
    reply_layout = new QHBoxLayout;
    info_text_label = new QLabel;
    info_text_label->setPixmap ( QPixmap ( ":/rp.png" ) );
    info_text_label->setVisible ( false );
    reply_layout->addWidget ( info_text_label );
    main_layout->addLayout ( reply_layout );
    quote_layout = new QHBoxLayout;
    info_text_label = new QLabel;
    info_text_label->setPixmap ( QPixmap ( ":/rt.png" ) );
    info_text_label->setVisible ( false );
    quote_layout->addWidget ( info_text_label );
    main_layout->addLayout ( quote_layout );
    return;
}

TweetInfo::~TweetInfo() {
    return;
}

/*
* 引数:index(0から始まる数で何番目のImageLabelか指定)
* 戻値:QPixmap
* 概要:ImageLabelのPixmapをとってくる。
*/
const QPixmap *TweetInfo::getImage ( const unsigned int index ) const {
    if ( countImage() <= index ) return nullptr;
    QLayoutItem *item = media_layout->itemAt ( index + 2/*QLabel + addStretch分*/ );
    return item? ( qobject_cast<ImageLabel*> ( item->widget() ) )->pixmap() :nullptr;
}

/*
* 引数:pixmap,index(0から始まる数で何番目のImageLabelか指定、すでにある場合は置き換える。)
* 戻値:なし
* 概要:Pixmapを追加し、必要であればImagelabelを生成する。
*/
void TweetInfo::setImage ( const QPixmap &pixmap,const unsigned int index ) {
    unsigned int size = countImage();
    if ( size == 0 ) media_layout->itemAt ( 0 )->widget()->setVisible ( true );
    if ( size <= index ) {
        ImageLabel *iml = new ImageLabel ( 50,50,size );
        iml->setPixmap ( pixmap );
        iml->setFixedSize ( 50,50 );
        media_layout->addWidget ( iml );
    } else {
        QLayoutItem *item = media_layout->itemAt ( index + 2/*QLabel + addStretch分*/ );
        if ( item != nullptr ) ( qobject_cast<ImageLabel*> ( item->widget() ) )->setPixmap ( pixmap );
    }
    return;
}

/*
* 引数:index(0から始まる数で何番目のImageLabelか指定)
* 戻値:なし
* 概要:Pixmapを削除する。
*/
void TweetInfo::deleteImage ( const unsigned int index ) {
    if ( countImage() <= index ) return;
    QLayoutItem *old = media_layout->takeAt ( index + 2/*QLabel + addStretch分*/ ); //一番最初に追加したやつから番号が振られる。
    if ( old != nullptr ) delete old->widget();
    if ( countImage() == 0 ) media_layout->itemAt ( 0 )->widget()->setVisible ( false );
    return;
}

/*
* 引数:なし
* 戻値:なし
* 概要:すべてのPixmapを削除する。
*/
void TweetInfo::deleteImageAll() {
    unsigned int i;
    if ( ( i = countImage() ) )
        for ( i--;; i-- ) {
            deleteImage ( i ); //i==0のときも作業しないといけない
            if ( !i ) break;
        }
    return;
}

/*
* 引数:なし
* 戻値:なし
* 概要:Imagelabel(Pixmap)の数を返す。
*/
unsigned int TweetInfo::countImage() const {
    return media_layout->count() - 2;//QLabel + addStretch分
}

/*
 * 引数:なし
 * 戻値:対象のTweetData
 * 概要:引用するツイートのTweetDataを返す。
 */
TwitterJson::TweetData * TweetInfo::getQuoteTweet() {
    QLayoutItem *item = quote_layout->itemAt ( 1 );
    return ( item != nullptr ) ?qobject_cast<TweetContent*> ( item->widget() )->getTweetData() :nullptr;
}

/*
* 引数:対象のTweetData
* 戻値:なし
* 概要:ツイートの引用表示を行う。
*/
void TweetInfo::setQuoteTweet ( TweetContent *data ) {
    if ( data == nullptr ) return;
    if ( quote_layout->count() > 1 ) deleteQuoteTweet();
    data->setFrameShape ( QFrame::StyledPanel );
    data->setFrameShadow ( QFrame::Sunken );
    quote_layout->addWidget ( data );
    connect ( data,&TweetContent::action,this,&TweetInfo::deleteQuoteTweet ); //現在は削除以外ないのでこの実装
    quote_layout->itemAt ( 0 )->widget()->setVisible ( true );
    return;
}

/*
* 引数:対象のTweetData
* 戻値:なし
* 概要:ツイートの引用表示の削除を行う。
*/
void TweetInfo::deleteQuoteTweet() {
    QLayoutItem *old = quote_layout->takeAt ( 1 );
    if ( old != nullptr ) old->widget()->deleteLater();//TweetContentからのSignal時にクラッシュしないように
    quote_layout->itemAt ( 0 )->widget()->setVisible ( false );
    return;
}

/*
 * 引数:なし
 * 戻値:対象のTweetData
 * 概要:返信するツイートのTweetDataを返す。
 */
TwitterJson::TweetData * TweetInfo::getReplyTweet() {
    QLayoutItem *item = reply_layout->itemAt ( 1 );
    return ( item != nullptr ) ?qobject_cast<TweetContent*> ( item->widget() )->getTweetData() :nullptr;
}

/*
 * 引数:対象のTweetData
 * 戻値:なし
 * 概要:ツイートの返信表示を行う。
 */
void TweetInfo::setReplyTweet ( TweetContent *data ) {
    if ( data == nullptr ) return;
    if ( reply_layout->count() > 1 ) deleteReplyTweet();
    data->setFrameShape ( QFrame::StyledPanel );
    data->setFrameShadow ( QFrame::Sunken );
    reply_layout->addWidget ( data );
    connect ( data,&TweetContent::action,this,&TweetInfo::deleteReplyTweet ); //現在は削除以外ないのでこの実装
    reply_layout->itemAt ( 0 )->widget()->setVisible ( true );
    return;
}

/*
 * 引数:対象のTweetData
 * 戻値:なし
 * 概要:ツイートの返信表示の削除を行う。
 */
void TweetInfo::deleteReplyTweet() {
    QLayoutItem *old = reply_layout->takeAt ( 1 );
    if ( old != nullptr ) old->widget()->deleteLater();//TweetContentからのSignal時にクラッシュしないように
    reply_layout->itemAt ( 0 )->widget()->setVisible ( false );
    return;
}
