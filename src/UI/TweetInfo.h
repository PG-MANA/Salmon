/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * TweetInfo クラス
 * MainWindowのしたにある画像添付したりリプライしたりするときに出てくるやつ。ツイートに関するデータも保有する。
 */
#pragma once

#include "ImageLabel.h"
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class MainWindow;
namespace TwitterJson {
struct TweetData;
}

class TweetInfo : public QWidget {
    Q_OBJECT
public:
    explicit TweetInfo ( MainWindow* parent_window,QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
    virtual ~TweetInfo();
    const QPixmap *getImage ( const unsigned int index ) const;
    void setImage ( const QPixmap &pixmap,const unsigned int index );
    void deleteImage ( const unsigned int index );
    void deleteImageAll();
    TwitterJson::TweetData *getQuoteTweet();
    void setQuoteTweet ( TweetContent *data );
    TwitterJson::TweetData *getReplyTweet();
    void setReplyTweet ( TweetContent *data );
    unsigned int countImage() const;
    bool isEmpty();

public slots:
    void deleteQuoteTweet();
    void deleteReplyTweet();
    void ImageMenu(TwitterJson::TweetData *twdata,unsigned int index);

private:
    void closeWhenEmpty();
    MainWindow *win;
    QVBoxLayout *main_layout;
    QHBoxLayout *media_layout;
    QHBoxLayout *reply_layout;
    QHBoxLayout *quote_layout;
};
