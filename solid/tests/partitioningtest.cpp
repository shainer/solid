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
#include "partitioningtest.h"
#include <solid/managerbase_p.h>
#include <solid/devicenotifier.h>

#include <QtTest/QTest>
#include <QtCore/QDebug>

#include <stdlib.h>
#include <solid/device.h>
#include <solid/partitioner/volumemanager.h>

#ifndef FAKE_PARTITIONS_XML
    #error "FAKE_PARTITIONS_XML not set. An XML file describing disks and partitions is required for this test"
#endif

QTEST_MAIN(PartitioningTest)

using namespace Solid::Partitioner;

void PartitioningTest::initTestCase()
{
    setenv("SOLID_FAKEHW", FAKE_PARTITIONS_XML, 1);
    Solid::ManagerBasePrivate *manager
        = dynamic_cast<Solid::ManagerBasePrivate*>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager*>(manager->managerBackends().first());
}

void PartitioningTest::test()
{
    VolumeManager* manager = VolumeManager::instance();
    
    foreach (const VolumeTree& tree, manager->allDiskTrees().values()) {
        tree.print();
    }
}

#include "partitioningtest.moc"