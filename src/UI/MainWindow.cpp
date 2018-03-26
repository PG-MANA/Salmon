/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * メインウィンドウ クラス
 * 主役である画面の表示を行う。
 */
#include "MainWindow.h"
#include "ImageLabel.h"
#include "TweetContent.h"
#include "TweetInfo.h"
#include "ImageViewer.h"
#include "VideoPlayer.h"
#include "TextLabel.h"
#include "../Twitter/Twitter.h"
#include "../Twitter/Streamer.h"
#include "../Twitter/TwitterJson.h"
#include "../Twitter/TwitterUrl.h"
#include "../Twitter/TwitterSetting.h"
#include "../Twitter/MediaUpload.h"
#include "../Salmon.h"
#include <QtWidgets>
#include <QNetworkReply>



//MEMO:trは翻訳する可能性のあるものに使う。
//MEMO:mocのincludeはソースでQObjectの子クラスを作る場合に必要らしい
//MEMO:MainWindowを閉じたときにQApplication::closeAllWindowsを呼ばないのはMainWindowを複数出せるようにする計画が少なからずあるから。ただしメニューの終了は除く。
//MEMO:ウィジットはヒープ上に作らないと破棄時にバグる
MainWindow::MainWindow() :QMainWindow() {
    //ウィンドウ準備
    QWidget *center = new QWidget;//センターウィジット
    main_layout = new QVBoxLayout ( center ); //メインレイアウト
    QPalette Palette = center->palette();
    Palette.setColor ( QPalette::Window, Qt::black ); //背景を黒く
    Palette.setColor ( QPalette::WindowText,Qt::white );
    center->setAutoFillBackground ( true );
    center->setPalette ( Palette );
    setCentralWidget ( center );

    //メニュー
    createMenus();
    //TimeLine
    createTimeLine();
    //ツイートボックス
    createTweetBox();
    //通知
    tray_info = new QSystemTrayIcon ( qApp->windowIcon(),this );

    //Stream APIのスレッド
    timeline_streamer = new Streamer;//親がいるとスレッドが動かせない。
    timeline_thread = new QThread ( this );
    timeline_streamer->moveToThread ( timeline_thread ); //動作スレッドを移動する。
    timeline_thread->start();
    //connect祭り
    connect ( timeline_streamer,&Streamer::newTweet,this,&MainWindow::showTweet );
    connect ( timeline_streamer,&Streamer::deleteTweet,this,&MainWindow::removeTweet );
    connect ( timeline_streamer,&Streamer::newNotification,this,&MainWindow::showNotification );
    connect ( timeline_streamer,&Streamer::abort,this,&MainWindow::abortedTimeLine );
    connect ( timeline_thread,&QThread::finished,timeline_streamer,&QObject::deleteLater ); //意外と重要
    return;
}

MainWindow::~MainWindow() {
    timeline_thread->quit();//timeline_streamerはdeleteLaterにより削除される。
    timeline_thread->wait();//呼ばれても動かないようにする。
    delete twitter;
    delete twset;
    return;
}

/*
 * 引数:setting_file(設定ファイルのパス)
 * 戻値:成功はtrue、失敗はfalse
 * 概要:渡された設定ファイルから設定を読み込み、基本設定を行う。初期化のあとに呼ぶ。
 */
bool MainWindow::init ( const char *setting_file ) {
    //設定読み込み
    twset = new TwitterSetting ( setting_file );
    restoreGeometry ( twset->geometry() );

    if ( !twset->isEmpty() ) {
        twitter = new Twitter ( twset );
    } else {
        //初期設定
        twitter = nullptr;
        try {
            ( QMessageBox::question ( this,APP_NAME,tr ( "Twitterアプリ連携の認証を行いますか。" ),QMessageBox::Yes | QMessageBox::No,QMessageBox::Yes ) == QMessageBox::Yes ) ?authorize_twitter () /*エラーはthrowされて下に行く*/:throw tr ( "Twitterアプリ連携を行わなければこのソフトウェアは使用できません。" );
        } catch ( QString &error ) {
            //delete twitter;<=~MainWindowで呼ばれる。
            QMessageBox::critical ( this,APP_NAME,error );
            return false;
        }
    }

    //タイトル
    setWindowTitle ( ( *twset->user_name ) ? ( APP_NAME " at " + QString ( twset->user_name ) ) : APP_NAME_LONG );
    QMetaObject::invokeMethod ( timeline_streamer,"setTwitter",   Qt::BlockingQueuedConnection,QGenericReturnArgument ( nullptr ),Q_ARG ( const TwitterSetting*,twset ) ); //別スレッドでTwitterクラスを作らないといろいろ怒られる。
    return true;
}


/*
 * 引数:なし
 * 戻値:なし
 * 概要:メニューバーを作って設定する。ウィンドウ初期化用。
 */
