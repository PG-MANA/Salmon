/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * enumなど定義
 */
#pragma once

namespace TwitterCore {
enum Error : unsigned int {
    CannotConnect =1,/*接続不可能*/
    NetworkError,/*切断、APIエラー*/
    BadPointer/*不正なポインタがあった*/
};

enum Action : unsigned int {
    Tweet =1,
    Retweet,
    QuoteTweet,
    Reply,
    Favorite,
    Destroy
};
};
