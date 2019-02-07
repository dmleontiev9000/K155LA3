#pragma once

#include "klang_global.h"
#include "../core/compat.h"
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
enum class EvalOrder {
    NAMESPACES,TYPES,MEMBERS,STMTS,
    MAX_EVAL_ORDER
};

class Node;
class Reference;
class Signature;

class ContextPrivate;
class K_LANG_EXPORT Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(QObject *parent = nullptr);
    void deleteLater();
private:
    friend class Node;
    ContextPrivate * const d;
    void timerEvent(QTimerEvent *event);
    /*
     * do not call descructor directly. Use deleteLater instead
     * because you may be called by context itself.
     */
    ~Context();
};

} //namespace Lang;
} //namespace K;

