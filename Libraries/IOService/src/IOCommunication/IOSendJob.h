///////////////////////////////////////////////////////////////////////////////
///
/// @file IOSendJob.h
///
/// Special class to handle send results, or response results.
///
/// @author romanp
/// @owner sergeym
///
/// Copyright (c) 2005-2017, Parallels International GmbH
/// Copyright (c) 2017-2019 Virtuozzo International GmbH, All rights reserved.
///
/// This file is part of Virtuozzo Core Libraries. Virtuozzo Core
/// Libraries is free software; you can redistribute it and/or modify it
/// under the terms of the GNU Lesser General Public License as published
/// by the Free Software Foundation; either version 2.1 of the License, or
/// (at your option) any later version.
///
/// This library is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
/// Lesser General Public License for more details.
///
/// You should have received a copy of the GNU Lesser General Public
/// License along with this library.  If not, see
/// <http://www.gnu.org/licenses/> or write to Free Software Foundation,
/// 51 Franklin Street, Fifth Floor Boston, MA 02110, USA.
///
/// Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef IOSENDJOB_H
#define IOSENDJOB_H

#include <QMutex>
#include <QReadWriteLock>
#include <QHash>
#include <QVector>
#include <QWeakPointer>
#include <QWaitCondition>

#include "IOProtocol.h"
#include "BlockingQueue.h"
#include <boost/noncopyable.hpp>

namespace IOService {

class IOJobManager;
class IOSendJobInterface;

class IOSendJob: boost::noncopyable
{
public:
    enum Result
    {
        // Common

        Success = 0,      /**< Job succeed */
        Fail,             /**< Job failed, unknown error */
        InvalidJob,       /**< Job handle is invalid */
        InvalidPackage,   /**< Package is invalid: buffer is not accessible */
        Timeout,          /**< Time elapsed */
        UrgentlyWaked,    /**< Job was waked up _NOT_ from IO service thread,
                               but from some other for some urgent purposes */

        // Sending

        SendPended,       /**< Send was pended */
        SendQueueIsFull,  /**< Send queue is full. Try next time */

        ConnClosedByPeer, /**< Other side of connection has been closed */
        ConnClosedByUser, /**< User closed connection */

        // Response

        NoResponse        /**< Response has not been received yet */
    };

    class Handle
    {
    public:
        Handle ();
        Handle ( const SmartPtr<IOSendJob>& );

        bool isValid () const;

    private:
        SmartPtr<IOSendJob> m_job;

        // Only these classes/functions can access m_job directly.
        friend quint32 qHash ( const IOSendJob::Handle& h );
        friend bool operator== ( const IOSendJob::Handle& h1,
                                 const IOSendJob::Handle& h2 );
        friend bool operator!= ( const IOSendJob::Handle& h1,
                                 const IOSendJob::Handle& h2 );

        friend class IOSendJobInterface;
        friend class IOJobManager;
    };

    /** Define invalid default handle for easy use */
    static Handle InvalidHandle;

    class Response
    {
    public:
        Response ();

        Result responseResult;
        IOSender::Handle senderHandle;
        QList< SmartPtr<IOPackage> > responsePackages;
    };

    IOSendJob ();
    ~IOSendJob ();

    /** Set all members to default values */
    void registerPackageUuid ( const Uuid_t );
    const Uuid& getPackageUuid () const;

    bool isResponsibleForPackageUuid ( const Uuid_t ) const;

    bool waitForSend ( quint32 timeout, bool& urgentlyWaked );
    bool waitForResponse ( quint32 timeout, bool& urgentlyWaked );

    void wakeSendWaitings ( IOSendJob::Result );
    void wakeResponseWaitings ( IOSendJob::Result,
                                const IOSender::Handle&,
                                const SmartPtr<IOPackage>& );

    IOSendJob::Result getSendResult () const;

    IOSendJob::Response takeResponse ();
    bool sendWaitingsWereWaken () const;
    bool responseWaitingsWereWaken () const;

    quint32 getSendWaitingsNumber () const;
    quint32 getResponseWaitingsNumber () const;

private:
    Result m_sendResult;

    mutable QMutex m_mutex;
    mutable QWaitCondition m_sendWait;
    mutable QWaitCondition m_responseWait;

