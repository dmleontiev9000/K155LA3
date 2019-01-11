#pragma once

#include "klang_global.h"
#include <QObject>
#include <QVector>

namespace K {
namespace Lang {

enum class RC {
    CONTINUE,
    SUCCESS,
    ERROR,
    INTERNAL_ERROR,
    BLOCKED,
    INTERRUPTED,
    IGNORE,
};
typedef K::function<bool ()> InterruptTest;

class Node;
class Reference;
class Signature;

class ContextPrivate;
class K_LANG_EXPORT Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(QObject *parent = nullptr);
    ~Context();

private:
    friend class Node;
    ContextPrivate * const d;
    void timerEvent(QTimerEvent *event);
};

} //namespace Lang;
} //namespace K;

