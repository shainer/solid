/*
    Copyright 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

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

#ifndef SOLID_JOB_H
#define SOLID_JOB_H

#include <QObject>

namespace Solid
{
class JobPrivate;
/**
 * This class represents an asynchronous job performed by Solid,
 * tt is usually not used directly but instead it is inherit by some
 * other class, for example \See ConserveResourcesJob or \See SleepStatsJob
 *
 * There are two ways of using this class, one is via exec() which will block
 * the thread until a result is fetched, the other is via connecting to the
 * signal \See finished
 *
 * @note: SolidJob and its subclasses are meant to be used
 * in a fire-and-forget way. Jobs will delete themselves
 * when they finish using deleteLater() (although this
 * behaviour can be changed), so a job instance will
 * disappear after the next event loop run.
 */
class Job : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int error READ error NOTIFY result)

public:
    explicit Job(QObject* parent = 0);
    virtual ~Job();

    /**
     * Starts the job asynchronously.
     *
     * When the job is finished, result() is emitted.
     *
     * Warning: Never implement any synchronous workload in this method. This method
     * should just trigger the job startup, not do any work itself. It is expected to
     * be non-blocking.
     *
     * This is the method all subclasses need to implement.
     * It should setup and trigger the workload of the job. It should not do any
     * work itself. This includes all signals and terminating the job, e.g. by
     * emitResult(). The workload, which could be another method of the
     * subclass, is to be triggered using the event loop, e.g. by code like:
     * \code
     * void ExampleJob::start()
     * {
     *  QMetaObject::invokeMethod(this, "doWork", Qt::QueuedConnection)
     * }
     * \endcode
     */
    Q_SCRIPTABLE virtual void start() = 0;

    /**
     * Executes the job synchronously.
     *
     * This will start a nested QEventLoop internally. Nested event loop can be dangerous and
     * can have unintended side effects, you should avoid calling exec() whenever you can and use the
     * asynchronous interface of SolidJob instead.
     *
     * Should you indeed call this method, you need to make sure that all callers are reentrant,
     * so that events delivered by the inner event loop don't cause non-reentrant functions to be
     * called, which usually wreaks havoc.
     *
     * Note that the event loop started by this method does not process user input events, which means
     * your user interface will effectivly be blocked. Other events like paint or network events are
     * still being processed. The advantage of not processing user input events is that the chance of
     * accidental reentrancy is greatly reduced. Still you should avoid calling this function.
     *
     * @return true if the job has been executed without error, false otherwise
     */
    bool exec();

    /**
     * Returns the error code, if there has been an error.
     *
     * Make sure to call this once result() has been emitted
     *
     * @return the error code for this job, 0 if no error.
     */
    int error() const;

protected:
    void emitResult();

    JobPrivate *const d_ptr;
    Job(JobPrivate &dd, QObject *parent);
private:
    Q_DECLARE_PRIVATE(Job)

Q_SIGNALS:
    void result(Solid::Job *job);
};
}
#endif //SOLID_JOB_H