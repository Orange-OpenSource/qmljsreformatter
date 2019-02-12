import QtQuick 2.5

Item {
    function a() {
        var a
        var a = a // a
        var a = a() // a
        var a, a
        var a, a = a // a
        var a, a = a() // a
        var a, a = a(), a
    }

    onA: {
        var a
        var a = a // a
        var a = a() // a
        var a, a
        var a, a = a // a
        var a, a = a() // a
        var a, a = a(), a
    }
}
