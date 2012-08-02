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
#ifndef SOLID_PARTITIONER_DEVICES_DEVICEMODIFIED_H
#define SOLID_PARTITIONER_DEVICES_DEVICEMODIFIED_H

#include <solid/deviceinterface.h>

namespace Solid
{
    namespace Partitioner
    {   
        namespace Devices
        {
            /**
             * @class DeviceModified
             * 
             * This is an abstract base class to represent a device in use by the partitioning module. This means each
             * property the device stores may be changed in any time by the module.
             */
            class SOLID_EXPORT DeviceModified
            {
            public:
                
                /**
                 * @enum DeviceModifiedType
                 * 
                 * This enum shows the most specific type of the device. 
                 */
                enum DeviceModifiedType
                {
                    DiskDevice,
                    PartitionDevice,
                    FreeSpaceDevice
                };
                
                /**
                 * Creates a new object taking the initial values of some properties from the corresponding DeviceInterface.
                 * 
                 * @param iface the device interface provided by Solid.
                 */
                explicit DeviceModified(DeviceInterface *);
                
                /**
                 * Creates a new object not related to any existing interface. This can mean the device doesn't exist
                 * currently in the system.
                 */
                DeviceModified();
                
                /**
                 * Destructor. The device interface isn't deallocated.
                 */
                virtual ~DeviceModified();
                
                /**
                 * @returns the most specific type of the object.
                 */
                virtual DeviceModifiedType deviceType() const = 0;
                
                /**
                 * @returns a dynamically allocated copy of this device.
                 */
                virtual DeviceModified* copy() const = 0;
                
                /**
                 * @returns the initial offset of the device in bytes.
                 */
                virtual qulonglong offset() const = 0;
                
                /**
                 * @returns the size of the device in bytes.
                 */
                virtual qulonglong size() const = 0;
                
                /**
                 * @returns offset() + size() for each device.
                 */
                virtual qulonglong rightBoundary() const = 0;
                
                /**
                 * @returns true if the device interface the object was built from represented a valid device,
                 * false otherwise. For object not created with an interface, it returns true.
                 */
                virtual bool existent() const;
                
                /**
                 * @returns the name associated to the device.
                 */
                virtual QString name() const;
                
                /**
                 * Retrieves a readable description for the device. In most cases this description is identical to the name.
                 * However, when a device's offset and size are included in the name, the name uniquely identifies the device
                 * only when those are left in bytes: string representations are nicer to look at but with rounding we can end
                 * with two devices showing the same initial offset and size, therefore the name won't be unique anymore.
                 * 
                 * To summarize, name() shows offset/size in bytes, to maintain uniqueness. A description() aims to be readable
                 * by the user so it converts them to a nice string representation.
                 * 
                 * @returns a readable description of the device.
                 */
                virtual QString description() const;
                
                /**
                 * @returns the name of the parent object, or an empty string if there isn't any parent.
                 */
                virtual QString parentName() const;
                
                /**
                 * Sets a new size.
                 * 
                 * @param size the new size in bytes.
                 */
                virtual void setSize(qulonglong) = 0;
                
                /**
                 * Sets a new initial offset.
                 * 
                 * @param offset the initial offset in bytes.
                 */
                virtual void setOffset(qulonglong) = 0;
                
                /**
                 * Comparison operator.
                 * 
                 * @returns true if both devices share the same UDI, false otherwise.
                 */
                virtual bool operator==(const DeviceModified &) const;
                
                /**
                 * Sets a new parent name for the device.
                 * 
                 * @param udi the new parent UDI.
                 */
                virtual void setParentName(const QString &);
                
            protected:
                             
                /**
                 * Sets a new name for the device.
                 * 
                 * @param udi the new UDI.
                 */
                virtual void setName(const QString &);
                
                /**
                 * Sets a new description for this device.
                 * 
                 * @param description the new description
                 */
                virtual void setDescription(const QString &);
                
            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif
