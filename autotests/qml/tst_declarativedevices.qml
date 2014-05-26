import QtQuick 2.0
import QtTest 1.0

import org.kde.solid 1.0 as Solid

TestCase {
    name: "SolidDevices"

    Solid.Devices {
        id: devices
    }

    function test_minimal() {
        compare(devices.count, 0);
    }
}
