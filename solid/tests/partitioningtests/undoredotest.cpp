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
#include <tests/partitioningtests/undoredotest.h>
#include <solid/partitioner/actions/removepartitionaction.h>
#include <solid/partitioner/actions/formatpartitionaction.h>
#include <solid/partitioner/volumemanager.h>
#include <solid/managerbase_p.h>
#include <solid/devicenotifier.h>

#include <QtTest/QTest>
#include <QtCore/QDebug>

#ifndef FAKE_PARTITIONS_XML
    #error "FAKE_PARTITIONS_XML not set. An XML file describing disks and partitions is required for this test"
#endif

QTEST_MAIN(UndoRedoTest)

using namespace Solid::Partitioner;
using namespace Solid::Partitioner::Actions;

void UndoRedoTest::initTestCase()
{
    setenv("SOLID_FAKEHW", FAKE_PARTITIONS_XML, 1);
    Solid::ManagerBasePrivate *manager
        = dynamic_cast<Solid::ManagerBasePrivate*>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager*>(manager->managerBackends().first());
}

void UndoRedoTest::test()
{
    VolumeManager* manager = VolumeManager::instance();
    VolumeTree diskTree = manager->diskTree("/org/kde/solid/fakehw/storage_serial_HD56890I");
    RemovePartitionAction* action1 = new RemovePartitionAction("/org/kde/solid/fakehw/root_volume");
    FormatPartitionAction* action2 = new FormatPartitionAction("/org/kde/solid/fakehw/home_volume", "NTFS");
    
    QCOMPARE(manager->isRedoPossible(), false);
    
    /*
     * The QCOMPARE lines for manager->error() shouldn't be here, but they save trouble if the partitions chosen for
     * the tests cannot be removed/formatted for some reason.
     */
    manager->registerAction(action1);
    QCOMPARE(manager->error().type(), PartitioningError::None);
    manager->registerAction(action2);
    QCOMPARE(manager->error().type(), PartitioningError::None);
    
    manager->undo();
    QCOMPARE(manager->isRedoPossible(), true);
    QCOMPARE(manager->isUndoPossible(), true);
    checkHomeFilesystem("XFS");
    checkRootExistence(false);
    
    manager->undo();
    QCOMPARE(manager->isUndoPossible(), false);
    checkHomeFilesystem("XFS");
    checkRootExistence(true);
    
    manager->redo();
    manager->redo();
    QCOMPARE(manager->isRedoPossible(), false);
    checkHomeFilesystem("NTFS");
    checkRootExistence(false);
    
    manager->undo();
    checkHomeFilesystem("XFS");
    checkRootExistence(false);
    
    manager->redo();
    checkHomeFilesystem("NTFS");
    checkRootExistence(false);    
}

void UndoRedoTest::checkHomeFilesystem(const QString& fsName)
{
    VolumeTree diskTree = VolumeManager::instance()->diskTree("/org/kde/solid/fakehw/storage_serial_HD56890I");
    DeviceModified* devHome = diskTree.searchDevice("/org/kde/solid/fakehw/home_volume");
    QCOMPARE(devHome == NULL, false);
    
    Partition* home = dynamic_cast< Partition* >(devHome);
    QCOMPARE(home->filesystem().name(), fsName);
}

void UndoRedoTest::checkRootExistence(bool mustExist)
{
    VolumeTree diskTree = VolumeManager::instance()->diskTree("/org/kde/solid/fakehw/storage_serial_HD56890I");
    DeviceModified* devForeign = diskTree.searchDevice("/org/kde/solid/fakehw/root_volume");
    
    QCOMPARE(devForeign != NULL, mustExist);
}

#include "undoredotest.moc" 