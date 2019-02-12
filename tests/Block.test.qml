import QtQuick 2.5

Item {
    a: if (a()) {
           a()
       }

    onA: {
        a()

        if (a()) {
            a()
        }
    }
}
