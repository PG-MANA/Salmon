/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * TwitterのAPIのUrlをおいておく
 */

//classではないほうがいいかと...
namespace TwitterUrl {
//認証関係
static const char access_token[] = "https://api.twitter.com/oauth/access_token";
static const char authorize[] = "https://api.twitter.com/oauth/authorize?oauth_token=";
static const char request_token[] = "https://api.twitter.com/oauth/request_token";

//TimeLine関係
static const char home_timeline[] = "https://api.twitter.com/1.1/statuses/home_timeline.json";

//ストリーム関係
static const char filter_stream[] = "https://stream.twitter.com/1.1/statuses/filter.json";

//ツイート関係
static const char statuse_update[] = "https://api.twitter.com/1.1/statuses/update.json";
static const char statuse_destroy[] = "https://api.twitter.com/1.1/statuses/destroy/";//ここ.jsonがないので注意
static const char statuse_retweet[] = "https://api.twitter.com/1.1/statuses/retweet/";

//お気に入り関係
static const char favorite_create[] = "https://api.twitter.com/1.1/favorites/create.json";

//アップロード関係
static const char media_upload[] = "https://upload.twitter.com/1.1/media/upload.json";

//リスト関係
static const char lists_list[] = "https://api.twitter.com/1.1/lists/list.json";

//ユーザ関係
static const char friends_ids[] = "https://api.twitter.com/1.1/friends/ids.json";
}
