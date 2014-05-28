/*
 *  Copyright 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) version 3, or any
 *  later version accepted by the membership of KDE e.V. (or its
 *  successor approved by the membership of KDE e.V.), which shall
 *  act as a proxy defined in Section 6 of version 3 of the license.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtTest 1.0

import org.kde.solid 1.0 as Solid

TestCase {
    name: "SolidDevices"

    Solid.Devices {
        id: devices
        query: "IS Battery"
    }

    function test_minimal() {
        compare(devices.count, 3);
        for (var i; i<devices.count; ++i) {
            if ((devices.get(i).udi == "/org/kde/solid/fakehw/acpi_BAT0")) {
                compare(devices.get(i).vendor, "Acme Corporation");
                verify(devices.get(i).name, "Battery Bay");
            }
        }
    }
}
