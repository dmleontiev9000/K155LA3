#pragma once

#include "klang_global.h"
#include "ast.h"
#include "context.h"

namespace K {
namespace Lang {

class KGeneratorPrivate;
class K_LANG_EXPORT KGenerator : public ASTGenerator
{
public:
    KGenerator();
    virtual ~KGenerator();

    class Context : public ASTGenerator::Context {
    public:
        virtual ~Context();
    protected:
        Context();
    };

    Context * createContext(Node * node);
private:
    KGeneratorPrivate * d;
protected:
protected:
private:
    std::vector<bool> mSimpleFlag;
protected:
    /*
     * element generators
     */
private:
    /*
     * keywords
     */
    void langInit();
    void langAddResolve();
    void CALL_RESOLVE_ELT(int v, int after);
    void CALL_RESOLVE_ELT_SIMPLE(int v, int after);
    void langAddExpr();
    void CALL_COMPLEX_EXPR(int v, int after);
    void CALL_SIMPLE_EXPR(int v, int after);

    static KW keywords[];
    enum Keywords {
        KEYWORD_MIN = T::TOKEN_MAX,
        //keywords
        KEYWORD_CONST,
        KEYWORD_VOLATILE,
        KEYWORD_STATIC,
        KEYWORD_FINAL,
        KEYWORD_MUTABLE,
        KEYWORD_VIRTUAL,

        KEYWORD_IN,
        KEYWORD_OUT,
        KEYWORD_INOUT,

        KEYWORD_INT,
        KEYWORD_UINT,
        KEYWORD_BITVEC,
        KEYWORD_TYPE,
        KEYWORD_THIS,

        KEYWORD_PRIVATE,
        KEYWORD_PUBLIC,
        KEYWORD_PROTECTED,

        KEYWORD_ENUM,
        KEYWORD_FLAGS,
        KEYWORD_TYPEDEF,
        KEYWORD_NAMESPACE,
        KEYWORD_STRUCT,
        KEYWORD_CLASS,

        KEYWORD_CONSTRUCTOR,
        KEYWORD_DESTRUCTOR,
        KEYWORD_OPERATOR,
        KEYWORD_OPERATOR_MEM,
        KEYWORD_OPERATOR_POST,
        KEYWORD_OPERATOR_PREF,
        KEYWORD_NEW,
        KEYWORD_DELETE,
        KEYWORD_MOVE,
        KEYWORD_COPY,
        KEYWORD_PROPERTY,
        KEYWORD_FUNCTION,
        KEYWORD_RETURN,
        KEYWORD_VAR,
        KEYWORD_SLOT,
        KEYWORD_SIGNAL,

        KEYWORD_SIZEOF,
        KEYWORD_TYPEOF,
        KEYWORD_OFFSETOF,
        KEYWORD_ALIGNOF,

        KEYWORD_ASSERT,

        KEYWORD_IF,
        KEYWORD_ELSE,

        KEYWORD_FOR,
        KEYWORD_WHILE,
        KEYWORD_BREAK,
        KEYWORD_CONTINUE,

        KEYWORD_SWITCH,
        KEYWORD_CASE,
        KEYWORD_DEFAULT,
    };
};

} //namespace Lang
} //namespace K
