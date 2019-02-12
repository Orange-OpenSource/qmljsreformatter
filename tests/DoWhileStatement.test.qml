import QtQuick 2.5

Item {
    onA: {
        do
            a // a
        while (a)

        do
            a() // a
        while (a())

        do {
            a //a
        } while (a)

        do {
            a() // a
        } while (a())
    }
}
