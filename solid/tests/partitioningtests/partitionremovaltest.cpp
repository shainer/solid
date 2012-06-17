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
#include <tests/partitioningtests/partitionremovaltest.h>
#include <solid/partitioner/actions/removepartitionaction.h>
#include <solid/partitioner/volumemanager.h>
#include <solid/managerbase_p.h>
#include <solid/devicenotifier.h>

#include <QtTest/QTest>
#include <QtCore/QDebug>

#ifndef FAKE_PARTITIONS_XML
    #error "FAKE_PARTITIONS_XML not set. An XML file describing disks and partitions is required for this test"
#endif

QTEST_MAIN(PartitionRemovalTest)

using namespace Solid::Partitioner;
using namespace Solid::Partitioner::Actions;

void PartitionRemovalTest::initTestCase()
{
    setenv("SOLID_FAKEHW", FAKE_PARTITIONS_XML, 1);
    Solid::ManagerBasePrivate *manager
        = dynamic_cast<Solid::ManagerBasePrivate*>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager*>(manager->managerBackends().first());
}

void PartitionRemovalTest::test()
{
    VolumeManager* manager = VolumeManager::instance();
    RemovePartitionAction* mountedRemoval = new RemovePartitionAction("/org/kde/solid/fakehw/foreign_logical");
    RemovePartitionAction* ignoredRemoval = new RemovePartitionAction("/org/kde/solid/fakehw/swap_volume");
    RemovePartitionAction* good1 = new RemovePartitionAction("/org/kde/solid/fakehw/home_volume");
    RemovePartitionAction* good2 = new RemovePartitionAction("/org/kde/solid/fakehw/root_volume");
    RemovePartitionAction* extended = new RemovePartitionAction("/org/kde/solid/fakehw/extended_volume");
    
    /* You cannot remove a mounted partition */
    manager->registerAction(mountedRemoval);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::MountedPartitionError);
    
    /* Ignored partitions exist for special uses only, so same thing */
    manager->registerAction(ignoredRemoval);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::IgnoredPartitionError);
    
    /*
     * This is the removal of a logical partition, which means the merge is more complicated due to the reserved
     * space for EBR entries.
     */
    manager->registerAction(good1);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
    
    VolumeTree diskTree = manager->diskTree("/org/kde/solid/fakehw/storage_serial_HD56890I");
    QCOMPARE(diskTree.searchDevice("Free space of offset 23623300960 and size 223338463904") == NULL, false);
    
    /*
     * This removes a primary partition, but we still check the merge result.
     */
    manager->registerAction(good2);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
    QCOMPARE(diskTree.searchDevice("Free space of offset 1048576 and size 21474836480") == NULL, false);
    
    /* Removes the extended and checks if the logicals are removed correctly */
    manager->registerAction(extended);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::MountedLogicalError);
}

#include "partitionremovaltest.moc" 
