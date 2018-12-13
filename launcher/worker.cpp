#include "worker.h"
#include "../core/utils.h"
#include <vector>
#include <deque>
#include <QEventLoop>
#include <QThread>
#include <QMutex>
#include <QReadWriteLock>
#include <QSet>
#include <QWaitCondition>
#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QDebug>

namespace {
    enum Type { BLOCKING, SYNC, ASYNC };
    struct Task {
        Task(QObject * owner,
             const K::function<bool ()>& f,
             Type sync)
            : mOwner(owner)
            , mFunc(f)
            , mSync(sync)
        { }
        Task(QObject * owner,
             K::function<bool ()>&& f,
             Type sync)
            : mOwner(owner)
            , mFunc(std::move(f))
            , mSync(sync)
        { }

        QObject                   * mOwner;
        K::function<bool ()>        mFunc;
        Type                        mSync;
    };

    class F : public QThread {
    public:
        F(QObject * parent, bool fake) : QThread(parent) {
            mVec.reserve(64);
            mBack.reserve(64);
            mStop = false;
            mReallyRunning = false;
            mMayExit = false;
            mFake = fake;
            setObjectName("thread");
        }
        void enqueue(QObject * tag, const K::function<bool ()>& f, Type sync) {
            if (mStop)
                return;
            mMutex.lock();
            mVec.emplace_back(tag, f, sync);
            if (sync != BLOCKING) {
                mMutex.unlock();
                wakeUp();
            } else {
                waitSync(tag);
            }
        }
        void enqueue(QObject * tag, K::function<bool ()>&& f, Type sync) {
            if (mStop)
                return;
            mMutex.lock();
            mVec.emplace_back(tag, f, sync);
            if (sync != BLOCKING) {
                mMutex.unlock();
                wakeUp();
            } else {
                waitSync(tag);
            }
        }
        void waitSync(QObject *tag) {
            if (mStop)
                return;
            QEventLoop     loop;
            volatile bool  busyFlag = true;
            mVec.emplace_back(tag, [=, &busyFlag] () {
                busyFlag = false;
                return true;
            }, BLOCKING);
            mMutex.unlock();
            wakeUp();

            while(busyFlag) {
                loop.processEvents();
                QThread::yieldCurrentThread();
            }
        }
        void drop(QObject * tag) {
            if (mStop)
                return;

            volatile bool  busyFlag = true;

            mMutex.lock();
            for(auto i = mVec.begin(); i != mVec.end(); ) {
                if (i->mOwner == tag)
                    i = mVec.erase(i);
                else
                    ++i;
            }

            if (mFake) {
                for(auto j = mPending.begin();
                    j != mPending.end();) {
                        if (j->mOwner == tag)
                            j = mPending.erase(j);
                        else
                            ++j;
                }
            } else {
                mVec.emplace_back(tag, [=, &busyFlag] () {
                    for(auto j = mPending.begin();
                        j != mPending.end();) {
                            if (j->mOwner == tag)
                                j = mPending.erase(j);
                            else
                                ++j;
                    }
                    busyFlag = false;
                    return true;
                }, BLOCKING);
            }
            mMutex.unlock();

            if (!mFake) {
                wakeUp();

                QEventLoop     loop;

                while(busyFlag) {
                    loop.processEvents();
                    QThread::yieldCurrentThread();
                }
            }
        }
        void waitStart() {
            while(!mReallyRunning)
                QThread::yieldCurrentThread();
        }
        void stop() {
            mStop = true;
            if (QThread::currentThread() != this)
                wakeUp();
            mMayExit = true;
        }
        void run() {
            QEventLoop loop;
            QAbstractEventDispatcher * dispatcher =
                    QAbstractEventDispatcher::instance();
            mReallyRunning = true;
            while(!mStop) {
                dispatcher->processEvents(mPending.empty() ?
                    QEventLoop::WaitForMoreEvents :
                    QEventLoop::AllEvents);
                process();
            }
            mReallyRunning = false;
            //we must wait for wakeup to return here
            while (!mMayExit)
                QThread::yieldCurrentThread();
        }
    private slots:
        void wakeUp() {
            QAbstractEventDispatcher * dispatcher =
                    QAbstractEventDispatcher::instance(this);
            if (dispatcher)
                dispatcher->wakeUp();
        }

