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
        INVALID=0,
        COMMENT,
        NAMESPACE,
        TYPE,
        VOID,//special void 'type'
        BASICTYPE,
        BLOCK,
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

    void   attach(Node * __restrict__ attachment, Node * __restrict__ after = nullptr);
    void   destroy();

    void   detach();
    void   invalidate();
    void   recheck();

    void   enable();
    void   disable();
    bool   disabled() const;
    bool   comment() const;
    void   comment(bool);


    uint   type() const;
    bool   isType() const {
        constexpr uint m =
                (1<<NAMESPACE)|
                (1<<TYPE)|
                (1<<VOID)|
                (1<<BASICTYPE)|
                (1<<BLOCK)|
                (1<<ATTRIBUTE)|
                (1<<TEMPLATE)|
                (1<<TPARAMETER)|
                (1<<VPARAMETER)|
                (1<<FUNCTION)|
                (1<<ARGUMENT)|
                (1<<VARIABLE);
        return ((1<<type()) & m) != 0;
    }
    bool   isValid() const;
    Node * parent() const;
    Node * next() const;
    Node * previous() const;
    Node * firstChild() const;
    Node * lastChild() const;
    Node * firstExplicitChild() const;
    Node * declType() const;
protected:
    Node() {}
    ~Node() {}
};

} //namespace Lang;
} //namespace K;

