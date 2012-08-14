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
#include <tests/partitioningtests/partitiontabletest.h>
#include <solid/managerbase_p.h>
#include <solid/devicenotifier.h>
#include <solid/partitioner/volumemanager.h>
#include <solid/partitioner/actions/createpartitiontableaction.h>

#include <QtTest/QTest>
#include <QtCore/QDebug>

#ifndef FAKE_PARTITIONS_XML
    #error "FAKE_PARTITIONS_XML not set. An XML file describing disks and partitions is required for this test"
#endif

QTEST_MAIN(PartitionTableTest)

using namespace Solid::Partitioner;
using namespace Solid::Partitioner::Actions;

void PartitionTableTest::initTestCase()
{
    setenv("SOLID_FAKEHW", FAKE_PARTITIONS_XML, 1);
    Solid::ManagerBasePrivate *manager
        = dynamic_cast<Solid::ManagerBasePrivate*>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager*>(manager->managerBackends().first());
}

void PartitionTableTest::test()
{
    VolumeManager* manager = VolumeManager::instance();
    
    /*
     * Changing partition table scheme should automatically deletes all partitions present and start from scratch
     * with a huge free space block.
     */
    manager->registerAction( new CreatePartitionTableAction("/org/kde/solid/fakehw/storage_serial_HD56890I", Utils::GPTScheme) );
    QCOMPARE(manager->error().type(), PartitioningError::None);
    
    VolumeTree tree = manager->diskTree("/org/kde/solid/fakehw/storage_serial_HD56890I");
    QCOMPARE(tree.freeSpaceBlocks("/org/kde/solid/fakehw/storage_serial_HD56890I").size(), 1);
    QCOMPARE(tree.partitions().size(), 0);
}

#include "partitiontabletest.moc"
