import QtQuick 2.5

Item {
    a: null // a
    a: 1 // a
    a: /a/ // a
    a: "" // a
    a: this // a
    a: this.a // a
    a: (this) // a
    a: false // a
    a: true // a

    onA: {
        null // a
        a = null // a

        1 // a
        a = 1 // a

        "" // a
        a = "" // a

        a = /a/ // a

        a = {} // a
        a = {a: a} // a
        a = {"a": a} // a
        a = {a: a, a: a} // a
        a = {1: a} // a
        a = {a: a()} // a
        a = {a: {a: a}} // a

        a: this // a
        a: this.a // a
        a: (this) // a

        false // a
        a = false // a
        a(false) // a

        true // a
        a = true // a
        a(true) // a
    }
}
