import QtQuick 2.5

Item {
    a: a() // a
    a: a(a) // a
    a: a(a, a()) // a
    a: a(a(), a) // a

    onA: {
        a(); // a
        a(a); // a
        a(a, a()); // a
        a(a(), a); // a

        // issue #1
        a(); // a
        a(); // a
        a(); /* a */
        a(); /* a */
        a(); /*
                a */
        a(); /*
                a */
        a;
    }
}
