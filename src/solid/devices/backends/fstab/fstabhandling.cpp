/*
    Copyright 2006-2010 Kevin Ottens <ervin@kde.org>
    Copyright 2010 Mario Bensi <mbensi@ipsquad.net>

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

#include "fstabhandling.h"

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include <QtCore/QTime>

#include <solid/devices/soliddefs_p.h>

#include "solid/config-solid.h"
#include <stdlib.h>

#if HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif
#if HAVE_MNTENT_H
#include <mntent.h>
#elif defined(HAVE_SYS_MNTENT_H)
#include <sys/mntent.h>
#endif

// This is the *BSD branch
#if HAVE_SYS_MOUNT_H
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif

#ifdef Q_OS_SOLARIS
#define FSTAB "/etc/vfstab"
#else
#define FSTAB "/etc/fstab"
#endif

#if ! HAVE_GETMNTINFO
# ifdef _PATH_MOUNTED
// On some Linux, MNTTAB points to /etc/fstab !
#  undef MNTTAB
#  define MNTTAB _PATH_MOUNTED
# else
#  ifndef MNTTAB
#   ifdef MTAB_FILE
#    define MNTTAB MTAB_FILE
#   else
#    define MNTTAB "/etc/mnttab"
#   endif
#  endif
# endif
#endif

// There are (at least) four kind of APIs:
// setmntent + getmntent + struct mntent (linux...)
//             getmntent + struct mnttab
// mntctl                + struct vmount (AIX)
// getmntinfo + struct statfs&flags (BSD 4.4 and friends)
// getfsent + char* (BSD 4.3 and friends)

#if HAVE_SETMNTENT
#define SETMNTENT setmntent
#define ENDMNTENT endmntent
#define STRUCT_MNTENT struct mntent *
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) ((var = getmntent(file)) != 0)
#define MOUNTPOINT(var) var->mnt_dir
#define MOUNTTYPE(var) var->mnt_type
#define MOUNTOPTIONS(var) var->mnt_opts
#define FSNAME(var) var->mnt_fsname
#else
#define SETMNTENT fopen
#define ENDMNTENT fclose
#define STRUCT_MNTENT struct mnttab
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) (getmntent(file, &var) == 0)
#define MOUNTPOINT(var) var.mnt_mountp
#define MOUNTTYPE(var) var.mnt_fstype
#define MOUNTOPTIONS(var) var.mnt_mntopts
#define FSNAME(var) var.mnt_special
#endif

Q_GLOBAL_STATIC(Solid::Backends::Fstab::FstabHandling, globalFstabCache)

Solid::Backends::Fstab::FstabHandling::FstabHandling()
    : m_fstabCacheValid(false),
      m_mtabCacheValid(false)
{ }

bool _k_isFstabNetworkFileSystem(const QString &fstype, const QString &devName)
{
    if (fstype == "nfs"
            || fstype == "nfs4"
            || fstype == "smbfs"
            || fstype == "cifs"
            || devName.startsWith(QLatin1String("//"))) {
        return true;
    }
    return false;
}

void Solid::Backends::Fstab::FstabHandling::_k_updateFstabMountPointsCache()
{
    if (globalFstabCache->m_fstabCacheValid) {
        return;
    }

    globalFstabCache->m_fstabCache.clear();

#if HAVE_SETMNTENT

    FILE *fstab;
    if ((fstab = setmntent(FSTAB, "r")) == 0) {
        return;
    }

    struct mntent *fe;
    while ((fe = getmntent(fstab)) != 0) {
        if (_k_isFstabNetworkFileSystem(fe->mnt_type, fe->mnt_fsname)) {
            const QString device = QFile::decodeName(fe->mnt_fsname);
            const QString mountpoint = QFile::decodeName(fe->mnt_dir);

            globalFstabCache->m_fstabCache.insert(device, mountpoint);
        }
    }

    endmntent(fstab);

#else

    QFile fstab(FSTAB);
    if (!fstab.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream stream(&fstab);
    QString line;

    while (!stream.atEnd()) {
        line = stream.readLine().simplified();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        // not empty or commented out by '#'
        const QStringList items = line.split(' ');

#ifdef Q_OS_SOLARIS
        if (items.count() < 5) {
            continue;
        }
#else
        if (items.count() < 4) {
            continue;
        }
#endif
        //prevent accessing a blocking directory
        if (_k_isFstabNetworkFileSystem(items.at(2), items.at(0))) {
            const QString device = items.at(0);
            const QString mountpoint = items.at(1);

            globalFstabCache->m_fstabCache.insert(device, mountpoint);
        }
    }

    fstab.close();
#endif
    globalFstabCache->m_fstabCacheValid = true;
}

QStringList Solid::Backends::Fstab::FstabHandling::deviceList()
{
    _k_updateFstabMountPointsCache();
    _k_updateMtabMountPointsCache();

    QStringList devices = globalFstabCache->m_fstabCache.keys();
    devices += globalFstabCache->m_mtabCache.keys();
    devices.removeDuplicates();
    return devices;
}

QStringList Solid::Backends::Fstab::FstabHandling::mountPoints(const QString &device)
{
    _k_updateFstabMountPointsCache();
    _k_updateMtabMountPointsCache();

    QStringList mountpoints = globalFstabCache->m_fstabCache.values(device);
    mountpoints += globalFstabCache->m_mtabCache.values(device);
    mountpoints.removeDuplicates();
    return mountpoints;
}

QProcess *Solid::Backends::Fstab::FstabHandling::callSystemCommand(const QString &commandName,
        const QStringList &args,
        QObject *obj, const char *slot)
{
    QStringList env = QProcess::systemEnvironment();
    env.replaceInStrings(QRegExp("^PATH=(.*)", Qt::CaseInsensitive), "PATH=/sbin:/bin:/usr/sbin/:/usr/bin");

    QProcess *process = new QProcess(obj);

    QObject::connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
                     obj, slot);

    process->setEnvironment(env);
    process->start(commandName, args);

    if (process->waitForStarted()) {
        return process;
    } else {
        delete process;
        return 0;
    }
}

QProcess *Solid::Backends::Fstab::FstabHandling::callSystemCommand(const QString &commandName,
        const QString &device,
        QObject *obj, const char *slot)
{
    return callSystemCommand(commandName, QStringList() << device, obj, slot);
}

void Solid::Backends::Fstab::FstabHandling::_k_updateMtabMountPointsCache()
{
    if (globalFstabCache->m_mtabCacheValid) {
        return;
    }

    globalFstabCache->m_mtabCache.clear();

#if HAVE_GETMNTINFO

#if GETMNTINFO_USES_STATVFS
    struct statvfs *mounted;
#else
    struct statfs *mounted;
#endif

    int num_fs = getmntinfo(&mounted, MNT_NOWAIT);

    for (int i = 0; i < num_fs; i++) {
#ifdef __osf__
        QString type = QFile::decodeName(mnt_names[mounted[i].f_type]);
#else
        QString type = QFile::decodeName(mounted[i].f_fstypename);
#endif
        if (_k_isFstabNetworkFileSystem(type, QString())) {
            const QString device = QFile::decodeName(mounted[i].f_mntfromname);
            const QString mountpoint = QFile::decodeName(mounted[i].f_mntonname);
            globalFstabCache->m_mtabCache.insert(device, mountpoint);
        }
    }

#elif defined(_AIX)

    struct vmount *mntctl_buffer;
    struct vmount *vm;
    char *mountedfrom;
    char *mountedto;
    int fsname_len, num;
    int buf_sz = 4096;

    mntctl_buffer = (struct vmount *)malloc(buf_sz);
    num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    if (num == 0) {
        buf_sz = *(int *)mntctl_buffer;
        free(mntctl_buffer);
        mntctl_buffer = (struct vmount *)malloc(buf_sz);
        num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    }

    if (num > 0) {
        /* iterate through items in the vmount structure: */
        vm = (struct vmount *)mntctl_buffer;
        for (; num > 0; --num) {
            /* get the name of the mounted file systems: */
            fsname_len = vmt2datasize(vm, VMT_STUB);
            mountedto     = (char *)malloc(fsname_len + 1);
            mountedto[fsname_len] = '\0';
            strncpy(mountedto, (char *)vmt2dataptr(vm, VMT_STUB), fsname_len);

            fsname_len = vmt2datasize(vm, VMT_OBJECT);
            mountedfrom     = (char *)malloc(fsname_len + 1);
            mountedfrom[fsname_len] = '\0';
            strncpy(mountedfrom, (char *)vmt2dataptr(vm, VMT_OBJECT), fsname_len);

            /* Look up the string for the file system type,
             * as listed in /etc/vfs.
             * ex.: nfs,jfs,afs,cdrfs,sfs,cachefs,nfs3,autofs
             */
            struct vfs_ent *ent = getvfsbytype(vm->vmt_gfstype);

            QString type = QFile::decodeName(ent->vfsent_name);
            if (_k_isFstabNetworkFileSystem(type, QString())) {
                const QString device = QFile::decodeName(mountedfrom);
                const QString mountpoint = QFile::decodeName(mountedto);
                globalFstabCache->m_mtabCache.insert(device, mountpoint);
            }

            free(mountedfrom);
            free(mountedto);

            /* goto the next vmount structure: */
            vm = (struct vmount *)((char *)vm + vm->vmt_length);
        }

        endvfsent();
    }

    free(mntctl_buffer);
#else
    STRUCT_SETMNTENT mnttab;
    if ((mnttab = SETMNTENT(MNTTAB, "r")) == 0) {
        return;
    }

    STRUCT_MNTENT fe;
    while (GETMNTENT(mnttab, fe)) {
        QString type = QFile::decodeName(MOUNTTYPE(fe));
        if (_k_isFstabNetworkFileSystem(type, QString())) {
            const QString device = QFile::decodeName(FSNAME(fe));
            const QString mountpoint = QFile::decodeName(MOUNTPOINT(fe));
            globalFstabCache->m_mtabCache.insert(device, mountpoint);
        }
    }
    ENDMNTENT(mnttab);
#endif

    globalFstabCache->m_mtabCacheValid = true;
}

QStringList Solid::Backends::Fstab::FstabHandling::currentMountPoints(const QString &device)
{
    _k_updateMtabMountPointsCache();
    return globalFstabCache->m_mtabCache.values(device);
}

void Solid::Backends::Fstab::FstabHandling::flushMtabCache()
{
    globalFstabCache->m_mtabCacheValid = false;
}

void Solid::Backends::Fstab::FstabHandling::flushFstabCache()
{
    globalFstabCache->m_fstabCacheValid = false;
}
