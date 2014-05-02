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

#ifndef SOLID_CONSERVE_RESOURCES_JOB_H
#define SOLID_CONSERVE_RESOURCES_JOB_H

#include "solidjob.h"

namespace Solid
{
/**
 * When this job finishes it will return wether it is ok to performce
 * expensive tasks. One of the values taken into account for example is
 * wether the device is connected to the current or not.
 */
class ConserveResourcesJob : public Job
{
Q_OBJECT
public:
    explicit ConserveResourcesJob(QObject* parent = 0);
    virtual ~ConserveResourcesJob();

    virtual void start();

private Q_SLOTS:
    void doStart();
};
}
#endif //SOLID_CONSERVE_RESOURCES_JOB_H