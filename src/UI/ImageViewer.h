/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * ImageViewer クラス
 * 画像を表示する。
 */
#pragma once

#include <QWidget>
#include "../Network/Network.h"
#include "../Twitter/Twitter.h"
#include "../Twitter/TwitterJson.h"

class ImageLabel;
class QScrollArea;
class QVBoxLayout;
class QPushButton;

class ImageViewer: public QWidget {
    Q_OBJECT
public:
    explicit ImageViewer ( TwitterJson::TweetData *twdata,unsigned int index = 0,QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );

public slots:
    void nextImage();
    void backImage();
    void copy();
    void save();
private slots:
    void fit();
private:
    void setImage ( const QString &url );
    void createButtons ( QVBoxLayout *main_layout );
    TwitterJson::MediaInfo media_data;
    ImageLabel *iml;
    QPushButton *next_button;
    QPushButton *back_button;
    QPushButton *save_button;
    unsigned int now_index;
    bool first;
    Network net;
};
