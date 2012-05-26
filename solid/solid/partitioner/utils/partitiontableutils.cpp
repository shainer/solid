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
#include "partitiontableutils.h"
#include <kglobal.h>
#include <QtCore/QStringList>

namespace Solid
{
namespace Partitioner
{
namespace Utils
{

class PartitionTableUtils::Private
{
public:
    Private()
    {
        QStringList mbrFlags = QStringList() << "boot";
        QStringList gptFlags = QStringList() << "required";
        QStringList apmFlags = QStringList() << "allocated"
                                             << "allow_read"
                                             << "allow_write"
                                             << "boot"
                                             << "boot_code_is_pic"
                                             << "in_use";

        supportedFlags.insert(MBR, mbrFlags);
        supportedFlags.insert(GPT, gptFlags);
        supportedFlags.insert(APM, apmFlags);
    }

    ~Private()
    {}

    QHash< PTableType, QStringList > supportedFlags;
};

class PartitionTableUtilsHelper
{
public:
    PartitionTableUtilsHelper()
        : q(0)
    {}

    ~PartitionTableUtilsHelper()
    {
        delete q;
    }

    PartitionTableUtils* q;
};

K_GLOBAL_STATIC(PartitionTableUtilsHelper, s_ptableutils);

PartitionTableUtils::PartitionTableUtils()
    : d( new Private )
{
    Q_ASSERT(!s_ptableutils->q);
    s_ptableutils->q = this;
}

PartitionTableUtils::~PartitionTableUtils()
{
    delete d;
}

PartitionTableUtils* PartitionTableUtils::instance()
{
    if (!s_ptableutils->q) {
        new PartitionTableUtils;
    }

    return s_ptableutils->q;
}

QStringList PartitionTableUtils::supportedFlags(PTableType scheme)
{
    return d->supportedFlags.value(scheme);
}

}
}
}
