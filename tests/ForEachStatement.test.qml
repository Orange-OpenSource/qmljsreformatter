import QtQuick 2.5

Item {
    onA: {
        for (var a in a)
            a()

        for (a in a)
            a()

        for (a() in a())
            a()

        for (var a in a) {
            a()
        }

        for (a in a) {
            a()
        }

        for (a() in a()) {
            a()
        }
    }
}
