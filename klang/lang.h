#pragma once

#include "klang_global.h"
#include <QObject>
#include <QVector>

namespace K {
namespace Lang {

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