        bool process() {
            if (mStop) return false;

            for(auto i = mPending.begin();
                i != mPending.end();)
            {
                bool not_blocked = i->mFunc();
                if (not_blocked) {
                    i = mPending.erase(i);
                } else if (i->mSync == ASYNC){
                    ++i;
                } else {
                    break;
                }
            }

            mMutex.lock();
            mBack.clear();
            std::swap(mVec, mBack);
            mMutex.unlock();

            bool blocked;
            for(std::size_t i = 0; i < mBack.size(); ++i) {
                Task& task = mBack[i];
                if (task.mSync != ASYNC) {
                    blocked = !mPending.empty();
                    //синхронизируемся с другими задачами
                    if (blocked) {
                        blocked = false;
                        for(auto j = mPending.begin();
                            j != mPending.end(); ++j) {
                            if (j->mOwner == task.mOwner) {
                                blocked = true;
                                break;
                            }
                        }
                    }
                    if (!blocked)
                        blocked = !task.mFunc();
                } else {
                    //если задача просто завершилась
                    //сбрасываем её со счетов, иначе кладем в список pending
                    blocked = !task.mFunc();
                }
                if (blocked)
                    mPending.emplace_back(task.mOwner, task.mFunc, task.mSync);
            }
            return true;
        }

    private:
        volatile bool     mStop;
        volatile bool     mReallyRunning;
        volatile bool     mMayExit;
        QMutex            mMutex;
        int               mTimer;
        std::vector<Task> mVec;
        std::vector<Task> mBack;
        std::deque<Task>  mPending;
        bool              mFake;
    };

    static QReadWriteLock gMutex;
    static QSet<F*>       gWorkers;
    static QSet<QObject*> gRemoved;
}

K::Launcher::Worker::Worker(Mode mode, QObject *parent) : QObject(parent)
{
    setObjectName("Worker");
    F * f = new F(this, mode == Mode::FAKE);
    dptr = f;

    gMutex.lockForWrite();
    gWorkers.insert(f);
    gMutex.unlock();

    if (mode == Mode::FAKE) {
        wt = QThread::currentThread();
    } else {
        wt = f;
        f->start();
        f->waitStart();
    }
}
K::Launcher::Worker::~Worker()
{
    F * f = (F*) dptr;

    gMutex.lockForWrite();
    gWorkers.remove(f);
    if (wt != QThread::currentThread()) {
        f->stop();
        f->wait();
    }
    gMutex.unlock();

    delete f;
}
void K::Launcher::Worker::run() {
    F * f = (F*) dptr;
    f->run();
}
void K::Launcher::Worker::quit() {
    F * f = (F*) dptr;
    f->stop();
}
QThread * K::Launcher::Worker::thread() const {
    return wt;
}
void K::Launcher::Worker::async(QObject *owner, const K::function<bool ()> &func) {
    F * f = (F*) dptr;
    bool allowed;
    gMutex.lockForRead();
    allowed = gRemoved.isEmpty() || !gRemoved.contains(owner);
    gMutex.unlock();
    if (allowed)
        f->enqueue(owner, func, ASYNC);
}
void K::Launcher::Worker::async(QObject *owner, K::function<bool ()> &&func) {
    F * f = (F*) dptr;
    bool allowed;
    gMutex.lockForRead();
    allowed = gRemoved.isEmpty() || !gRemoved.contains(owner);
    gMutex.unlock();
    if (allowed)
        f->enqueue(owner, func, ASYNC);
}
void K::Launcher::Worker::sync(QObject *owner, const K::function<bool ()> &func) {
    F * f = (F*) dptr;
    bool allowed;
    gMutex.lockForRead();
    allowed = gRemoved.isEmpty() || !gRemoved.contains(owner);
    gMutex.unlock();
    if (allowed)
        f->enqueue(owner, func, SYNC);
}
void K::Launcher::Worker::sync(QObject *owner, K::function<bool ()> &&func) {
    F * f = (F*) dptr;
    bool allowed;
    gMutex.lockForRead();
    allowed = gRemoved.isEmpty() || !gRemoved.contains(owner);
    gMutex.unlock();
    if (allowed)
        f->enqueue(owner, func, SYNC);
}
void K::Launcher::Worker::blocking(QObject *owner, const K::function<bool ()> &func) {
    F * f = (F*) dptr;
    f->enqueue(owner, func, SYNC);
}
void K::Launcher::Worker::blocking(QObject *owner, K::function<bool ()> &&func) {
    F * f = (F*) dptr;
    f->enqueue(owner, func, SYNC);
}
void K::Launcher::Worker::drop(QObject *tag) {
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    F * f = (F*) dptr;
    f->drop(tag);
}
void K::Launcher::Worker::dropAll(QObject *tag) {
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    gMutex.lockForWrite();
    gRemoved.insert(tag);
    auto i = gWorkers.constBegin();
    while(i != gWorkers.constEnd()) {
        (*i)->drop(tag);
        ++i;
    }
    gMutex.unlock();
}
void K::Launcher::Worker::finalize(QObject *tag) {
    gMutex.lockForWrite();
    gRemoved.remove(tag);
    gMutex.unlock();
}
