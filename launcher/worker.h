#pragma once

#include "../core/interfaces.h"
#include "../core/compat.h"

namespace K {
namespace Launcher {

class Core;

class Worker : public QObject, public IfWorker
{
    Q_OBJECT
public:
    virtual ~Worker();

    QThread * thread() const override;

    void async(QObject *owner, K::function<bool ()>&& func) override;
    void async(QObject *owner, const K::function<bool ()>& func) override;

    void sync(QObject *owner, K::function<bool ()>&& func) override;
    void sync(QObject *owner, const K::function<bool ()>& func) override;

    void blocking(QObject *owner, K::function<bool ()>&& func) override;
    void blocking(QObject *owner, const K::function<bool ()>& func) override;

    void drop(QObject *tag) override;

    static void dropAll(QObject *tag);
    static void finalize(QObject * tag);
protected:
    void run();
    void quit();
private:
    friend class ::K::Launcher::Core;

    enum class Mode { FAKE, REAL };
    explicit Worker(Mode mode, QObject *parent = 0);

    void    * dptr;
    QThread * wt;
};

} //namespace Launcher;
} //namespace K;

