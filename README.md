The qmljsreformatter tool allows to automatically format a QML file via a command-line interface. It is based on a functionality embedded in the Qt Creator IDE.

## How to build qmljsreformatter as a standalone binary

Download at least the Qt 5.8 source version: https://download.qt.io/archive/qt/5.8/5.8.0/single/qt-everywhere-opensource-src-5.8.0.tar.gz.mirrorlist

Build Qt 5.8 sources:
```
$ tar xf qt-everywhere-opensource-src-5.8.0.tar.gz
$ cd qt-everywhere-opensource-src-5.8.0
$ ./configure -prefix {SOMEWHERE}/Qt5.8.0.static -opensource -static -skip wayland -no-egl -nomake examples
$ make -j 4 (or more jobs if it is possible)
$ make install
```

Build qmljsreformatter:
```
$ cd qmljsreformatter
$ mkdir build && cd build
$ {SOMEWHERE}/Qt5.8.0.static/bin/qmake .. 
$ make -j 4
```
