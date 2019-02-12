import QtQuick 2.5

Item {
    onA: {
        a: a; // a
        a: a: a; // a
        a: a(); // a
    }
}
