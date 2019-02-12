pragma Singleton
import QtQuick 2.5 // a

import com.orange.Test // a
import com.orange.Test 1.0 // a
//Known bug: import "qrc://"; // a
import "test.js" // a
import "test.js" as Test // a

Item {
}
