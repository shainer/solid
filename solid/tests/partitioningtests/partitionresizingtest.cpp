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
#include <tests/partitioningtests/partitionresizingtest.h>
#include <solid/managerbase_p.h>
#include <solid/devicenotifier.h>
#include <solid/partitioner/actions/resizepartitionaction.h>
#include <solid/partitioner/volumemanager.h>

#include <QtTest/QTest>
#include <QtCore/QDebug>

#ifndef FAKE_PARTITIONS_XML
    #error "FAKE_PARTITIONS_XML not set. An XML file describing disks and partitions is required for this test"
#endif

QTEST_MAIN(PartitionResizingTest)

using namespace Solid::Partitioner;
using namespace Solid::Partitioner::Actions;

void PartitionResizingTest::initTestCase()
{
    setenv("SOLID_FAKEHW", FAKE_PARTITIONS_XML, 1);
    Solid::ManagerBasePrivate *manager
        = dynamic_cast<Solid::ManagerBasePrivate*>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager*>(manager->managerBackends().first());
}

/*
 * Action name conventions: B = backward, F = forward, so offsetBsizeF means a resizing action that for the partition
 * takes the offset backward and the size forward. The suffix Bad means the specific action should trigger an out of bounds
 * error.
 */
void PartitionResizingTest::test()
{
    VolumeManager* manager = VolumeManager::instance();
    VolumeTree tree = manager->diskTree("/org/kde/solid/fakehw/storage_serial_HD56890I");
    Actions::ResizePartitionAction* offsetBBad = new ResizePartitionAction("/org/kde/solid/fakehw/root_volume",
                                                                           1000000,
                                                                           21464836480);
    Actions::ResizePartitionAction* extendedResizing = new ResizePartitionAction("/org/kde/solid/fakehw/extended_volume",
                                                                                 21475885056,
                                                                                 246950716288);
    Actions::ResizePartitionAction* offsetBsizeF = new ResizePartitionAction("/org/kde/solid/fakehw/home_volume",
                                                                             23623333216,
                                                                             223338399392);
    Actions::ResizePartitionAction* offsetBsizeFBad = new ResizePartitionAction("/org/kde/solid/fakehw/home_volume",
                                                                                23623333216,
                                                                                223338399492);
    Actions::ResizePartitionAction* offsetB = new ResizePartitionAction("/org/kde/solid/fakehw/home_volume",
                                                                        23613433216,
                                                                        223338399392);
    Actions::ResizePartitionAction* sizeB = new ResizePartitionAction("/org/kde/solid/fakehw/home_volume",
                                                                      23613433216,
                                                                      223338299392);
    Actions::ResizePartitionAction* mounted = new ResizePartitionAction("/org/kde/solid/fakehw/foreign_logical",
                                                                        246971764864,
                                                                        21464836480);
    Actions::ResizePartitionAction* offsetF = new ResizePartitionAction("/org/kde/solid/fakehw/root_volume",
                                                                        11048576,
                                                                        21464836480);
    Actions::ResizePartitionAction* offsetFBad = new ResizePartitionAction("/org/kde/solid/fakehw/root_volume",
                                                                           12048576,
                                                                           21464836480);
    Actions::ResizePartitionAction* offsetBsizeB = new ResizePartitionAction("/org/kde/solid/fakehw/root_volume",
                                                                             10048576,
                                                                             21463836480);
    Actions::ResizePartitionAction* offsetFsizeB = new ResizePartitionAction("/org/kde/solid/fakehw/swap_volume",
                                                                             21476917312,
                                                                             2136483648);
    Actions::ResizePartitionAction* offsetFsizeBBad = new ResizePartitionAction("/org/kde/solid/fakehw/swap_volume",
                                                                                21477917312,
                                                                                 2136383648);
    Actions::ResizePartitionAction* offsetFsizeFBad = new ResizePartitionAction("/org/kde/solid/fakehw/home_volume",
                                                                                23618433216,
                                                                                223353299392);
    Actions::ResizePartitionAction* offsetFsizeF = new ResizePartitionAction("/org/kde/solid/fakehw/home_volume",
                                                                             23618433216,
                                                                             223343299392);
    
    manager->registerAction(offsetBBad);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::ResizeOutOfBoundsError);
    
    manager->registerAction(extendedResizing);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::ExtendedResizingError);
    
    manager->registerAction(offsetBsizeF);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
    
    manager->registerAction(offsetBsizeFBad);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::ResizeOutOfBoundsError);
    
    manager->registerAction(offsetB);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
    
    manager->registerAction(sizeB);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);

    manager->registerAction(mounted);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::MountedPartitionError);
    
    manager->registerAction(offsetF);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
    
    manager->registerAction(offsetFBad);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::ResizeOutOfBoundsError);
    
    manager->registerAction(offsetBsizeB);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
    
    manager->registerAction(offsetFsizeB);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
    
    manager->registerAction(offsetFsizeBBad);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::ResizeOutOfBoundsError);
    
    manager->registerAction(offsetFsizeFBad);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::ResizeOutOfBoundsError);
    
    manager->registerAction(offsetFsizeF);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
}


#include "partitionresizingtest.moc" 
