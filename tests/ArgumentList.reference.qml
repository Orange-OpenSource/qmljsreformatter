import QtQuick 2.5

Item {
    onA: {
        a(a);
        a(a, b);
        a(a(), b++);
        a(a() /* a */
          , ++b); // a
        a(1, 1, a(a(), a().x));
    }
}
