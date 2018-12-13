#pragma once

#include "coregui_global.h"
#include <QObject>
#include <QList>
#include <QMap>
#include <QQueue>
#include <QElapsedTimer>

namespace K {
namespace Core {

class COREGUISHARED_EXPORT Clock : public QObject
{
    Q_OBJECT
public:
    enum class ObjectState {
        PENDING,
        BLOCKED,
        READY
    };

    class COREGUISHARED_EXPORT Object {
    public:
        Object();
        virtual ~Object();

        virtual qint64 startTime() = 0;
        virtual ObjectState computeState(qint64 time, bool fake) = 0;
        virtual qint64 applyState(qint64 time) = 0;
        virtual void   resetDevice() = 0;
        void wake();
        void schedule(qint64 t);
    private:
        friend class ::K::Core::Clock;
        Clock * mSync;
    };

    Clock(QObject * parent = nullptr);
    virtual ~Clock();

    void addObject(Object* o);
    void removeObject(Object * o);
public slots:
    void setTime(qint64 time);
    void setRate(qreal rate);
    void play(bool on);
    void stop();
signals:
    void displayTestFrame(qint64);
    void playing(char mode);
    void displayTime(qint64 time, qreal rate);
private:
    friend class ::K::Core::Clock::Object;
    void scheduleObject(Object * o, qint64 time);
    void wakeObject(Object * o);

    qreal mDesiredRate = 1.0;
    int   mTimer       = -1;
    int   mSeekTimer   = -1;
    bool  mPlaying     = false;
    bool  mMaySchedule = true;

    QElapsedTimer mElapsed;
    qint64 mCurrentTimestamp = 0;
    qint64 mSeekTimestamp = -1;


    QList<Object*> mObjects;
    struct Entry {
        ObjectState state;
        Object    * object;
    };
    QMap<qint64, Entry> mSchedule;
    void processSchedule();
    void timerEvent(QTimerEvent * event);
};

} //namespace Elements;
} //namespace K;

