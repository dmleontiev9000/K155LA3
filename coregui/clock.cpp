#include "clock.h"
#include <math.h>
#include <limits>
#include <QTimerEvent>

using namespace K::Core;

Clock::Clock(QObject *parent)
    : QObject(parent)
{

}

Clock::~Clock()
{
    Q_ASSERT(mObjects.isEmpty());
}

Clock::Object::Object()
    : mSync(nullptr)
{

}
Clock::Object::~Object()
{
    if (mSync) mSync->removeObject(this);
}

void Clock::addObject(Object *o)
{
    Q_ASSERT(!mObjects.contains(o));
    Q_ASSERT(o->mSync == nullptr);

    stop();
    mObjects.append(o);
    o->mSync = this;
}
void Clock::removeObject(Object *o)
{
    Q_ASSERT(o->mSync == this);
    stop();
    for(auto i = mSchedule.begin(); i != mSchedule.end(); ) {
        if (i.value().object == o) {
            i = mSchedule.erase(i);
        } else {
            ++i;
        }
    }
    mObjects.removeOne(o);
    o->mSync = nullptr;
}
void Clock::Object::schedule(qint64 t) {
    if (mSync)
        mSync->scheduleObject(this, t);
}
void Clock::scheduleObject(Object *o, qint64 time) {
    Q_ASSERT(mMaySchedule == true);
    Entry e;
    e.object = o;
    e.state  = ObjectState::PENDING;
    mSchedule.insertMulti(time, e);
}
void Clock::Object::wake() {
    if (mSync)
        mSync->wakeObject(this);
}
void Clock::wakeObject(Object *o) {
    auto i = mSchedule.begin();
    while (i != mSchedule.end())
    {
        if (i->object == o)
            i->state = ObjectState::READY;
        ++i;
    }
}
void Clock::timerEvent(QTimerEvent *event) {
    if (event->timerId() == mTimer) {
        processSchedule();
    }
    if (event->timerId() == mSeekTimer) {
        bool blocked = false;
        for(Object * o : mObjects) {
            ObjectState st = o->computeState(mSeekTimestamp, true);
            blocked |= (st != ObjectState::READY);
        }
        if (!blocked) {
            emit displayTestFrame(mSeekTimestamp);
            killTimer(mSeekTimer);
            mSeekTimer = -1;
        }
    }
}
void Clock::processSchedule() {
    if (mSchedule.isEmpty()) {
        if (mTimer >= 0) {
            killTimer(mTimer);
            mTimer = -1;
        }
        emit playing('s');
        return;
    }

    qint64 timeElapsed   = mElapsed.restart();
    qreal  timePassed    = timeElapsed*mDesiredRate;
    qint64 nextTimestamp = (qint64)timePassed + mCurrentTimestamp;

    //ставим защиту от дурака: чтоб нам не сломали итератор
    mMaySchedule = false;
    do {
        QMap<qint64, Entry>::iterator i = mSchedule.begin();
        //ищем насколько далеко мы можем симулировать.
        qint64 maxtime = std::numeric_limits<qint64>::max();
        while(i != mSchedule.end()) {
            switch(i.value().state) {
            case ObjectState::BLOCKED:
                maxtime = qMin(maxtime, i.key());
                break;
            case ObjectState::PENDING:
                i.value().state = i.value().object->computeState(i.key(), false);
                break;
            default:;
            }
            ++i;
        }

        //теперь мы знаем насколько далеко можем зайти в симуляции
        nextTimestamp = qBound(mCurrentTimestamp, maxtime, nextTimestamp);
        i = mSchedule.begin();
        while(!mSchedule.isEmpty() && i.key() < nextTimestamp) {
            Entry  e = i.value();
            qint64 t = i.key();
            if (e.state != ObjectState::READY) {
                nextTimestamp = t;
                break;
            }
            mSchedule.erase(i);

            mMaySchedule = true;
            qint64 n = e.object->applyState(t);
            mMaySchedule = false;

            if (n > t) {
                e.state  = ObjectState::PENDING;
                mSchedule.insertMulti(n, e);
            }
            i = mSchedule.begin();
        }
    } while(!mSchedule.isEmpty());
    mMaySchedule = true;

    qreal rate = NAN;
    if (nextTimestamp > mCurrentTimestamp) {
        qint64 dt = qMax(0LL, nextTimestamp - mCurrentTimestamp);
        rate = qBound(0.0, qreal(dt) / timePassed, 1.0);
    }
    mCurrentTimestamp = nextTimestamp;
    emit displayTime(nextTimestamp, rate);

    if (mTimer >= 0) {
        killTimer(mTimer);
        mTimer = -1;
    }
    if (!mSchedule.isEmpty() &&
        mDesiredRate >= 1.0e-3)
    {
        qint64 dt = qMax(0LL, mSchedule.firstKey() - mCurrentTimestamp);
        qreal  dtf = qMax(dt / mDesiredRate, 100.0);
        mTimer = startTimer(int(dtf), Qt::PreciseTimer);
    }
}
void Clock::play(bool on)
{
    if (mTimer >= 0) {
        killTimer(mTimer);
        mTimer = -1;
    }
    emit displayTime(mCurrentTimestamp, NAN);

    if (on) {
        if (mSeekTimer) {
            killTimer(mSeekTimer);
            mSeekTimer = -1;
        }
        if (!mPlaying) {
            mCurrentTimestamp = 0;
            mPlaying = true;
            for(Object * o : mObjects) {
                qint64 n = o->startTime();
                if (n >= 0)
                    scheduleObject(o, n);
            }
        }
        emit playing('>');
        mElapsed.start();
        processSchedule();
    } else {
        emit playing(mPlaying ? 'p':'s');
    }
}
void Clock::stop()
{
    if (mTimer >= 0) {
        killTimer(mTimer);
        mTimer = -1;
    }
    for (Object * o : mObjects)
        o->resetDevice();
    mSchedule.clear();
    mPlaying = false;
    displayTime(mCurrentTimestamp, NAN);
    emit playing('s');
}
void Clock::setRate(qreal rate) {
    if (mTimer >= 0) {
        killTimer(mTimer);
        mTimer = -1;
    }
    mDesiredRate = qBound(0.0, rate, 100.0);
    if (mPlaying) {
        mElapsed.start();
        processSchedule();
    }
}
void Clock::setTime(qint64 time)
{
    play(false);
    mSeekTimestamp = qMax(0LL, time);
    if (mSeekTimer < 0)
        mSeekTimer = startTimer(100);
    displayTime(mSeekTimestamp, NAN);
}
