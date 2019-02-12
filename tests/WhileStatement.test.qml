import QtQuick 2.5

Item {
    onA: {
        while (a)
            a // a

        while (a())
            a() // a

        while (a) {
            a // a
        }

        while (a()) {
            a() // a
        }
    }
}
