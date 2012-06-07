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

#include "filesystem.h"

namespace Solid
{
namespace Partitioner
{
namespace Utils
{

class Filesystem::Private
{
public:
    Private(const QString& n, const QStringList& flags)
        : name(n)
    {
        foreach (const QString& flag, flags) {
            if (flag.startsWith("label")) {
                label = flag.split("=").last();
            }
            else if (flag.startsWith("take_ownership_uid")) {
                ownerUid = flag.split("=").last().toUInt();
            }
            else if (flag.startsWith("take_ownership_gid")) {
                ownerGid = flag.split("=").last().toUInt();
            }
            else {
                unsupported.append( flag.split("=").first() );
            }
        }
    }

    Private(const QString& n, const QString& l, uid_t ouid, gid_t ogid)
        : name(n)
        , label(l)
        , ownerUid(ouid)
        , ownerGid(ogid)
    {}

    ~Private()
    {}

    QString name;
    QString label;
    uid_t ownerUid;
    gid_t ownerGid;
    QStringList unsupported;
};

Filesystem::Filesystem(const QString &name, const QStringList &flags)
    : d( new Private(name, flags) )
{}

Filesystem::Filesystem(const QString& name, const QString& label, uid_t ownerUid, gid_t ownerGid)
    : d( new Private(name, label, ownerUid, ownerGid) )
{}

Filesystem::Filesystem()
    : d( new Private(QString(), QStringList()) )
{}

Filesystem::Filesystem(const Filesystem &other)
    : d( new Private(other.name(), other.label(), other.ownerUid(), other.ownerGid()) )
{}

Filesystem::~Filesystem()
{
    delete d;
}

QString Filesystem::name() const
{
    return d->name;
}

QString Filesystem::label() const
{
    return d->label;
}

uid_t Filesystem::ownerUid() const
{
    return d->ownerUid;
}

gid_t Filesystem::ownerGid() const
{
    return d->ownerGid;
}

QStringList Filesystem::flags() const
{
    QStringList fsOptions;
    fsOptions.append( QString("label=") + d->label );
    
    if (d->ownerUid != -1) {
        fsOptions.append( QString("take_ownership_uid=") + QString::number(d->ownerUid) );
    }
    if (d->ownerGid != -1) {
        fsOptions.append( QString("take_ownership_gid=") + QString::number(d->ownerGid) );
    }
    
    return fsOptions;
}

QStringList Filesystem::unsupportedFlags() const
{
    return d->unsupported;
}

}
}
}