    volatile mutable qint32 m_sendWaitingsNum;
    volatile mutable qint32 m_responseWaitingsNum;

    Uuid m_packageUuid;
    Cancellation::Sink m_sendSink;
    Cancellation::Sink m_responseSink;
    Cancellation::Token m_token;
    mutable BlockingQueue<Response, 7> m_throttle;
};

class IOJobManager
{
public:
    enum MaxSizeType {
        MaxActiveJobsSize = 20,
        OptimalPoolSize = 50,
        OptimalNonFreePoolSize = quint32(OptimalPoolSize * 0.75)
    };

    class Job;
    typedef QWeakPointer<Job> JobRefType;

    class Job
    {
    public:
        Job ();

        SmartPtr<IOSendJob> sendJob;
        IOPackage::PODHeader pkgHeader;
        SmartPtr<IOPackage> pkg;
        JobRefType nextJob;
        bool isActive;
    };

    class JobPool
    {
    public:
        JobPool ( const Uuid& );

        Uuid uuid;
	mutable QReadWriteLock rwLock;
        QVector< QSharedPointer<Job> > jobList;
        quint32 activeJobsSize;
        JobRefType firstActiveJob;
        JobRefType lastActiveJob;
        QSharedPointer<Job> heartBeatJob;
    };

    IOJobManager(): m_activeJobsLimit(MaxActiveJobsSize)
    {
    }

    /** Inits job pool with #MaxPoolSize jobs */
    SmartPtr<JobPool> initJobPool ();

    /** Finds job in pool by response package */
    SmartPtr<IOSendJob> findJobByResponsePackage (
                                           const SmartPtr<JobPool>& jobPool,
                                           const SmartPtr<IOPackage>& ) const;

    /**
     * Inits active job in job pool.
     * Returns true, if active jobs size is <= #MaxActiveJobsSize,
     * false otherwise.
     * If urgent param is true, always tries to init active job in spite
     * of active job list size.
     * Actually, false can be returned and job param inited to 0,
     * if allocation failed.
     *
     * @param jobPool   [in]  job pool object
     * @param pkgHeader [in]  pkg header to write
     * @param package   [in]  pkg to write
     * @param job       [out] output job
     */
    bool initActiveJob ( SmartPtr<JobPool>& jobPool,
                         const IOPackage::PODHeader& pkgHeader,
                         const SmartPtr<IOPackage>& package,
                         JobRefType& job,
                         bool urgent );

    /**
     * Returns next active job.
     * If 0 is returned, active job is not found.
     * @note #getNextActiveJob and #putActiveJob must be called from the same
     *       thread.
     */
    JobRefType getNextActiveJob ( const SmartPtr<JobPool>& ) const;

    /**
     * Always returns valid heart beat job
     */
    JobRefType getHeartBeatJob ( const SmartPtr<JobPool>& ) const;

    /**
     * Puts active job back to pool.
     * Actually makes job inactive.
     * @note #getNextActiveJob and #putActiveJob must be called from the same
     *       thread.
     */
    void putActiveJob ( SmartPtr<JobPool>, const JobRefType& );

    /**
     * Returns list of all busy (non free) send jobs and
     * their pacakges in job pool
     */
    QList<JobRefType>
        getBusySendJobs ( const SmartPtr<JobPool>& ) const;

    unsigned getActiveJobsLimit() const
    {
        return m_activeJobsLimit;
    }

    void setActiveJobsLimit(unsigned value_)
    {
        m_activeJobsLimit = value_;
    }
private:
    bool isJobFree ( const Job* ) const;

    unsigned m_activeJobsLimit;
};

/*****************************************************************************/

/** Qt hashes support */
inline quint32 qHash ( const IOSendJob::Handle& h )
{
    return qHash( (h.m_job.isValid() ? h.m_job->getPackageUuid() : Uuid()) );
}

/** Qt hashes support */
inline bool operator== ( const IOSendJob::Handle& h1,
                         const IOSendJob::Handle& h2 )
{
    return (h1.m_job == h2.m_job);
}

/** Another operator */
inline bool operator!= ( const IOSendJob::Handle& h1,
                         const IOSendJob::Handle& h2 )
{
    return (h1.m_job != h2.m_job);
}

} //namespace IOService

#endif //IOSENDJOB_H
