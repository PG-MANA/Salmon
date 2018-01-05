/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * Twitterの設定を保存&読込する構造体
 */
#include "TwitterSetting.h"
#include "../Salmon.h"
#include <QtCore>
#include <QSettings>
#include <QDir>


TwitterSetting::TwitterSetting ( const QString &file_name ) : setting ( getFilePath ( file_name ), QSettings::IniFormat ) {
    setting.beginGroup ( "Twitter" );
    strcpy ( user_id,setting.value ( "User_id","" ).toByteArray().constData() );
    strcpy ( user_name,setting.value ( "User_Name","" ).toString().toStdString().c_str() );
    strcpy ( oauth_token,setting.value ( "OAuth_token","" ).toByteArray().constData() );
    strcpy ( oauth_secret,setting.value ( "OAuth_secret","" ).toByteArray().constData() );
    if ( !decode_string ( oauth_token ) ) oauth_token[0] = 0;
    if ( !decode_string ( oauth_secret ) ) oauth_secret[0] = 0;
    setting.endGroup();
}

/*
 * 引数:file_name(設定ファイル名)
 * 戻値:ファイルパス
 * 概要:file_nameのパスを返す
 */
QString TwitterSetting::getFilePath ( const QString &file_name ) {
    QDir setting_dir ( QStandardPaths::writableLocation ( QStandardPaths::ConfigLocation ) );
    if ( !setting_dir.exists ( APP_NAME ) ) {
        setting_dir.mkdir ( APP_NAME );
    }
    setting_dir.cd ( APP_NAME );
    return setting_dir.filePath ( file_name );
}

/*
 * 引数:なし
 * 戻値:空かどうか
 * 概要:すべての項目がセットされていればfalse、それ以外はtrue
 */
bool TwitterSetting::isEmpty() {
    return !*user_id || !*user_name || !*oauth_token || !*oauth_secret;
}

/*
 * 引数:なし
 * 戻値:Window座標
 * 概要:Twitter関係ないけどオマケ機能。Window座標を読み取る
 */
QByteArray TwitterSetting::geometry() {
    setting.beginGroup ( "Window" );
    QByteArray &&result = setting.value ( "geometry" ).toByteArray();
    setting.endGroup();
    return result;
}

/*
 * 引数:geometry
 * 戻値:なし
 * 概要:Twitter関係ないけどオマケ機能2。Window座標を保存する
 */
void TwitterSetting::setGeometry ( const QByteArray &geometry ) {
    setting.beginGroup ( "Window" );
    setting.setValue ( "geometry", geometry );
    setting.endGroup();
}

/*
 * 引数:file_name(設定ファイルの名前)
 * 戻値:成功時true、失敗時false
 * 概要:設定を暗号化して指定されたファイルに書き込む。
 */
bool TwitterSetting::save () {
    char tmp_token[sizeof ( oauth_token )];
    char tmp_secret[sizeof ( oauth_secret )];

    strcpy ( tmp_token,oauth_token );
    strcpy ( tmp_secret,oauth_secret );

    setting.beginGroup ( "Twitter" );
    setting.setValue ( "User_id",user_id );
    setting.setValue ( "User_Name",user_name );
    if ( !encode_string ( tmp_token ) ||
            !encode_string ( tmp_secret ) ) return false;
    setting.setValue ( "OAuth_token",QByteArray ( tmp_token ) );
    setting.setValue ( "OAuth_secret",QByteArray ( tmp_secret ) );
    setting.endGroup();
    return true;
}

/*
 * str(暗号化する文字列。終端に\0をつける。)
 * 戻値:成功時true、失敗時false
 * 概要:CONS_SECを鍵として鍵と暗号化する文字列の各文字を足していく。ある程度の強度はありそうだがしっかりした共通鍵暗号化をしてみたい...(技術不足)
 * 備考:2017/10/08の変更により強度が低下している
 */
bool TwitterSetting::encode_string ( char *str ) {
    unsigned int i = 0;
    const char key[] = CONS_SEC;
    while ( str[i] ) {
        //ASCIIコード上、足したほうがいい
        if ( str[i] > '~' ) return false; //ASCII文字じゃない
        str[i] += key[i % sizeof ( key )];
        i++;
    }
    return true;
}

/*
 * str(暗号化された文字列。終端に\0をつける。)
 * 戻値:成功時true、失敗時false
 * 概要:CONS_SECを鍵として鍵と暗号化する文字列の各文字を引いていく。
 */
bool TwitterSetting::decode_string ( char *str ) {
    unsigned int i = 0;
    const char key[] = CONS_SEC;
    while ( str[i] ) {
        if ( * ( unsigned char* ) str > 0xFC ) return false; //('~'+'~')以上なのでASCII文字じゃない
        str[i] -= key[i % sizeof ( key )];
        i++;
    }
    return true;
}
