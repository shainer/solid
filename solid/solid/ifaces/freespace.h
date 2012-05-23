#ifndef SOLID_IFACES_DEVICE_H
#define SOLID_IFACES_DEVICE_H

#include <QtCore/QObject>
#include <solid/device.h>

namespace Solid
{
namespace Ifaces
{
    
    class FreeSpace : public QObject
    {
        Q_OBJECT
        
        explicit FreeSpace(QObject* parent = 0);
        virtual ~FreeSpace();
        
        virtual qulonglong offset() const =0;
        virtual qulonglong size() const =0;
        QString parentUdi() const =0;
    };
    
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::FreeSpace, "org.kde.Solid.Ifaces.FreeSpace/0.1")

#endif