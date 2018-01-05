# Salmon for Linux
(Format:UTF-8)  
LinuxデスクトップユーザのためのTwitter Client
## 概要
Linux/X11を対象としたTwitter Clientです。と言ってもQtを使用しているので環境さえ整えばMacやWindowsでも動作すると思います。
なおQtオープンソース版のLGPLv3を選択しています。 Qtのライセンスは https://www.qt.io/licensing/ をご覧ください。
Qtを使用したC++の勉強用でもあるので、「コードの書き方がナンセンス」「何だこのソースは(驚愕)」と言うことがありましたら優しく教えてくださると嬉しいです。  
基本的にはサラッと書いたので、メモリ使用量・コード最適化がしっかりできてないと思います。ただそれなりに軽く動くようにしているつもりです。

KDevelop 5で開発していますが、使用しなくてもビルドはできます。  
openSUSE Tumbleweed で開発してます。  
### 開発状況
master : 一応区切りがついたもの。安定版とは限らない。  
dev : 細かなことやとんでもないコードがあるかもしれないもの。実験要素満載。  
### 動作環境

* Qt 5.9
* Phonon4qt5
* Twitterができる程度の通信環境
* CPU:各デスクトップ環境の推奨する性能以上のもの、2コア以上はほしい。
* メモリ:起動時間が長いほどメモリ使用量は増加していく傾向にあります。たくさん積んでおきましょう。

ブラウザが快適に動作する程度ならこのソフトウェアも快適に動作するはずです。
## ライセンス
Copyright 2017 PG_MANA  
This software is Licensed under the Apache License Version 2.0  
See LICENSE.md  
注意:src/Resources/icon/以下のアイコンなどは「ロゴ」にあたり、派生成果物に含めることはできません。  
補足:src/Network/SHA1cc.cppとsrc/Network/SHA1cc.hはパブリックドメイン実装の https://ja.osdn.net/projects/sha1cc/ の派生物です。  
## ビルド
### ビルド環境
* Qt 5.9 (Qt5::Widgets Qt5::Network) 開発用ヘッダファイルなど
* Phonon4qt5 開発用ヘッダファイルなど  (無い場合自動的にQtMultiMediaを選択しますが、一部環境では動画が再生できない可能性があります)
* c++17対応C++コンパイラ
* cmake  3.1以上

### ビルド方法
cd プロジェクトディレクトリ  
mkdir build  
cd build  
cmake ../  
make  
./Salmon
### ビルドエラーが出た時

* 「Please set the CONS_KEY and CONS_SEC」と出た => src/Key.hに https://apps.twitter.com より取得してきたCONS_KEY CONS_SECを書き込み、「#error  Please set the CONS_KEY and CONS_SEC」の行を削除してください。
* 「_rotlが定義されていない」と出た => src/Network/Sha1cc.hに「#define NOT_ROL」という風にNOT_ROLを定義してください。

## 備考
* Gitリポジトリに上げる際、Key.hをGit管理対象から外すなどしたほうが良いです。

## リンク
### SalmonについてのWebページ
  https://mnas.info/soft/salmon/linux/
### Qtのドキュメントページ
  https://doc.qt.io/
### 開発者のTwitterアカウント
  https://twitter.com/PG_MANA_
### 開発者のWebページ
  https://pg-mana.net
