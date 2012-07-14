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
    {}

    ~Private()
    {}

    QHash< QString, QStringList > supportedFlags;
    QHash< QString, QHash<QString, QString> > types;
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
    
    QStringList mbrFlags = QStringList() << "boot";
    QStringList gptFlags = QStringList() << "required";
    QStringList apmFlags = QStringList() << "allocated"
                                            << "allow_read"
                                            << "allow_write"
                                            << "boot"
                                            << "boot_code_is_pic"
                                            << "in_use";

    d->supportedFlags.insert("mbr", mbrFlags);
    d->supportedFlags.insert("gpt", gptFlags);
    d->supportedFlags.insert("apm", apmFlags);
    
    /*
     * FIXME: for MBR, check CHS vs. LBA.
     * Use the property KnownFilesystem, when implemented.
     */
    QHash< QString, QString > apm;
    apm.insert("btrfs", "Apple_Unix_SVR2");
    apm.insert("ext2", "Apple_Unix_SVR2");
    apm.insert("ext3", "Apple_Unix_SVR2");
    apm.insert("ext4", "Apple_Unix_SVR2");
    apm.insert("minix", "Apple_Unix_SVR2");
    apm.insert("nilfs2", "Apple_Unix_SVR2");
    apm.insert("ntfs", "Apple_Unix_SVR2");
    apm.insert("reiserfs", "Apple_Unix_SVR2");
    apm.insert("swap", "Apple_Unix_SVR2");
    apm.insert("unformatted", "Apple_Scratch");
    apm.insert("xfs", "Apple_Unix_SVR2");
    apm.insert("vfat", "Apple_Unix_SVR2");
    apm.insert("bfs", "Be_BFS");
    
    QHash< QString, QString > gpt;
    gpt.insert("btrfs", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("ext2", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("ext3", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("ext4", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("minix", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("nilfs2", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("ntfs", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("reiserfs", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("swap", "0657FD6D-A4AB-43C4-84E5-0933C84B4F4F");
    gpt.insert("unformatted", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("vfat", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");
    gpt.insert("xfs", "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7");

    QHash< QString, QString > mbr;
    mbr.insert("btrfs", "0x83");
    mbr.insert("extended", "0x05");
    mbr.insert("ext2", "0x83");
    mbr.insert("ext3", "0x83");
    mbr.insert("ext4", "0x83");
    mbr.insert("minix", "0x81");
    mbr.insert("nilfs2", "0x83");
    mbr.insert("ntfs", "0x07");
    mbr.insert("reiserfs", "0x83");
    mbr.insert("swap", "0x82");
    mbr.insert("unformatted", "0x00");
    mbr.insert("vfat", "0x0b");
    mbr.insert("xfs", "0x83");
    
    d->types.insert("apm", apm);
    d->types.insert("gpt", gpt);
    d->types.insert("mbr", mbr);
    
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

QStringList PartitionTableUtils::supportedFlags(const QString& scheme)
{
    return d->supportedFlags.value(scheme);
}

QString PartitionTableUtils::typeString(const QString& scheme, QString type)
{
    if (type.isEmpty()) {
        type = "unformatted";
    }
    
    QHash<QString, QString> schemeTypes = d->types[scheme];
    return schemeTypes[type];
}

}
}
}
