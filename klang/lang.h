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

class K_LANG_EXPORT Node;
class K_LANG_EXPORT Reference;
class K_LANG_EXPORT Signature;

class ContextPrivate;
class K_LANG_EXPORT Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(QObject *parent = nullptr);
    ~Context();
private:
    ContextPrivate * d;
    void timerEvent(QTimerEvent *event);
};

} //namespace Lang;
} //namespace K;

