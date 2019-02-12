import QtQuick 2.5

Item {
    a: a === a // a
    a: a == a() // a
    a: a() < a // a
    a: a() > a() // a
    a: a === a == a // a

    onA: {
        a === a; // a
        a == a(); // a
        a() < a; // a
        a() > a(); // a
        a === a == a; // a

        a = a === a; // a
        a = a == a(); // a
        a = a() < a; // a
        a = a() > a(); // a
        a = a === a() == a; // a
        a = a === (a = a() === a) == a; // a
    }
}
