/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * TweetContent クラス
 * タイムラインのツイート一つ一つの(HTMLで言う)divみたいなもの
 */
#include "TweetContent.h"
#include "ImageLabel.h"
#include "TextLabel.h"
#include "ImageViewer.h"
#include "VideoPlayer.h"
#include "../Twitter/Twitter.h"
#include "../Twitter/TwitterJson.h"
#include "../Salmon.h"

#include <QNetworkReply>
#include <QtWidgets>

//Network TweetContent::net; staticだと終了時スレッドが残ってると怒られる(一番最後まで残るから?)

TweetContent::TweetContent ( TwitterJson::TweetData *_twdata,Mode _mode,QWidget *_root_widget/*ImageViewerなどに渡す*/,QWidget *parent, Qt::WindowFlags f )
    :QFrame ( parent,f ),mode ( _mode ),root_widget ( _root_widget ),popup ( nullptr ) {
    setTweetData ( _twdata );
    drawTweet();
    return;
}

TweetContent::~TweetContent() {
    delete twdata;
    return;
}

/*
 * 引数:twdata(新TweetData)
 * 戻値:なし
 * 概要:twdataを更新するときに使う。元のtwdataの削除はしないので呼び出し元でする。
 */
void TweetContent::setTweetData ( TwitterJson::TweetData *_twdata ) {
    twdata =_twdata;
    return;
};

/*
 * 引数:なし
 * 戻値:twdata(所持してるTweetData)
 * 概要:管理しているtwdataを返す。
 */
TwitterJson::TweetData *TweetContent::getTweetData() {
    return twdata;
}

/*
 * 引数:event
 * 戻値:なし
 * 概要:マウス操作がされたときに呼び出される。
 */
void TweetContent::mousePressEvent ( QMouseEvent* event ) {
    switch ( event->button() ) {
    case Qt::RightButton:
        if ( !popup ) {
            popup = new QMenu ( twdata->user_info.user_name,this ); //Title表示されない...
            createActions();
        }
        popup->popup ( event->globalPos() );
        break;
    default:
        event->ignore();
        return;
    }
    event->accept();
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ポップアップメニュー作成。
 */
void TweetContent::createActions() {

    if ( mode& Mode::Info ) {
        popup->addAction ( this->style()->standardIcon ( QStyle::SP_TitleBarCloseButton ) /*少し意図がずれてる気が*/,tr ( "Delete(&D)" ),this,&TweetContent::triggeredAction )->setData ( TwitterCore::Action::Destroy );
        return;//最低限の表示のみ
    }
    //リツイート
    popup->addSection ( ( twdata->text.size() >15 ) ?twdata->text.left ( 15 ).append ( "..." ) :twdata->text ); //使い方合ってるんかいな...
    popup->addAction ( QIcon ( ":/rt.png" ),tr ( "Retweet(&T)" ),this,&TweetContent::triggeredAction )->setData ( TwitterCore::Action::Retweet );

    //引用ツイート
    popup->addAction ( QIcon ( ":/rt.png" ),tr ( "Quote Tweet(&Q)" ),this,&TweetContent::triggeredAction )->setData ( TwitterCore::Action::QuoteTweet );

    //リプライ
    popup->addAction ( QIcon ( ":/rp.png" ),tr ( "Reply(&R)" ),this,&TweetContent::triggeredAction )->setData ( TwitterCore::Action::Reply );

    //お気に入り(今はいいねだが...)
    popup->addAction ( QIcon ( ":/fav.png" ),tr ( "Favorite(&F)" ),this,&TweetContent::triggeredAction )->setData ( TwitterCore::Action::Favorite );

    //削除(権限がある場合)
    if ( twdata->isMytweet() ) popup->addAction ( style()->standardIcon ( QStyle::SP_TitleBarCloseButton ) /*少し意図がずれてる気が*/,tr ( "Delete(&D)" ),this,&TweetContent::triggeredAction )->setData ( TwitterCore::Action::Destroy );

    //新しいウィンドウで開く(取っておきたいなど)
    popup->addAction ( style()->standardIcon ( QStyle::SP_TitleBarMaxButton ),tr ( "Open in new window(&W)" ),this,&TweetContent::openWindow );

    if ( twdata->url_info ) {
        popup->addSection ( tr ( "URL" ) );
        for ( int cnt = 0,size = twdata->url_info->url.size(); cnt < size; cnt++ ) {
            QAction *url_open_action = new QAction ( ( twdata->url_info->display_url[cnt].size() >15 ) ?twdata->url_info->display_url[cnt].left ( 15 ).append ( "..." ) :twdata->url_info->display_url[cnt],popup );
            url_open_action->setData ( cnt );
            connect ( url_open_action,&QAction::triggered,this,&TweetContent::openUrl );
            popup->addAction ( url_open_action );
        }
    }
    popup->addSection ( tr ( "クライアント" ) );
    QString via_text ( twdata->via );
    via_text.replace ( QRegExp ( "<a href=\"([^\"]*)\"[^>]*>(.*)</a>" ),"\\1\n\\2" );
    popup->addAction ( via_text.section ( '\n',1,1 ),this,[this] {QDesktopServices::openUrl ( QUrl ( ( ( qobject_cast<QAction*> ( sender() ) )->data().toString() ) ) );} )->setData ( via_text.section ( '\n',0,0 ) );
    return;
}

/*
* 引数:なし
* 戻値:なし
* 概要:各種アクションが押されたときに呼ばれる。twdataとActionとともにactionシグナルを呼ぶだけ。
* 備考:各種設定をしてQtでenumを認識できるようにしてもdata().vaiue<TwitterCore::Action>で取り出せないので諦めてuintでわたす。
*/
void TweetContent::triggeredAction() {
    return emit action ( twdata, ( qobject_cast<QAction*> ( sender() ) )->data().toUInt() );
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:クリックされたURLをブラウザで開く。
 */
void TweetContent::openUrl() {
    int cnt = qobject_cast<QAction*> ( sender() )->data().toInt();
    if ( cnt < twdata->url_info->display_url.size() )
        QDesktopServices::openUrl ( QUrl ( twdata->url_info->url[cnt] ) );
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:TweetDataを使ってツイートを表示する
 */
void TweetContent::drawTweet() {
    if ( twdata == nullptr ) return;
    QHBoxLayout *main_box = new QHBoxLayout;
    QVBoxLayout *text_box = new QVBoxLayout;

    text_box->setSpacing ( 0 );
    ImageLabel *icon = new ImageLabel ( 40,40 );
    icon->setFixedSize ( 40,40 );
    if ( !icon->setPixmapByName ( twdata->getOriginalUserInfo().user_icon_url ) ) { //アイコンのキャッシュがない
        connect ( net.get ( twdata->getOriginalUserInfo().user_icon_url ),&QNetworkReply::finished,icon,&ImageLabel::setPixmapByNetwork );
    }
    main_box->addWidget ( icon,0,Qt::AlignTop );

    if ( twdata->flag&1 ) { //RT
        QLabel *retweeted_user_name = new QLabel ( twdata->user_info.user_name + tr ( " retweeted" ) );
        retweeted_user_name->setStyleSheet ( "font-size:10px;color:lime;" ); //small指定ができない
        retweeted_user_name->setWordWrap ( true );
        text_box->addWidget ( retweeted_user_name );
    }

    QLabel  *user_name = new QLabel ( twdata->getOriginalUserInfo().user_name );
    user_name->setStyleSheet ( "font-weight:bold;color:white;" );
    user_name->setWordWrap ( true );
    text_box->addWidget ( user_name );
    QLabel  *screen_name = new QLabel ( '@' + twdata->getOriginalUserInfo().screen_name );
    screen_name->setStyleSheet ( "font-size:10px;color:gray;" );
    screen_name->setWordWrap ( true );
    text_box->addWidget ( screen_name );

    if ( twdata->flag&0x30 ) { //なんか微妙な実装だが...(２パターンに分離して処理するのをやめた。ところで鍵で確認済みユーザはいるのかな)
        user_name->setText ( user_name->text() /*.toHtmlEscaped()*/ + QString ( ( ( twdata->flag&0x10 ) /*鍵アカウント*/?"<img src=\":/protected.png\" />":"" ) ) + ( ( twdata->flag&0x20 ) /*確認済みアカウント*/?"<img src=\":/verified.png\" />":"" ) );
        user_name->setTextFormat ( Qt::RichText );
    }

    text_box->addWidget ( new TextLabel ( twdata->text ) );

    if ( twdata->media_info && ! ( mode & Mode::Info ) ) {
        if ( !twdata->media_info->video_url.isEmpty() ) { //動画

            ImageLabel   *iml = new ImageLabel ( 50,50,0,this );
            if ( !iml->setPixmapByName ( twdata->media_info->direct_links.at ( 0 ) ) ) { //キャッシュなし
                if ( twdata->flag&0x40 ) iml->setPixmap ( style()->standardIcon ( QStyle::SP_MessageBoxWarning ).pixmap ( 100,45 ) );
                else connect ( net.get ( twdata->media_info->direct_links.at ( 0 ) + ":thumb" ),&QNetworkReply::finished,iml,&ImageLabel::setPixmapByNetwork );
            }
            connect ( iml,&ImageLabel::clicked,this,&TweetContent::showPicture );
            iml->setStyleSheet ( "border:3px solid blue;" ); //動画かどうかの判別(Beta)
            text_box->addWidget ( iml );
        } else { //画像
            QScrollArea *media_box = new QScrollArea;
            media_box->setWidgetResizable ( true );
            QWidget *center = new QWidget;
            QHBoxLayout *media_layout = new QHBoxLayout ( center );
            media_box->setWidget ( center );

            for ( int cnt =0; twdata->media_info->direct_links.size() > cnt ; cnt++ ) { //範囲forにカウンターがついたらな...
                ImageLabel   *iml = new ImageLabel ( 100,45/*50にするとMediaBoxにY軸スクロールバーがついて気持ち悪い*/,cnt,this );
                if ( !iml->setPixmapByName ( twdata->media_info->direct_links.at ( cnt ) ) ) { //キャッシュなし
                    if ( twdata->flag&0x40 ) iml->setPixmap ( style()->standardIcon ( QStyle::SP_MessageBoxWarning ).pixmap ( 100,45 ) );
                    else connect ( net.get ( twdata->media_info->direct_links.at ( cnt ) + ":thumb" ),&QNetworkReply::finished,iml,&ImageLabel::setPixmapByNetwork );
                }
                connect ( iml,&ImageLabel::clicked,this,&TweetContent::showPicture );
                media_layout->addWidget ( iml );
            }
            text_box->addWidget ( media_box );
        }
    }

    //引用ツイート
    if ( twdata->quoted_status && ! ( mode & Mode::Info ) && ! ( mode & Mode::Simple ) ) {
        TweetContent *quote = new TweetContent ( new TwitterJson::TweetData ( *twdata->quoted_status ),Mode::Simple,root_widget );
        connect ( quote,&TweetContent::action,this,&TweetContent::transferAction );
        quote->setFrameShape ( QFrame::StyledPanel );
        quote->setFrameShadow ( QFrame::Sunken );
        text_box->addWidget ( quote );
    }
    //その他情報
    QLabel *info_text = new QLabel ( twdata->date.toString ( Qt::SystemLocaleShortDate ) );
    info_text->setWordWrap ( true );
    info_text->setStyleSheet ( "font-size:10px;color:gray;" );
    text_box->addWidget ( info_text,0,Qt::AlignRight );
    main_box->addLayout ( text_box );
    setLayout ( main_box );
}

/*
 * 引数:url(表示する画像のURL)
 * 戻値:なし
 * 概要:大きい画像を表示する。または動画を再生する。
 */
void TweetContent::showPicture ( TwitterJson::TweetData *twdata,unsigned int index ) {
    if ( twdata == nullptr || twdata->media_info == nullptr ) return;
    if ( twdata->flag & 0x40 && QMessageBox::question ( root_widget,APP_NAME,tr ( "この画像・動画を表示すると気分を害する可能性があります。表示しますか。" ),QMessageBox::Yes | QMessageBox::No,QMessageBox::No ) != QMessageBox::Yes ) return;
    if ( !twdata->media_info->video_url.isEmpty() ) { //動画
        VideoPlayer *player = new VideoPlayer ( twdata,root_widget,Qt::Window );
        player->show();
    } else { //画像
        ImageViewer *viewer = new ImageViewer ( twdata,index,root_widget,Qt::Window );
        viewer->show();
    }
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:自分をコピーして新たなウィンドウとして開く
 */
void TweetContent::openWindow() {
    if ( twdata == nullptr ) return;
    TweetContent *window = new TweetContent ( new TwitterJson::TweetData ( *twdata ),mode,root_widget,root_widget,Qt::Window );
    QPalette Palette = window->palette();
    window->setAttribute ( Qt::WA_DeleteOnClose );
    window->setWindowTitle ( tr ( "ツイートの詳細 " ) + APP_NAME );
    Palette.setColor ( QPalette::Window, Qt::black ); //背景を黒く
    Palette.setColor ( QPalette::WindowText,Qt::white );
    window->setAutoFillBackground ( true );
    window->setPalette ( Palette );
    window->resize ( sizeHint() );
    window->show();
    return;
}

/*
 * 引数:ori(root_widgetに手渡すTweetData、act(選択された操作)
 * 戻値:なし
 * 概要:root_widgetにactionシグナルを転送する。もっといい方法ないかな。
 */
void TweetContent::transferAction ( TwitterJson::TweetData* ori, unsigned int act ) {
    return emit action ( ori,act );
}
