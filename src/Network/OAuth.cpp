/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * OAuth 1.0a クラス
 * QtにOAuthあるが実装してしまったのでこれを使う。
 * かなり古いコード(Windows版からの移植)
*/
#include "OAuth.h"
#include "SHA1cc.h"
#include <QDateTime>
#include <QUrl>
#include <string.h>

OAuth::OAuth ( const char *_cons_key, const char *_cons_sec, const char *_oauth_token,const  char *_oauth_sec ) {
    //アプリ認証する前はoauth_keyなどが空だから、認証後はこれを呼ぶ。
    sprintf ( key, "%s&%s", _cons_sec, _oauth_sec ); //こっちが簡単。
    strcpy ( cons_key, _cons_key );
    strcpy ( cons_sec, _cons_sec );
    strcpy ( oauth_token, _oauth_token );
    strcpy ( oauth_secret, _oauth_sec );
}

OAuth::OAuth ( const OAuth &other ) {
    *this = other;//ポインタを所持するときは気をつけること
}

/*
 * 引数:url(アクセスするURL),post(POSTアクセスの場合はtrue,GETアクセスならfalse),elements(OAuthエントリー),OAuth_str(結果受け取り用)
 * 戻値:なし
 * 概要:渡された引数を使ってOAuthヘッダを作成する。
 */
void OAuth::makeOAuthHeader ( const char *url, bool post, std::vector<OAuth::entry>&elements, std::string &oauth_result ) {
    QByteArray    oauth_sign,timestamp,nonce,signature;
    char                   buf[buff_size];

    //下準備
    timestamp = QString::number ( QDateTime::currentDateTimeUtc().toSecsSinceEpoch() ).toLatin1(); //Qt5.8以降
    nonce = QString::number ( QTime::currentTime().msec() ).toLatin1(); //qrand使わんでもこれで事足りそう。

    //とりあえず必要項目をプッシュする。
    elements.push_back ( { "oauth_consumer_key",cons_key ,false } ); //変数名指定してない
    elements.push_back ( { "oauth_timestamp", timestamp.constData(),false } );
    elements.push_back ( { "oauth_nonce", nonce.constData() ,false } );
    elements.push_back ( { "oauth_signature_method","HMAC-SHA1",false } );
    if ( *oauth_token != '\0' ) elements.push_back ( { "oauth_token",oauth_token,false } );
    elements.push_back ( { "oauth_version","1.0",false } );

    //名前でソート(速度とメモリ使用量が...)
    std::sort ( elements.begin(), elements.end(), [] ( entry e1, entry e2 ) {
        return strcmp ( e1.title, e2.title ) <= 0;
    } );
    oauth_sign.reserve ( elements.capacity() );
    //シグネチャ生成
    for ( OAuth::entry &element : elements ) {
        oauth_sign += ( ( oauth_sign.isEmpty() ) ?"":"&" ) + QByteArray ( element.title ).replace ( ',',"%2C" )  + "=" + QByteArray ( element.body ).replace ( ',',"%2C" ); //メモリはどうなのだろうか...(追記:OAuth::entryのbodyはQByteArrayにしても良さそう)
    }

    oauth_sign = ( post ? "POST&" : "GET&" ) + QUrl::toPercentEncoding ( url ) + "&" + QUrl::toPercentEncoding ( oauth_sign );

    //鍵はあるはず。
    HMAC_SHA1 ( key, oauth_sign.constData(), buf );
    signature = QUrl::toPercentEncoding ( QByteArray::fromRawData ( buf,buff_size ).toBase64() ); //めんどい処理...

    //ようやく本体の生成。
    elements.insert (
    std::find_if ( elements.begin(), elements.end(), [] ( entry e ) {
        return strcmp ( e.title,"oauth_signature" ) > 0;
    } ),
    { "oauth_signature", signature.constData(),false } );
    //いちいちいらないelementsを消してても時間がかかるので削除しないようにして無視するようにした。
    oauth_result.reserve ( elements.capacity() );
    //OAuth_str = "OAuth ";//ここもWin版と違う。
    for ( OAuth::entry &element : elements ) {
        if ( element.del_when_header ) continue;
        oauth_result += ( ( oauth_result.empty() ) ?"OAuth "/*最初に追加する文字列*/:"," )+std::string ( element.title ) + "=\"" + std::string ( element.body ) + "\"";
    }
    return;
}

/*
 * 引数:key(鍵、終端に\0をつける),data(計算するデータ、終端に\0をつける),buff(結果受け取り用)
 * 戻値:なし
 * 概要:渡された引数を使ってHMAC-SHA1を算出する。自前だから間違った実装な可能性あり。
 */
void OAuth::HMAC_SHA1 ( const char *key, const char *data, char buff[buff_size] ) {
    SHA1_Context_t t;
    uint8_t SHA1_key[block_size];
    size_t  data_size = strlen ( data ),
            key_size = strlen ( key ),
            cnt/*なんとなく型合わせるために*/;

    if ( key_size > block_size ) { //Block_Sizeを超えたら収まるようにキーを縮める(ハッシュを出す)らしい
        SHA1cc_Init ( &t );
        SHA1cc_Update ( &t, key, key_size );
        SHA1cc_Finalize ( &t, SHA1_key );
        key_size = buff_size;//SHA1にかけると20Byteになる
    } else strcpy ( ( char * ) SHA1_key, key );

    SHA1cc_Init ( &t );
    //KeyでXORするらしい
    for ( cnt = 0; cnt < key_size; cnt++ ) * ( SHA1_key + cnt ) ^= 0x36;
    for ( ; cnt < block_size; cnt++ ) * ( SHA1_key + cnt ) = 0x36; //Keyが足りなかった分
    SHA1cc_Update ( &t, SHA1_key, block_size );
    SHA1cc_Update ( &t, data, data_size );
    SHA1cc_Finalize ( &t, ( uint8_t * ) buff );

    //おんなじことを0x5cでもするらしい
    SHA1cc_Init ( &t );
    //KeyでXORするらしい
    //for (cnt = 0; cnt < key_size; cnt++) *(SHA1_key+cnt)=(*(SHA1_key + cnt)^0x36/*再反転して元に戻す*/) ^ 0x5C;
    for ( cnt = 0; cnt < key_size; cnt++ ) * ( SHA1_key + cnt ) ^= 0x6A; //^0x36と^0x5Cをまとめたもの
    for ( ; cnt < block_size; cnt++ ) * ( SHA1_key + cnt ) = 0x5C; //Keyが足りなかった分
    SHA1cc_Update ( &t, SHA1_key, block_size );
    SHA1cc_Update ( &t, ( uint8_t * ) buff, buff_size );
    SHA1cc_Finalize ( &t, ( uint8_t * ) buff );
    return;
}
