import QtQuick 2.5

Item {
    property int a: 1
    property int a: a
    property int a: a()
    property int a: {
        a(); // a
    }

    default property int a: 1
    default property int a: a
    default property int a: a()
    default property int a: {
        a(); // a
    }

    readonly property int a: 1
    readonly property int a: a
    readonly property int a: a()
    readonly property int a: {
        a(); // a
    }

    signal a
    signal a(int a)
    signal a(int a, int a)

    a: {

    } // a
    a: {
        a: a();
    } // a

    a: [
        a {}
    ] // a

    a: [
        a {},
        a {}
    ] // a

    a: a {
        onA: {
            a();
        }
    }
}
