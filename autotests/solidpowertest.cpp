/*
    Copyright 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

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

#include <QObject>
#include <QTest>
#include <Solid/PowerManagement>

using namespace Solid;
class SolidPowerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testConserveResources();
    void testSleepStates();
    void testSupressSleep();
    void testConserveResourceChanged();
    void testAcPlugged();
    void testAcPluggedChanged();
};

void SolidPowerTest::testConserveResources()
{
      ConserveResourcesJob *job = PowerManagement::conserveResources();
      QVERIFY(job->exec());
}

void SolidPowerTest::testSleepStates()
{
    SleepStatesJob job = PowerManagement::sleeStates();
    job->exec();
    QCOMPARE(job->result(), PowerManagement::SuspendState);
}

void SolidPowerTest::testSupressSleep()
{
    SupressSleepJob job = PowerManagement::supressSleep(QLatin1Literal("No sleep for ya"));
    job->exec();
    QVERIFY(!job->isError());
}

void SolidPowerTest::testConserveResourceChanged()
{
//     (PowerManagement::self()
//     QSignalSpy...
}

void SolidPowerTest::testAcPlugged()
{
    AcPluggedJob *job = PowerManagement::acPlugged();
    job->exec();

    QVERIFY(!job->isError());
}

void SolidPowerTest::testAcPluggedChanged()
{
    PowerManagement* power = PowerManagement::self();
    QSignalSpy(power, acPluggedChanged);
}

QTEST_MAIN(SolidPowerTest)