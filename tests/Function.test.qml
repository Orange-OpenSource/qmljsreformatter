import QtQuick 2.5

Item {
    a: function a() {
        a()
    }

    function a() {
        a()
    }

    function a(a, a) {
        a()
    }

    function a() {
        function a() {
            a()
        }
        function a(a, a) {
            a()
        }
        a()
    }

    onA: {
        a(function() {
            a()
        })
    }
}
