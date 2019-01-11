#pragma once

#include "klang_global.h"
#include "str.h"
#include "context.h"
#include <QObject>
#include <QVector>

namespace K {
namespace Lang {

class Node;
class Reference;
class K_LANG_EXPORT Node {
public:
    enum {
        INVALID,
        COMMENT,
        NAMESPACE,
        TYPE,
        VOID,//special void 'type'
        BASICTYPE,
        ATTRIBUTE,
        TEMPLATE,
        INSTANCE,//template instance
        TPARAMETER,
        VPARAMETER,
        FUNCTION,
        ARGUMENT,
        SCOPE,
        STATEMENT,
        VARIABLE,
        OPERATOR_PREFIX,
        OPERATOR_POSTFIX,
        OPERATOR_BINARY,
        OPERATOR_MEMBER,
        FORCE_COMMENT=0x8000,
        FORCE_DISABLED=0x10000
    };
    static Node * create(Context * ctx);
    static Node * create(Context * ctx, const char * text);
    static Node * create(Context * ctx, const QString& text);
    static Node * create(Context * ctx, const QStringRef& text);
    bool   comment() const;
    void   comment(bool);
    void   attach(Node * __restrict__ attachment, Node * __restrict__ after = nullptr);
    void   detach();
    void   destroy();
    void   invalidate();
    void   enable();
    void   disable();
    bool   disabled() const;

    bool   exactNameMatch(const String * str, uint from, uint to) const;
    bool   guessNameMatch(const String * str, uint from, uint to) const;

    uint   type() const;
    bool   isType() const {
        constexpr uint m =
                (1<<TYPE)|
                (1<<BASICTYPE)|
                (1<<TEMPLATE)|
                (1<<INSTANCE)|
                (1<<TPARAMETER);
        return ((1<<type()) & m) != 0;
    }
    bool   isValid() const;
    Node * parent() const;
    Node * next() const;
    Node * previous() const;
    Node * firstChild() const;
    Node * lastChild() const;
    Node * firstExplicitChild() const;
    Node * namedImplicitChild(const String * str, uint from, uint to) const;
    Node * declType() const;
protected:
    Node() {}
    ~Node() {}
};

} //namespace Lang;
} //namespace K;

