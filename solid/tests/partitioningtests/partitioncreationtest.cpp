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
#include <tests/partitioningtests/partitioncreationtest.h>
#include <solid/managerbase_p.h>
#include <solid/devicenotifier.h>
#include <solid/partitioner/actions/createpartitionaction.h>
#include <solid/partitioner/volumemanager.h>

#include <QtTest/QTest>

#ifndef FAKE_PARTITIONS_XML
    #error "FAKE_PARTITIONS_XML not set. An XML file describing disks and partitions is required for this test"
#endif

QTEST_MAIN(PartitionCreationTest);

using namespace Solid::Partitioner;
using namespace Solid::Partitioner::Actions;

void PartitionCreationTest::initTestCase()
{
    setenv("SOLID_FAKEHW", FAKE_PARTITIONS_XML, 1);
    Solid::ManagerBasePrivate *manager
        = dynamic_cast<Solid::ManagerBasePrivate*>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager*>(manager->managerBackends().first());
}

void PartitionCreationTest::test()
{
    VolumeManager* manager = VolumeManager::instance();
    CreatePartitionAction* actionExtended = new CreatePartitionAction("/org/kde/solid/fakehw/storage_serial_HD56890I",
                                                                      21475875056,
                                                                      10000,
                                                                      true);
    CreatePartitionAction* actionWrongGeometry = new CreatePartitionAction("/org/kde/solid/fakehw/storage_serial_HD56890I",
                                                                           21475875056,
                                                                           50000,
                                                                           false);
    CreatePartitionAction* actionGood1 = new CreatePartitionAction("/org/kde/solid/fakehw/storage_serial_HD56890I",
                                                                   21475875056,
                                                                   5000);
    CreatePartitionAction* actionGood2 = new CreatePartitionAction("/org/kde/solid/fakehw/storage_serial_HD56890I",
                                                                   23623300960,
                                                                   132256);
    
    /* This action tries to create an extended partition when one is already present */
    manager->registerAction(actionExtended);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::BadPartitionTypeError);
    
    /* This action specifies an offset and size which doesn't fall inside a free space block */
    manager->registerAction(actionWrongGeometry);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::PartitionGeometryError);
    
    /* This action takes some space in a normal free blocks */
    manager->registerAction(actionGood1);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
    
    /* Check if the block has shrinked accordingly */
    VolumeTree diskTree = manager->diskTree("/org/kde/solid/fakehw/storage_serial_HD56890I");
    QCOMPARE(diskTree.searchDevice("Free space of offset 21475880056 and size 5000") == NULL, false);
    
    /* This action takes all the space in a free block between logical partitions */
    manager->registerAction(actionGood2);
    
    /* Check if the free space block was actually deleted */
    int count = 0;
    foreach (DeviceModified* dev, diskTree.logicalPartitions(true)) {
        if (dev->deviceType() == DeviceModified::FreeSpaceDevice) {
            count++;
        }
    }
    
    QCOMPARE(count, 1);
}

#include "partitioncreationtest.moc"