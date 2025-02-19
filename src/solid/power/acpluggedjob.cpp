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

#include "acpluggedjob.h"
#include "acpluggedjob_p.h"
#include "powerbackendloader.h"
#include "backends/abstractacpluggedjob.h"

#include <QDebug>

using namespace Solid;

AcPluggedJobPrivate::AcPluggedJobPrivate()
{
    backendJob = Q_NULLPTR;
    plugged = false;
    backendJobFinished = false;
}

AcPluggedJob::AcPluggedJob(QObject* parent) : Job(*new AcPluggedJobPrivate(), parent)
{
}

void AcPluggedJob::doStart()
{
    Q_D(AcPluggedJob);
    d->backendJob = PowerBackendLoader::AcPluggedJob();
    connect(d->backendJob, &AbstractAcPluggedJob::result, [this, d]() {
        d->backendJobFinished = true;
        d->plugged = d->backendJob->isPlugged();
        emitResult();
    });

    d->backendJob->start();
}

bool AcPluggedJob::isPlugged() const
{
    if(d_func()->backendJobFinished) {
        return d_func()->plugged;
    }
    qWarning() << "isPlugged called without having called start";
    return false;
}
