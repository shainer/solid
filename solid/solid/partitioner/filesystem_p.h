#ifndef SOLID_PARTITIONER_FILESYSTEM_PH
#define SOLID_PARTITIONER_FILESYSTEM_PH

#include <QtCore/QSharedData>
#include <QtCore/QStringList>

namespace Solid
{
    namespace Partitioner
    {
        class FilesystemPrivate : public QSharedData
        {
        public:
            FilesystemPrivate(const QString& n, const QStringList& flags);
            FilesystemPrivate(const QString& n, const QString& l, int ouid, int ogid);
            FilesystemPrivate(const FilesystemPrivate& other);
            ~FilesystemPrivate();

            QString name;
            QString label;
            int ownerUid;
            int ownerGid;
            QStringList unsupported;
        };
    }
}

#endif