void MainWindow::createMenus() {
    menuBar()->setNativeMenuBar ( true );

    //設定
    QMenu *setting_menu = menuBar()->addMenu ( tr ( "設定(&S)" ) );
    setting_menu->setToolTipsVisible ( true );
    stream_status = setting_menu->addAction ( style()->standardIcon ( QStyle::SP_BrowserReload ), tr ( "ストリーム接続(&S)" ),this,&MainWindow::changeStatusStream );//アイコンの意図が違っていて微妙
    stream_status->setCheckable ( true );
    stream_status->setChecked ( true );
    stream_status->setToolTip ( tr ( "チェックされるとストリームに接続し、外されると切断します。" ) );
    setting_menu->addAction ( style()->standardIcon ( QStyle::SP_TitleBarCloseButton ),tr ( "終了(&E)" ),qApp,&QApplication::closeAllWindows )->setToolTip ( tr ( "すべてのウィンドウを閉じ、アプリケーションを終了します。" ) );

    //表示
    /*QMenu *timeline_menu = menuBar()->addMenu ( tr ( "表示(&V)" ) );
    timeline_menu->setToolTipsVisible ( true );
    timeline_menu->addAction ( tr ( "ホーム(&H)" ),this,[] {} );
    timeline_menu->addAction ( tr ( "通知(&H)" ),this,[] {} );
    list_menu = timeline_menu->addMenu ( tr ( "リスト(&L)" ) );*/

    //ウィンドウ
    QMenu *window_menu = menuBar()->addMenu ( tr ( "ウィンドウ(&W)" ) );
    window_menu->setToolTipsVisible ( true );
    QAction *always_top_action = window_menu->addAction ( style()->standardIcon ( QStyle::SP_ArrowUp/*QStyle::SP_TitleBarShadeButton*/ ),tr ( "常に最前面に表示(&A)" ),this,&MainWindow::setAlwayTop );
    always_top_action->setToolTip ( tr ( "常にこのウィンドウを手前に表示します。(ウィンドウマネージャで設定できる場合はそちらで設定してください。)" ) );
    always_top_action->setCheckable ( true );

    //ヘルプ
    QMenu *help_menu = menuBar()->addMenu ( tr ( "ヘルプ(&H)" ) );
    help_menu->setToolTipsVisible ( true );
    help_menu->addAction ( QIcon ( ":/icon-normal.png" ),tr ( APP_NAME "について(&A)"/*分離すると翻訳しづらそう*/ ),this,&MainWindow::showAbout )->setToolTip ( tr ( "バージョンやライセンスについてのダイアログを表示します。" ) );
    help_menu->addAction ( style()->standardIcon ( QStyle::SP_TitleBarMenuButton ),tr ( "Qtについて(&Q)" ),qApp,&QApplication::aboutQt )->setToolTip ( tr ( "使用されているQtのライブラリのバージョンやライセンスについてのダイアログを表示します。" ) );

    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:タイムライン(ツイートを並べていくところ)を作って設定する。ウィンドウ初期化用。
 */
void MainWindow::createTimeLine() {
    QScrollArea *timeline_area = new QScrollArea;
    QPalette palette = timeline_area->palette();
    palette.setColor ( QPalette::Window, Qt::black );
    timeline_area->setPalette ( palette );
    timeline_area->setFrameShape ( QFrame::NoFrame ); //枠線をなくす
    //timeline_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//表示しない
    timeline_area->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOn ); //常に表示
    timeline_area->setWidgetResizable ( true ); //先にこれを設定する。

    //ツイートを並べていくスクロールエリア
    QWidget *timeline_center = new QWidget;
    timeline_layout = new QBoxLayout ( QBoxLayout::BottomToTop,timeline_center ); //下から上
    timeline_area->setWidget ( timeline_center );
    main_layout->addWidget ( timeline_area );
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ツイートボックス(入力欄とボタンなど)を作って設定する。ウィンドウ初期化用。
 */
void MainWindow::createTweetBox() {
    //入力ボックス
    tweet_editer = new QPlainTextEdit;//HTMLが扱えないようにする
    tweet_editer->setFixedHeight ( 100 ); //Editボックスのサイズ固定(サイズは適当)
    tweet_editer->installEventFilter ( this );
    main_layout->addWidget ( tweet_editer );

    //色々ツイートに関する情報を出すためのスクロールエリア
    info_scroll_area = new QScrollArea;
    info_scroll_area->setFixedHeight ( 100 ); //サイズ固定(サイズは適当)
    info_scroll_area->setWidgetResizable ( true );
    tweet_info = new TweetInfo ( this );
    info_scroll_area->setWidget ( tweet_info ); //ここの順序大切
    info_scroll_area->setVisible ( false );
    main_layout->addWidget ( info_scroll_area );

    //ツイートボタンボックス
    QHBoxLayout *tweet_button_layout = new QHBoxLayout;//将来ボタンを増やした時のために
    tweet_button_layout->addStretch();

    //Home Time Line更新
    QPushButton *ReloadButton = new QPushButton;
    ReloadButton->setIcon ( style()->standardIcon ( QStyle::SP_BrowserReload ) );
    ReloadButton->setToolTip ( tr ( "タイムラインを更新します。" ) );
    ReloadButton->setStyleSheet ( "background-color: #255080;" );
    connect ( ReloadButton,&QPushButton::clicked,this,&MainWindow::updateTimeLine );
    tweet_button_layout->addWidget ( ReloadButton );

    //メディア追加ボタン
    QPushButton *MediaButton = new QPushButton;
    MediaButton->setIcon ( QIcon ( ":/add.png" ) );
    MediaButton->setToolTip ( tr ( "写真・動画を追加します。" ) );
    MediaButton->setStyleSheet ( "background-color: #255080;" );
    connect ( MediaButton,&QPushButton::clicked,this,&MainWindow::addMedia );
    tweet_button_layout->addWidget ( MediaButton );

    //ツイート送信ボタン
    tweet_button = new QPushButton;
    tweet_button->setIcon ( QIcon ( ":/send.png" ) );
    tweet_button->setStyleSheet ( "background-color: #255080;" );
    tweet_button->setToolTip ( tr ( "ツイートを送信します。" ) );
    connect ( tweet_button,&QPushButton::clicked,this,&MainWindow::tweet );
    tweet_button_layout->addWidget ( tweet_button );

    //main_layoutにツイートボタンボックスを追加
    main_layout->addLayout ( tweet_button_layout );
    return;
}


/*
 * 引数:twset(結果格納)
 * 戻値:なし
 * 概要:Twitterアプリ連携の認証を行う。それ以外では呼ばない。同期処理を行う。
 */
void MainWindow::authorize_twitter ( ) {
    /*throwしまくりである...(throw時のtwitterの削除は他でするので気にしなくてよい*/
    QEventLoop event;//ここだけ同期処理
    QNetworkReply *rep;
    QList<QByteArray> result;
    QByteArray token_key,data,pincode;

    twitter = new Twitter;
    rep = twitter->request_token();
    connect ( rep,&QNetworkReply::finished,&event,&QEventLoop::quit );
    event.exec();
    if ( rep->error() != QNetworkReply::NoError ) {
        delete rep;
        throw tr ( "リクエストトークンの取得に失敗しました。" );
    }
    result = rep->readAll().split ( '&' );
    delete rep;

    token_key = result[0].split ( '=' ) [1];
    if ( !QDesktopServices::openUrl ( twitter->authorize_url ( token_key ) ) ) throw tr ( "ブラウザの起動に失敗しました。" );
    pincode = pin_dialog();//PIN入力
    if ( pincode.isEmpty() ) throw tr ( "認証がキャンセルされました。" );
    rep = twitter->access_token ( token_key.constData(),result[1].split ( '=' ) [1].constData(),pincode.constData() );
    connect ( rep,&QNetworkReply::finished,&event,&QEventLoop::quit );
    event.exec();
    if ( rep->error() != QNetworkReply::NoError ) {
        delete rep;
        throw tr ( "アクセストークンの取得に失敗しました。" );
    }
    data = rep->readAll();
    delete rep;
    twitter->decode_access_token ( data,*twset );
    if ( !twset->save( ) ) throw tr ( "設定の保存に失敗しました。" );
    return;
}

/*
 * 引数:なし
 * 戻値:pin(入力された文字列)
 * 概要:PIN入力ダイアログを作って表示して結果を返す。
 */
QByteArray MainWindow::pin_dialog() {
    QDialog dialog;
    QVBoxLayout *layout = new QVBoxLayout ( &dialog );
    QLineEdit *pincode_editer = new QLineEdit;
    QPushButton *ok_button = new QPushButton ( tr ( "OK" ) );

    connect ( ok_button,&QPushButton::clicked,&dialog,&QWidget::close );
    pincode_editer->setMaxLength ( 7 ); //最大7文字
    layout->addWidget ( new QLabel ( tr ( "表示されたブラウザでTwitterの認証して、表示されたPINコードを入力してください。" ) ) );
    layout->addWidget ( pincode_editer );
    layout->addWidget ( ok_button );
    dialog.exec();
    return pincode_editer->text().toLatin1();
}

/*
 * 引数:error(エラー番号)
 * 戻値:なし
 * 概要:Streamerでエラーが起きた時に呼ばれる。再接続するか確認する。
 */
void MainWindow::abortedTimeLine ( unsigned int error ) {
    QMessageBox mes_box;

    mes_box.setWindowTitle ( APP_NAME );
    mes_box.setIcon ( QMessageBox::Critical );
    stream_status->setChecked ( false );

    switch ( static_cast<TwitterCore::Error> ( error ) ) {
    case TwitterCore::CannotConnect:
        mes_box.setText ( tr ( "タイムラインに接続できませんでした。再接続しますか。" ) );
        mes_box.setStandardButtons ( QMessageBox::Yes|QMessageBox::No );
        break;
    case TwitterCore::NetworkError:
        mes_box.setText ( tr ( "タイムラインから切断されました。再接続しますか。" ) );
        mes_box.setStandardButtons ( QMessageBox::Yes|QMessageBox::No );
        break;
    case TwitterCore::BadPointer:
        mes_box.setText ( tr ( "メモリアクセス違反が発生しました。" ) );
        break;
    default:
        mes_box.setText ( tr ( "不明なエラーが発生しました。" ) );
    }
    if ( mes_box.exec() == QMessageBox::Yes ) {
        stream_status->setChecked ( true );
        QMetaObject::invokeMethod ( timeline_streamer,"startUserStream",Qt::QueuedConnection );
    }
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:メニューでストリームから切断がクリックされた時呼ばれる。
 */
void MainWindow::changeStatusStream ( bool checked ) {
    //この処理ではいつか問題が生じるのでもっと細かく制御すべき
    if ( checked ) QMetaObject::invokeMethod ( timeline_streamer,"startUserStream",Qt::QueuedConnection );
    else timeline_streamer->stopUserStream();
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:メインウィンドウを表示する時呼ばれる。home_timelineの取得を行う。
 */
void MainWindow::show() {
    QMainWindow::show();
    connect ( twitter->home_timeline(),&QNetworkReply::finished,this,&MainWindow::showTimeLine );
    //connect ( twitter->get_lists(),&QNetworkReply::finished,this,&MainWindow::setListsMenu );
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ウィンドウ表示の際、メニューの表示=>リストに所持してるリストを設定する。
 */
void MainWindow::setListsMenu() {
    QNetworkReply *rep = qobject_cast<QNetworkReply*> ( sender() );
    auto result = TwitterJson::getListInfo ( QJsonDocument::fromJson ( rep->readAll() ).array() );
    for ( int cnt = 0,len = result.size(); cnt < len; cnt++ ) {
        list_menu->addAction ( result[cnt].second,this,[] {/*Temporary*/} )->setData ( result[cnt].first );
    }
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReplyによって呼ばれる。home_timelineの処理を行う。
 */
void MainWindow::showTimeLine() {
    QNetworkReply *rep = qobject_cast<QNetworkReply*> ( sender() );
    if ( rep->error() == QNetworkReply::NoError ) {
        QJsonArray tweets = QJsonDocument::fromJson ( rep->readAll() ).array();

        for ( int i = tweets.size() - 1; i >= 0  ; i-- ) {
            QJsonObject obj = tweets[i].toObject();
            TwitterJson::TweetData *twdata = new TwitterJson::TweetData ( obj,twitter->getUserId() );
            if ( !twdata->isEmpty() ) showTweet ( twdata );
        }
    }
    rep->deleteLater();
    if ( stream_status->isChecked() ) QMetaObject::invokeMethod ( timeline_streamer,"startUserStream",Qt::QueuedConnection ); //ストリームスタート
#if ENABLE_NEW_STREAM
    tray_info->show();
     tray_info->showMessage ( APP_NAME,tr("現在UserStreamの代替としてFilterStreamを使用しています。これにより次のような仕様となっています。\n・鍵アカウントのツイートは取得できません。\n・フォロー通知などを受信できません。\n・フォローしていないアカウントへの返信を表示します。\n・5,000人以上フォローしているアカウントはすべてのツイートを表示できません。\n・全体的な動作が重くなります。") );
     tray_info->hide();
#endif
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ホームタイムラインの更新を行う。
 *  備考:15/15min、つまり平均１分に1回。
 */
void MainWindow::updateTimeLine() {
    if ( const int count = timeline_layout->count(); count > 0 ) {
        connect ( twitter->home_timeline ( ( qobject_cast<TweetContent *> ( timeline_layout->itemAt ( count - 1 )->widget() ) )->getTweetData()->id ),&QNetworkReply::finished,this,&MainWindow::showTimeLine );
    }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ツイートに画像・動画を追加する。動画は未対応。
 */
void MainWindow::addMedia() {
    try {
        QString &&file_path = QFileDialog::getOpenFileName ( this,tr ( "メディア追加" ),"",tr ( "メディア (*.png *.jpg *.gif *.bmp *.pbm *.pgm *.ppm *.xbm *.xpm *.svg)" ) ); //対応拡張子はImageViewer参考にして動的に作るべき
        if ( file_path.isEmpty() ) return;
        QFileInfo file_info ( file_path );
        //動画は分岐させる
        QByteArrayList && supported_formats = QImageReader::supportedImageFormats();
        QByteArray &&suffix = file_info.suffix().toLower().toUtf8();//ローマ字以外の拡張子ってあるんやろか...
        for ( const QByteArray &e : supported_formats ) {
            if ( e == suffix ) break;
            if ( e == supported_formats.last() ) throw tr ( "サポートしていない画像です。" );
        }
        if ( file_info.size() >= 5 * 1024 * 1024 ) throw tr ( "ファイルサイズが大きすぎます。" );
        unsigned int counter = tweet_info->countImage();
        if ( counter > 4 ) throw tr ( "４枚以上の画像は投稿できません。" );
        //tweet_infoに追加
        tweet_info->setImage ( QPixmap ( file_path ),counter );
        info_scroll_area->setVisible ( true );
    } catch ( QString &e ) {
        QMessageBox::critical ( this,APP_NAME,e );
    }
    return;
}

/*
 * 引数:なし
 * 戻値:クリップボードから設定できたか。
 * 概要:画像をクリップボードから設定する。tweet_editerの右クリックメニューで設定できたら面白そう。
 */
bool MainWindow::addMediaByClipboard() {
    QPixmap &&pic = QApplication::clipboard()->pixmap();
    if ( pic.isNull() ) return false;

    unsigned int counter = tweet_info->countImage();
    if ( counter > 4 ) {
        QMessageBox::critical ( this,APP_NAME,tr ( "４枚以上の画像は投稿できません。" ) );
        return false;
    }
    //tweet_infoに追加
    tweet_info->setImage ( pic,counter );
    info_scroll_area->setVisible ( true );
    return true;
}

/*
 * 引数:checked(メニューがチェックされてるかどうか)
 * 戻値:なし
 * 概要:Windowを最前面に表示するかどうか。ウィンドウマネージャで設定できるならそっちのほうが適切。
 */
void MainWindow::setAlwayTop ( bool checked ) {
    setWindowFlags ( ( checked ) ?/*0にしたら型が違うですって...キャストはめんどい*/Qt::WindowStaysOnTopHint : windowFlags() ^Qt::WindowStaysOnTopHint );
    QMainWindow::show();
    return;
}

/*
 * 引数:twdata(新規表示するツイート)
 * 戻値:なし
 * 概要:主にStreamerより呼ばれる。
 */
void MainWindow::showTweet ( TwitterJson::TweetData *twdata ) {
    if ( twdata == nullptr ) return;
    TweetContent *content = new TweetContent ( twdata,TweetContent::Normal,this ); //クリックの検出などをするため(Layoutを直接噛ませるのとどちらがいいんだろう)

    connect ( content,&TweetContent::action,this,&MainWindow::contentAction );

    //古いものを削除
    if ( timeline_layout->count() >= MAX_TWDATA ) {
        QLayoutItem *old = timeline_layout->takeAt ( 0 ); //一番最初に追加したやつから番号が振られる。
        if ( old ) delete old->widget();
    }

    //追加
    timeline_layout->addWidget ( content );
    return;
}

/*
 * 引数:Id(削除されたツイートのid)
 * 戻値:なし
 * 概要:主にStreamerのDeletetweetシグナルによって呼ばれる。削除されたツイートのTweetContentを消す。
 */
void MainWindow::removeTweet ( const QString &id ) { //実際隠すんじゃなくて消すけどね...
    for ( unsigned int c = timeline_layout->count() - 1; c; c-- ) { //地道に探す(遅そう)
        QLayoutItem *item = timeline_layout->itemAt ( c );
        if ( item == nullptr ) continue;
        TwitterJson::TweetData *twdata = ( qobject_cast<TweetContent *> ( item->widget() ) )->getTweetData();
        if ( twdata == nullptr ) continue;
        if ( twdata->id == id || ( twdata->retweeted_status && twdata->retweeted_status->id == id ) ) {
            item = timeline_layout->takeAt ( c ); //takeAtを一回呼ぶ
            if ( item != nullptr ) delete item->widget();
        }
    }
}

/*
 * 引数:twdata(新規表示するツイート)
 * 戻値:なし
 * 概要:主にStreamerより呼ばれる。NotificationDataを使って通知を表示する。
 */
void MainWindow::showNotification ( TwitterJson::NotificationData *nfdata ) {
    if ( nfdata == nullptr ) return;

    QString message;
    switch ( nfdata->event ) {
    case TwitterJson::Event::Favorite:
        message = nfdata->source.user_name + tr ( "さんが" ) + nfdata->target.user_name + tr ( "さんのツイートをお気に入りに登録しました。\n" ) + ( ( nfdata->target_object_tweet ) ?
                  nfdata->target_object_tweet->text.left ( 30 ) :tr ( "不明" ) );
        break;
    case TwitterJson::Event::Unfavorite:
        message = nfdata->source.user_name + tr ( "さんが" ) + nfdata->target.user_name + tr ( "さんのツイートのお気に入りの登録を解除しました。\n" ) + ( ( nfdata->target_object_tweet ) ?
                  nfdata->target_object_tweet->text.left ( 30 ) :tr ( "不明" ) );
        break;
    case TwitterJson::Event::QuotedTweet:
        if ( nfdata->source.id == twitter->getUserId() ) return delete nfdata;
        message = nfdata->source.user_name + tr ( "さんがあなたのツイートを引用ツイートしました。\n" ) + ( ( nfdata->target_object_tweet ) ?
                  nfdata->target_object_tweet->text.left ( 30 ) :tr ( "不明" ) );
        break;
    case TwitterJson::Event::Follow:
        message = nfdata->source.user_name + tr ( "さんが" ) + nfdata->target.user_name + tr ( "さんをフォローしました。" );
        break;
    case TwitterJson::Event::Unfollow:
        message = nfdata->target.user_name + tr ( "さんをフォロー解除しました。" ); //自分がアンフォローされたときは流れてこない。
        break;
    default:
        return delete nfdata;//未対応
    }
    tray_info->show();
#if(QT_VERSION >= QT_VERSION_CHECK(5,9,0))
    //もしアイコンが取得済みならアイコンを表示(ネットから取ってくるのには手間がかかる)(beta)
    QIcon icon;
    ImageLabel icon_search;

    if ( icon_search.setPixmapByName ( nfdata->source.user_icon_url ) ) { //nullアクセスはないはず
        if ( const QPixmap *icon_pixmap = icon_search.pixmap() ) icon.addPixmap ( *icon_pixmap );
    } else {
        icon = qApp->windowIcon();
    }
    tray_info->showMessage ( APP_NAME,message,icon ); //イベントによって変えるとよさげ
#else
    tray_info->showMessage ( APP_NAME,message );
#endif
    tray_info->hide();//これで一応メッセージだけ出る。
    return delete nfdata;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ツイートが完了した時に呼ばれる。ツイートボックスの内容の削除やツイートボタンの有効化を行う。
 */
void MainWindow::finishedTweet() {
    QNetworkReply *rep = qobject_cast<QNetworkReply*> ( sender() );
    if ( rep->error() != QNetworkReply::NoError ) {
        QMessageBox::critical ( this,tr ( "ツイートに失敗しました " ) + APP_NAME,rep->errorString() );
    } else {
        tweet_editer->clear();
        tweet_info->deleteImageAll();
        tweet_info->deleteQuoteTweet();
        tweet_info->deleteReplyTweet();
        info_scroll_area->setVisible ( false );
    }
    setEnabledTweet ( true );
    rep->deleteLater();//成功時、本来は読むべきだが...特に読む必要もないので静かに閉じる
    return;
}

/*
 * 引数:bool(ツイート入力を受け付けるかどうか)
 * 戻値:なし
 * 概要:ツイートボックスの有効化やツイートボタンの有効化を行う。
 */
void MainWindow::setEnabledTweet ( bool enable ) {
    tweet_button->setEnabled ( enable );
    tweet_editer->setEnabled ( enable );
    if ( enable ) tweet_editer->setFocus(); //こうしとくと便利
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:後処理の必要ない作業が完了したら呼ばれる。
 */
void MainWindow::finishedRequest() {
    QNetworkReply *rep = qobject_cast<QNetworkReply*> ( sender() );
    if ( rep->error() != QNetworkReply::NoError ) {
        QMessageBox::critical ( this,tr ( "作業に失敗しました " ) + APP_NAME,rep->errorString() );
    }
    rep->deleteLater();
    return;
}

/*
 * 引数:qkey
 * 戻値:なし
 * 概要:キーが押されたときに呼ばれる。ショートカットキーを拾う。
 */
void MainWindow::keyPressEvent ( QKeyEvent *qkey ) {
    qkey->ignore();
    if ( qkey->modifiers().testFlag ( Qt::ControlModifier ) ) {
        switch ( qkey->key() ) {
        case Qt::Key_Return:
            tweet();//Ctrl + Enter
            break;
        case Qt::Key_V:
            if ( !addMediaByClipboard() ) return;
            break;
        default:
            return;
        }
        qkey->accept();
    }
    return;
}

/*
 * 引数:obj(発生源),event(イベントの種類)
 * 戻値:処理したかどうか
 * 概要:installEventFilterしたオブジェクトのイベントが流れてくる。
 */
bool MainWindow::eventFilter ( QObject *obj,QEvent *event ) {
    event->ignore();
    if ( obj == tweet_editer ) {
        switch ( event->type() ) {
        case QEvent::KeyPress:
            keyPressEvent ( static_cast<QKeyEvent*> ( event ) );
            break;
        default://警告回避
            ;
        }
    }
    return event->isAccepted();//意味ない気も..
}

/*
 * 引数:event
 * 戻値:なし
 * 概要:ウィンドウを閉じる時に呼ばれる。ウィンドウ位置の保存を行う。
 */
void MainWindow::closeEvent ( QCloseEvent *event ) {
    //ウィンドウ状態保存
    twset->setGeometry ( saveGeometry() );
    QMainWindow::closeEvent ( event );
    tray_info->hide();
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:メインウィンドウ下のツイートボックスを使ってツイートするときに呼ぶ。ツイート作業を行う。
 */
void MainWindow::tweet() {
    if ( !tweet_button->isEnabled() ) return; //作業中
    setEnabledTweet ( false );
    try {
        if ( unsigned int c = tweet_info->countImage() ) {
            //画像あり
            QByteArrayList media;
            QByteArrayList mime;
            media.reserve ( c );
            mime.reserve ( c );
            for ( unsigned int i = 0; i < c; i++ ) { //QByteArrayListの初期化がなければこのforと上のifを統合したけど...
                media.push_back ( QByteArray() ); //予め作成する。
                QBuffer buff ( &media[i] );
                buff.open ( QIODevice::WriteOnly );
                if ( !tweet_info->getImage ( i )->save ( &buff,"PNG" ) ) throw tr ( "画像の読み込みに失敗しました。" ); //すべてPNGで再エンコする鬼畜
                mime.push_back ( QByteArray ( "image/png" ) );
            }
            MediaUpload *upload = new MediaUpload ( media,mime,twitter,this );
            connect ( upload,&MediaUpload::finished,this,&MainWindow::tweetWithMedia );
            connect ( upload,&MediaUpload::aborted,this,[this] {MediaUpload *upload = qobject_cast<MediaUpload*> ( sender() ); if ( QMessageBox::question ( this,APP_NAME,tr ( "アップロード中にエラーが発生しました。再試行しますか。" ) ) == QMessageBox::Yes ) upload->retry(); else delete upload; setEnabledTweet ( true );} );
            if ( !upload->start() ) {
                delete upload;
                throw tr ( "アップロードの初期化作業に失敗しました。" );
            }
            return;
        }
        QString text = tweet_editer->toPlainText();
        processTweet ( text );
        if ( text.isEmpty() ) return setEnabledTweet ( true );

        connect ( twitter->tweet ( text,QByteArray(), ( tweet_info->getReplyTweet() ) ?tweet_info->getReplyTweet()->id:QByteArray() ),&QNetworkReply::finished,this,&MainWindow::finishedTweet );
    } catch ( QString &e ) {
        QMessageBox::critical ( this,APP_NAME,e );
        setEnabledTweet ( true );
    }
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:MediaUploadによって呼ばれる。画像とともにツイートする。
 */
void MainWindow::tweetWithMedia ( const QByteArray &id ) {
    qobject_cast<MediaUpload*> ( sender() )->deleteLater();
    QString text = tweet_editer->toPlainText();
    processTweet ( text );
    connect ( twitter->tweet ( text,id, ( tweet_info->getReplyTweet() ) ?tweet_info->getReplyTweet()->id:QByteArray() ),&QNetworkReply::finished,this,&MainWindow::finishedTweet );
}

/*
 * 引数:加工するツイートテキスト
 * 戻値:なし
 * 概要:tweet_infoにある情報を使ってツイートテキストを加工する。
 */
void MainWindow::processTweet ( QString& text ) {
    if ( TwitterJson::TweetData *reply_tweet = tweet_info->getReplyTweet() ) {
        if ( !text.contains ( "@" + reply_tweet->getOriginalUserInfo().screen_name ) ) {
            text.prepend ( "@" + reply_tweet->getOriginalUserInfo().screen_name + " " );
        }
    }
    if ( TwitterJson::TweetData *quote_tweet = tweet_info->getQuoteTweet() ) {
        text += "  https://twitter.com/" + quote_tweet->getOriginalUserInfo().screen_name + "/status/" + quote_tweet->getOriginalId();
    }
    return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:バージョン情報を表示するダイアログを出す。
 */
void MainWindow::showAbout() {
    QMessageBox::about ( this,APP_NAME + tr ( "について" ),
                         "<b>" APP_NAME_LONG "</b>"
                         "<p>Ver " APP_VERSION "</p>"
                         "<p>" + tr ( "Qtを使用して製作されているTwitterクライアント。" ) + "</p>"
                         "<p>" + tr ( "本ソフトウェアはQtオープンソース版のLGPLv3を選択しています。詳しくは<a href=\"https://www.qt.io/licensing/\">https://www.qt.io/licensing/</a>をご覧ください。" ) + "</p>" //ここは<a>で区切ると訳しにくいはず
                         "<b>" + tr ( "License" ) + "</b>"
                         "<p>" APP_COPYRIGHT  "<br /><br />"
                         "Licensed under the Apache License, Version 2.0 (the \"License\");<br />"
                         "you may not use this file except in compliance with the License.<br />"
                         "You may obtain a copy of the License at<br /><br />"
                         "<a href=\"https://www.apache.org/licenses/LICENSE-2.0\" >https://www.apache.org/licenses/LICENSE-2.0</a><br />"
                         "Unless required by applicable law or agreed to in writing, software"
                         "distributed under the License is distributed on an \"AS IS\" BASIS,"
                         "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br />"
                         "See the License for the specific language governing permissions and"
                         "limitations under the License.</p>"
                         "<a href=\"" APP_HOMEPAGE "\">" + tr ( "このソフトウェアについてのページを開く" ) + "</a>" );
    return;
}



/*
 * 引数:twdata(リツイートしたいツイート)
 * 戻値:なし
 * 概要:リツイートをする。
 */
void MainWindow::retweet ( TwitterJson::TweetData *twdata ) {
    if ( twdata == nullptr );
    else if ( twdata->flag2 & 1 )
        QMessageBox::information ( this,APP_NAME,tr ( "このツイートはすでにリツイートしています。" ) );
    else if ( twdata->flag & 0x10 )
        QMessageBox::information ( this,APP_NAME ,tr ( "鍵アカウントのツイートのためリツイートできません。" ) );
    else
        connect ( twitter->retweet ( twdata->id ),&QNetworkReply::finished,this,&MainWindow::finishedRequest );
    return;
}

/*
 * 引数:twdata(お気に入りに登録したいツイート)
 * 戻値:なし
 * 概要:お気に入りに登録する。
 */
void MainWindow::favorite ( TwitterJson::TweetData *twdata ) {
    if ( twdata == nullptr ) return;
    if ( twdata->flag2 & 2 )
        QMessageBox::information ( this,APP_NAME,tr ( "このツイートはすでにお気に入りに登録しています。" ) );
    else if ( QMessageBox::question ( this,APP_NAME,twdata->user_info.user_name+tr ( "さんのツイートをお気に入りに登録しますか。" ) ) == QMessageBox::Yes )
        connect ( twitter->favorite ( twdata->id ),&QNetworkReply::finished,this,&MainWindow::finishedRequest ); //どうやって成功時にflag2を立てるか。
    return;
}

/*
 * 引数:twdata(削除したいツイート)
 * 戻値:なし
 * 概要:ツイートを削除する。
 */
void MainWindow::deleteTweet ( TwitterJson::TweetData *twdata ) {
    if ( twdata == nullptr );
    else {
        if ( QMessageBox::question ( this,APP_NAME,tr ( "ツイートを削除しますか。" ) ) == QMessageBox::Yes )
            connect ( twitter->tweet_destroy ( twdata->id ),&QNetworkReply::finished,this,&MainWindow::finishedRequest ); //どうやって成功時にflag2を立てるか。
    }
    return;
}

void MainWindow::contentAction ( TwitterJson::TweetData *twdata,unsigned int act ) {
    if ( twdata == nullptr ) return;
    switch ( static_cast<TwitterCore::Action> ( act ) ) { //ここにまとめてもいいかも
    case TwitterCore::Retweet:
        retweet ( twdata );
        break;
    case TwitterCore::QuoteTweet:
        if ( twdata->flag & 0x10 )
            QMessageBox::information ( this,APP_NAME,tr ( "鍵アカウントのツイートのため引用ツイートできません。" ) );
        else {
            TweetContent *content = new TweetContent ( new TwitterJson::TweetData ( *twdata ),TweetContent::Info );
            tweet_info->setQuoteTweet ( content );
            tweet_info->setVisible ( true );
            info_scroll_area->setVisible ( true );
        }
        break;
    case TwitterCore::Reply: {
        TweetContent *content = new TweetContent ( new TwitterJson::TweetData ( *twdata ),TweetContent::Info );
        tweet_info->setReplyTweet ( content );
        tweet_info->setVisible ( true );
        info_scroll_area->setVisible ( true );
        break;
    }
    case TwitterCore::Favorite:
        favorite ( twdata );
        break;
    case TwitterCore::Destroy:
        deleteTweet ( twdata );
        break;
    default:
        ;//とりあえず今の所何もしない
    }
    return;
}







