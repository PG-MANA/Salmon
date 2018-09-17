/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * ================
 * Salmon for Linux
 * ================
 * Qtを使用したLinux用Twitter クライアント
 */
#include "Salmon.h"
#include "UI/MainWindow.h"
#include <QApplication>
#include <QTextCodec>

int main ( int argc, char *argv[] ) {
    QApplication app ( argc, argv );
    //全般設定
    QTextCodec::setCodecForLocale ( QTextCodec::codecForName ( "UTF-8" ) );
    app.setWindowIcon ( QIcon ( ":/icon-normal.png" ) ); //埋め込みMainWindow window;
    MainWindow window;
    return window.init ( "default.ini" ) ?window.show(),app.exec() :EXIT_FAILURE;
}
