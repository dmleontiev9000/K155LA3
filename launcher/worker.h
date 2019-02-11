#pragma once

#include "../core/interfaces.h"
#include "../core/compat.h"
#include <QThread>
#include <QMutex>
#include <QAtomicPointer>

namespace K {
namespace Launcher {

class Core;

class Worker : public QThread, public IfWorker
{
    Q_OBJECT
public:
    ~Worker();

    QThread * thread() const override;

    void async(QObject *owner, K::function<bool ()>&& func) override;
    void async(QObject *owner, const K::function<bool ()>& func) override;

    void sync(QObject *owner, K::function<bool ()>&& func) override;
    void sync(QObject *owner, const K::function<bool ()>& func) override;

    void blocking(QObject *owner, K::function<bool ()>&& func) override;
    void blocking(QObject *owner, const K::function<bool ()>& func) override;

    void drop(QObject *tag) override;
protected:
    void run();
    void quit();
private:
    enum Type { SYNC, ASYNC, CLEAR };
    struct Task {
        Task(QObject * owner,
             const K::function<bool ()>& f,
             Type sync)
            : mOwner(owner)
            , mFunc(f)
            , mSync(sync)
            , mMutex(nullptr)
        { }
        Task(QObject * owner,
             K::function<bool ()>&& f,
             Type sync)
            : mOwner(owner)
            , mFunc(std::move(f))
            , mSync(sync)
            , mMutex(nullptr)
        { }
        Task(QObject * owner)
            : mOwner(owner)
            , mSync(CLEAR)
        { }

        Task                      * mNext;
        Task                      * mSame;
        QObject                   * mOwner;
        K::function<bool ()>        mFunc;
        Type                        mSync;
        QMutex                    * mMutex;
    };
    void enqueue(Task * t);

    friend class ::K::Launcher::Core;
    QAtomicPointer<Task> mQueue;
    Task                *mHead = nullptr;
    Task                *mTail = nullptr;
    volatile bool        mQuit = false;
    enum class Mode { FAKE, REAL };
    explicit Worker(Mode mode, QObject *parent = 0);

    QThread * wt;
};

} //namespace Launcher;
} //namespace K;

