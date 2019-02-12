import QtQuick 2.5

Item {
    a: delete a // a
    a: delete a() // a

    a: new a // a
    a: new (a) // a
    a: new (a()) // a
    a: new a() // a
    a: a = new a // a

    a: !a // a
    a: !(a) // a
    a: !(a()) // a
    a: !a() // a
    a: a = !a // a

    a: ~a // a
    a: ~(a) // a
    a: ~(a()) // a
    a: ~a() // a
    a: a = ~a // a

    a: typeof a // a
    a: typeof a() // a

    a: void a // a
    a: void (a) // a
    a: void (a()) // a
    a: void a() // a
    a: a = void a // a

    a: with (a) {
           a() // a
       }
    a: with (a) {
           a() // a
       }
    a: with (a()) {
           a // a
       }

    onA: {
        delete a; // a
        delete (a); // a
        delete (a()); // a
        delete a(); // a

        new a; // a
        new (a); // a
        new (a()); // a
        new a(); // a
        a = new a; // a
        a = new (a); // a
        a = new (a()); // a
        a = new a(); // a
        a = a = new a; // a

        !a; // a
        !(a); // a
        !(a()); // a
        !a(); // a
        a = !a; // a
        a = !(a); // a
        a = !(a()); // a
        a = !a(); // a
        a = a = !a; // a

        ~a; // a
        ~(a); // a
        ~(a()); // a
        ~a(); // a
        a = ~a; // a
        a = ~(a); // a
        a = ~(a()); // a
        a = ~a(); // a
        a = a = ~a; // a

        return a; // a
        return (a); // a
        return (a()); // a
        return a(); // a

        throw a; // a
        throw (a); // a
        throw (a()); // a
        throw a(); // a

        typeof a; // a
        typeof (a); // a
        typeof (a()); // a
        typeof a(); // a

        void a; // a
        void (a); // a
        void (a()); // a
        void a(); // a
        a = void a; // a

        a = void a; // a
        a = void (a); // a
        a = void (a()); // a
        a = void a(); // a
        a = a = void a; // a

        with (a) {
            a(); // a
        }
        with (a) {
            a(); // a
        }
        with (a()) {
            a; // a
        }
    }
}
