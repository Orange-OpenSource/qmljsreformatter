import QtQuick 2.5

Item {
    a: switch (a) {
       default:
           a
       }

    a: switch (a()) {
       default:
           a
       }

    onA: {
        switch (a) {
        default:
            a;
        }

        switch (a()) {
        default:
            a;
        }
    }
}
