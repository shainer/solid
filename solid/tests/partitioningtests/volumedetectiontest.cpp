/*
    Copyright 2012 Lisa Vitolo <shainer@chakra-project.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/
#include <tests/partitioningtests/volumedetectiontest.h>
#include <solid/managerbase_p.h>
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/partitioner/volumemanager.h>

#include <QtTest/QTest>
#include <QtCore/QDebug>

#include <stdlib.h>

#ifndef FAKE_PARTITIONS_XML
    #error "FAKE_PARTITIONS_XML not set. An XML file describing disks and partitions is required for this test"
#endif

QTEST_MAIN(VolumeDetectionTest)

using namespace Solid::Partitioner;

void VolumeDetectionTest::initTestCase()
{
    setenv("SOLID_FAKEHW", FAKE_PARTITIONS_XML, 1);
    Solid::ManagerBasePrivate *manager
        = dynamic_cast<Solid::ManagerBasePrivate*>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager*>(manager->managerBackends().first());
}

void VolumeDetectionTest::test()
{
    VolumeManager* manager = VolumeManager::instance();
    VolumeTree diskTree = manager->diskTree("/org/kde/solid/fakehw/storage_serial_HD56890I");
    
    diskTree.print();
 
    QCOMPARE(diskTree.valid(), true); /* a tree has been created for this drive? */
    QCOMPARE(diskTree.extendedPartition() == NULL, false); /* there should be an extended partition... */
    QCOMPARE(diskTree.extendedPartition()->name(), QString("/org/kde/solid/fakehw/extended_volume")); /* this one! */
    
    /*
     * Checks if all the logical partitions have been detected as such.
     */
    QStringList logicalNames;
    QStringList expectedNames = QStringList() << "/org/kde/solid/fakehw/swap_volume" << "/org/kde/solid/fakehw/home_volume"
                                              << "/org/kde/solid/fakehw/foreign_logical";
    foreach (DeviceModified* logical, diskTree.logicalPartitions()) {
        logicalNames << logical->name();
    }
    QCOMPARE(logicalNames, expectedNames);
    
    /*
     * Checks if all the free space blocks have been detected in the correct position.
     * The name contains offset and size so we're implicitly checking the geometry is what we expect it to be.
     */
    DeviceModified* freeSpace1 = diskTree.searchDevice("Free space of offset 21465885056 and size 10000000");
    DeviceModified* freeSpace2 = diskTree.searchDevice("Free space of offset 23613400960 and size 10000000");
    DeviceModified* freeSpace3 = diskTree.searchDevice("Free space of offset 268426601344 and size 10000000");
    
    QCOMPARE(freeSpace1 == NULL, false);
    QCOMPARE(freeSpace2 == NULL, false);
    QCOMPARE(freeSpace3 == NULL, false);
}

#include "volumedetectiontest.moc"