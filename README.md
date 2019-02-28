## Overview

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

## How to use qmljsreformatter

Considering the following _Example.qml_ file:

```qml
// Example.qml
import QtQuick 2.5
Item{property int id:5;Component.onCompleted:{console.log("Completed!")}
                       }
```

Run _qmljsreformatter_ to create a new file:

```
$ ./qmljsreformatter Example.qml Example.new.qml
```

or to overwrite the source file:

```
$ ./qmljsreformatter Example.qml Example.qml
```

The resulting file will contain:

```qml
// Example.qml
import QtQuick 2.5

Item {
    property int id: 5
    Component.onCompleted: {
        console.log("Completed!");
    }
}
```

## How to run the tests

```
$ cd qmljsreformatter/tests
$ ./run-tests.sh
```

It is also possible to run a specific test. The following commands will execute the tests contained in of the Header.test.qml file.

```
$ cd qmljsreformatter/tests
$ ./run-test.sh Header
[OK] Header.test.qml
```
