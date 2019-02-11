#include "worker.h"
#include "../core/utils.h"
#include <QEventLoop>
#include <QThread>
#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QDebug>

K::Launcher::Worker::Worker(Mode mode, QObject *parent) : QThread(parent)
{
    setObjectName("Worker");
    if (mode == Mode::FAKE) {
        wt = QThread::currentThread();
    } else {
        wt = this;
        start();
    }
}
K::Launcher::Worker::~Worker()
{
    quit();
}
QThread * K::Launcher::Worker::thread() const {
    return wt;
}
void K::Launcher::Worker::quit() {
    mQuit = true;
    auto dispatcher = QAbstractEventDispatcher::instance(wt);
    if (dispatcher) dispatcher->wakeUp();
    wait();
}
void K::Launcher::Worker::run() {
    QEventLoop loop;
    auto dispatcher = QAbstractEventDispatcher::instance(wt);
    for(;;) {
        auto p = mQueue.fetchAndStoreOrdered(nullptr);
        while(p) {
            auto n = p->mNext;
            if (p->mSync == CLEAR)
            {
                //removing all objects from queue
                QObject * owner = p->mOwner;
                auto ** pv = &mHead;
                auto     v = mHead;
                mTail      = nullptr;
                while(v) {
                    if (v->mOwner == owner) {
                        auto c = v;
                        *pv = v->mNext;
                        v = v->mNext;
                        if (c->mMutex)
                            c->mMutex->unlock();
                        delete c;
                    } else {
                        mTail = v;
                        v = v->mNext;
                        pv = &v->mNext;
                    }
                }
                delete p;
            } else {
                p->mNext = nullptr;
                if (!mHead) {
                    mHead = mTail = p;
                } else {
                    mTail->mNext = p;
                    mTail = p;
                }
            }
            p = n;
        }
        //process pending calls
        p       = mHead;
        auto pp = &mHead;

        while (p) {
            bool not_blocked = p->mFunc();
            if (not_blocked) {
                auto c = p;
                *pp = p->mNext;
                p   = p->mNext;
                if (c->mMutex)
                    c->mMutex->unlock();
                delete c;
                continue;
            }
            if (p->mSync != ASYNC)
                break;

            p = p->mNext;
            pp= &p->mNext;
        }
        if (!mHead) mTail = nullptr;
        if (mQueue.load())
            continue;
        if (mQuit) break;
        //if no more tasks can be processed, process events
        dispatcher->processEvents(QEventLoop::WaitForMoreEvents);
    }
    auto p = mQueue.load();
    while(p) {
        auto n = p->mNext;
        delete p;
        p = n;
    }
}
void K::Launcher::Worker::enqueue(Task * f) {
    QMutex   mutex;
    if (f->mSync != ASYNC) {
        f->mMutex = &mutex;
        mutex.lock();
    }
    Task * p = mQueue.load();
    f->mNext = p;
    while (!mQueue.testAndSetOrdered(p, f))
        p = f->mNext = mQueue.load();
    auto dispatcher = QAbstractEventDispatcher::instance(wt);
    if (dispatcher) dispatcher->wakeUp();
    //mutex will be unlocked in worker thread
    mutex.lock();
}
void K::Launcher::Worker::async(QObject *owner, const K::function<bool ()> &func) {
    enqueue(new Task(owner, func, ASYNC));
}
void K::Launcher::Worker::async(QObject *owner, K::function<bool ()> &&func) {
    enqueue(new Task(owner, func, ASYNC));
}
void K::Launcher::Worker::sync(QObject *owner, const K::function<bool ()> &func) {
    enqueue(new Task(owner, func, ASYNC));
}
void K::Launcher::Worker::sync(QObject *owner, K::function<bool ()> &&func) {
    enqueue(new Task(owner, func, ASYNC));
}
void K::Launcher::Worker::blocking(QObject *owner, const K::function<bool ()> &func) {
    enqueue(new Task(owner, func, SYNC));
}
void K::Launcher::Worker::blocking(QObject *owner, K::function<bool ()> &&func) {
    enqueue(new Task(owner, func, SYNC));
}
void K::Launcher::Worker::drop(QObject *tag) {
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    enqueue(new Task(tag));
}
