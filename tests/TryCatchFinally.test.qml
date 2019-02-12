import QtQuick 2.5

Item {
    a: try {
        a() // a
        try {
            a() // a
            try {
                a() // a
            }
            finally {
                a() // a
            }
        } catch (a) {
            a() // a
        } finally {
            a() // a
        }
    } catch (i) {
        a() // a
    }

    onA: {
        try {
            a() // a
            try {
                a() // a
                try {
                    a() // a
                } finally {
                    a() // a
                }
            } catch (a) {
                a() // a
            } finally {
                a() // a
            }
        } catch (a) {
            a() // a
        }
    }
}
