/*
    Copyright 2012 Lisa Vitolo <shainer@chakra-project.org>
    
    This class has been inspired by the ExternalCommand class in partitionmanager
    Copyright (C) 2008 by Volker Lanz <vl@fidra.de> 

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
#include <solid/partitioner/externalcommand.h>
#include <stdlib.h>
#include <QtCore/QDebug>
#include <QtCore/QFile>

namespace Solid
{
namespace Partitioner
{
    
class ExternalCommand::Private
{
public:
    Private(const QString& c, const QStringList& a, const QString& p)
        : command(c)
        , args(a)
        , path(p)
    {}
    
    QString command;
    QStringList args;
    QString path;
    
    QString output;
    int exitCode;
};
    
ExternalCommand::ExternalCommand(const QString& cmd, const QStringList& args, const QString& path, QObject* parent)
    : QProcess(parent)
    , d( new Private(cmd, args, path) )
{
    connect(this, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(addOutput()));

    setProcessChannelMode(MergedChannels);
}

ExternalCommand::~ExternalCommand()
{
    delete d;
}

bool ExternalCommand::run()
{    
    foreach (const QString& path, d->path.split(":")) {
        QString cmd = d->command;
        cmd.prepend(path + "/");
        
        if (QFile::exists(cmd)) {
            d->command = cmd;
        }
    }
    
    start(d->command, d->args);
    
    if (!waitForStarted()) {
        return false;
    }
    
    closeWriteChannel();
    
    if (!waitForFinished()) {
        return false;
    }
    
    addOutput();
    return true;
}

int ExternalCommand::exitCode() const
{
    return d->exitCode;
}

QString ExternalCommand::output() const
{
    return d->output;
}

void ExternalCommand::onFinished(int exitStatus)
{
    d->exitCode = exitStatus;
}

void ExternalCommand::addOutput()
{
    QString output = QString( readAllStandardOutput() );
    d->output += output;
}

#include "externalcommand.moc"

}
}