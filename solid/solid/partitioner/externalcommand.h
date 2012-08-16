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
#ifndef SOLID_PARTITIONER_UTILS_EXTERNALCOMMAND
#define SOLID_PARTITIONER_UTILS_EXTERNALCOMMAND

#include <QtCore/QProcess>

namespace Solid
{
    namespace Partitioner
    {
        /**
         * @class ExternalCommand
         * @extends QProcess
         * @brief This class executes a command with arguments in a separate process, collecting its output.
         */
        class ExternalCommand : public QProcess
        {
            Q_OBJECT
            
        public:
            /**
             * Builds an object representing a command to be executed separately.
             * 
             * @param cmd the command name.
             * @param args a list of arguments (empty by default).
             * @param path the PATH environment variable (unavailable from inside the helper).
             * @param parent the QObject parent.
             */
            explicit ExternalCommand(const QString& cmd,
                                     const QStringList& args,
                                     const QString& path,
                                     QObject* parent = 0);
            
            /**
             * Deconstructor.
             */
            virtual ~ExternalCommand();
            
            /**
             * Executes the command synchronously.
             * 
             * @returns false if something goes wrong while calling the process, true otherwise.
             */
            bool run();
            
            /**
             * @returns the process' exit code.
             */
            int exitCode() const;
            
            /**
             * @returns a string containing all the process output.
             */
            QString output() const;
            
        private slots:
            void onFinished(int);
            void addOutput();
            
        private:
            class Private;
            Private* d;
        };
    }
}

#endif