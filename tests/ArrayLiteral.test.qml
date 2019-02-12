import QtQuick 2.5

Item {
    a: [] // a
    a: [0] // a
    a: [a()] // a
    a: [a(), a()] // a
    a: [, [a]] // a
    a: [[], [0], [a()], [a(), a()]] // a
    a: [[], [0], [a()], , [a(), a()]] // a

    onA: {
        []; // a
        [0]; // a
        [a()]; // a
        [a(), a()]; // a
        [, [a]]; // a
        [[], [0], [a()], [a(), a()]]; // a
        [[], [0], [a()], , [a(), a()]]; // a

        a = [] // a
        a = [0] // a
        a = [a()] // a
        a = [a(), a()] // a
        a = [, [a]] // a
        a = [[], [0], [a()], [a(), a()]] // a
        a = [[], [0], [a()], , [a(), a()]] // a
    }
}