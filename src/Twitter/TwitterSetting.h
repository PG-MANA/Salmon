/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * Twitterの設定を保存&読込する構造体
 */
#pragma once

#include <QSettings>

struct TwitterSetting {
    char oauth_token[64];
    char oauth_secret[64];
    char user_id[20];//本当は15文字だけど将来考えて...
    char user_name[20];

    explicit TwitterSetting ( const QString &file_name );

    bool save ();
    bool isEmpty();
    //おまけ
    QByteArray geometry();
    void setGeometry ( const QByteArray &geometry );
private:
    //暗号化
    bool encode_string ( char *str );
    bool decode_string ( char *str );
    //その他
    QString getFilePath ( const QString &file_name );
    QSettings setting;
};
