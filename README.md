## Overview

The qmljsreformatter tool allows to automatically format a QML file via a command-line interface. It is based on a functionality embedded in the Qt Creator IDE.

This project supports two versions of qmljsreformatter:
* The original version whose the source code comes from Qt Creator.
* The Orange version which is a modified version of the Qt Creator source code, which:
  * allows to split (or not) the long lines.
  * adds automatically semicolons (`;`) at the end of the JavaScript statements.

## How to build qmljsreformatter as a standalone binary

Download at least the Qt 5.8 source version: https://download.qt.io/archive/qt/5.8/5.8.0/single/qt-everywhere-opensource-src-5.8.0.tar.gz.mirrorlist

Build Qt 5.8 sources:
```bash
tar xf qt-everywhere-opensource-src-5.8.0.tar.gz
cd qt-everywhere-opensource-src-5.8.0
./configure -prefix {SOMEWHERE}/Qt5.8.0.static -opensource -static -skip wayland -no-egl -nomake examples
make -j 4 # (or more jobs if possible)
make install
```

Build qmljsreformatter
```bash
cd qmljsreformatter
mkdir build && cd build

# Original (from Qt Creator source code) version of qmljsreformatter
{SOMEWHERE}/Qt5.8.0.static/bin/qmake ..
# or the Orange version
# $ {SOMEWHERE}/Qt5.8.0.static/bin/qmake .. CONFIG+=ORANGE

make -j 4 # (or more jobs if possible)
```

## How to use qmljsreformatter

Considering the following _Example.qml_ file:

```qml
// Example.qml
import QtQuick 2.0
Item{property int id:5;Component.onCompleted:{console.log("Completed!")}
                       }
```

Run _qmljsreformatter_ to create a new file:

```bash
./qmljsreformatter Example.qml Example.new.qml
```

or to overwrite the source file:

```bash
./qmljsreformatter Example.qml Example.qml
```

The resulting file will be reformatted that way:

```qml
// Example.qml
import QtQuick 2.0

Item {
    property int id: 5
    Component.onCompleted: {
        console.log("Completed!")
    }
}
```

**Note**: If qmljsreformatter has been built with the Orange configuration, the resulting file will have semicolons (`;`) at the end of all the JavaScript statements. This last will then be reformatted that way:

```qml
// Example.qml
import QtQuick 2.0

Item {
    property int id: 5
    Component.onCompleted: {
        console.log("Completed!");
    }
}
```

## How to run the tests

The tests are only valid if qmljsreformatter is compiled with the Orange configuration.

```bash
cd qmljsreformatter/tests
./run-tests.sh
```

It is also possible to run a specific test. The following commands will execute the tests contained in of the Header.test.qml file.

```bash
cd qmljsreformatter/tests
./run-test.sh Header
[OK] Header.test.qml
```
