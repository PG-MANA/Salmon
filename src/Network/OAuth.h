/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * OAuth 1.0a用クラス
*/
#pragma once

#include <string>
#include <vector>
#include <algorithm>

class OAuth {
public:
    typedef struct entry {
        const char *title;//エントリー名
        const char *body;//内容
        bool del_when_header;//ヘッダー生成時には削除するかしないか
    } entry;

    explicit OAuth ( const char *_cons_key,const  char *_cons_sec, const char *_oauth_token,const char *_oauth_sec ) ;
    explicit OAuth ( const OAuth &other ); //copy
    void makeOAuthHeader ( const char *url, bool post, std::vector<OAuth::entry>&elements, std::string &oauth_result );

private:
    static const unsigned int buff_size = 20;
    static const unsigned int block_size = 64;
    void HMAC_SHA1 ( const char *key, const char *data, char buff[buff_size] );
    char key[128];//適当...
    char cons_key[64];
    char cons_sec[64];
    char oauth_token[64];
    char oauth_secret[64];
};
