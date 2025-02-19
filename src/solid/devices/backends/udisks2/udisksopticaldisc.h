/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010 - 2012 Lukáš Tinkl <ltinkl@redhat.com>

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

#ifndef UDISKS2OPTICALDISC_H
#define UDISKS2OPTICALDISC_H

#include <solid/devices/ifaces/opticaldisc.h>

#include "../shared/udevqt.h"

#include "udisksstoragevolume.h"
#include "udisksdevice.h"

namespace Solid
{
namespace Backends
{
namespace UDisks2
{

class OpticalDisc: public StorageVolume, virtual public Solid::Ifaces::OpticalDisc
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDisc)

public:
    OpticalDisc(Device *dev);
    virtual ~OpticalDisc();

    qulonglong capacity() const Q_DECL_OVERRIDE;
    bool isRewritable() const Q_DECL_OVERRIDE;
    bool isBlank() const Q_DECL_OVERRIDE;
    bool isAppendable() const Q_DECL_OVERRIDE;
    Solid::OpticalDisc::DiscType discType() const Q_DECL_OVERRIDE;
    Solid::OpticalDisc::ContentTypes availableContent() const Q_DECL_OVERRIDE;

    class Identity
    {
    public:
        Identity();
        Identity(const Device &device, const Device &drive);
        bool operator ==(const Identity &) const;

    private:
        long long m_detectTime;
        long long m_size;
        uint m_labelHash;
    };

private:

    mutable Identity m_identity;
    QString media() const;
    mutable Solid::OpticalDisc::ContentTypes m_cachedContent;
    Device *m_drive;
    UdevQt::Device m_udevDevice;
};

}
}
}
#endif // UDISKS2OPTICALDISC_H
