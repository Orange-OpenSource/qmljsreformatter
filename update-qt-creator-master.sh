#!/bin/bash

TEMPDIR=`mktemp -d`
wget https://github.com/qt-creator/qt-creator/archive/master.zip -O $TEMPDIR/master.zip
unzip $TEMPDIR/master.zip -d $TEMPDIR

rm -rf qt-creator-simplified/*

QTCREATORLIBS='qt-creator-simplified/src/libs/'

mkdir -p $QTCREATORLIBS/qmljs
mkdir -p $QTCREATORLIBS/languageutils
mkdir -p $QTCREATORLIBS/utils
mkdir -p $QTCREATORLIBS/cplusplus
mkdir -p $QTCREATORLIBS/3rdparty/cplusplus
mkdir -p $QTCREATORLIBS/3rdparty/optional

cp -R $TEMPDIR/qt-creator-master/src/libs/qmljs/* $QTCREATORLIBS/qmljs/
cp -R $TEMPDIR/qt-creator-master/src/libs/languageutils/* $QTCREATORLIBS/languageutils/
cp -R $TEMPDIR/qt-creator-master/src/libs/utils/* $QTCREATORLIBS/utils/
cp -R $TEMPDIR/qt-creator-master/src/libs/cplusplus/* $QTCREATORLIBS/cplusplus/
cp -R $TEMPDIR/qt-creator-master/src/libs/3rdparty/cplusplus/* $QTCREATORLIBS/3rdparty/cplusplus/
cp -R $TEMPDIR/qt-creator-master/src/libs/3rdparty/optional/* $QTCREATORLIBS/3rdparty/optional/

rm -rf $TEMPDIR
