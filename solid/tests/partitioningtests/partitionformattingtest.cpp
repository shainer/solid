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
#include <tests/partitioningtests/partitionformattingtest.h>
#include <solid/managerbase_p.h>
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/partitioner/volumemanager.h>
#include <solid/partitioner/actions/formatpartitionaction.h>

#include <QtTest/QTest>
#include <QtCore/QDebug>

#include <stdlib.h>

#ifndef FAKE_PARTITIONS_XML
    #error "FAKE_PARTITIONS_XML not set. An XML file describing disks and partitions is required for this test"
#endif

QTEST_MAIN(PartitionFormattingTest)

using namespace Solid::Partitioner;
using namespace Solid::Partitioner::Actions;

void PartitionFormattingTest::initTestCase()
{
    setenv("SOLID_FAKEHW", FAKE_PARTITIONS_XML, 1);
    Solid::ManagerBasePrivate *manager
        = dynamic_cast<Solid::ManagerBasePrivate*>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager*>(manager->managerBackends().first());
}

/*
 * TODO: test the existence and correctness of filesystem flags and names.
 */
void PartitionFormattingTest::test()
{
    VolumeManager* manager = VolumeManager::instance();
    FormatPartitionAction* action1 = new FormatPartitionAction("/org/kde/solid/fakehw/foreign_logical", "fat32");
    FormatPartitionAction* action2 = new FormatPartitionAction("/org/kde/solid/fakehw/extended_volume", "ntfs");
    
    /*
     * First test: valid formatting
     */
    manager->registerAction(action1);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::None);
    
    /*
     * NOTE: this device always exists and the cast doesn't fail, otherwise the previous instructions would have failed
     * at some point (either throught QCOMPARE, or with a segfault in registerAction()
     */
    DeviceModified* foreign = manager->diskTree("/org/kde/solid/fakehw/storage_serial_HD56890I").searchDevice("/org/kde/solid/fakehw/foreign_logical");
    Partition* p = dynamic_cast< Partition* >(foreign);
    QCOMPARE(p->filesystem().name(), QString("fat32")); /* check if the new filesystem has been set to this partition */
    
    /*
     * This action isn't legal, as you can't format an extended partition.
     */
    manager->registerAction(action2);
    QCOMPARE(manager->error().type(), Utils::PartitioningError::CannotFormatPartition);
}

#include "partitionformattingtest.moc"