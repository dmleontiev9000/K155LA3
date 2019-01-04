#pragma once

#include "klang_global.h"
#include "tokenizer.h"
#include "lang_p.h"
#include <../core/compat.h>
#include <QVector>
#include <QElapsedTimer>

namespace K {
namespace Lang {

class ASTGeneratorPrivate;
class K_LANG_EXPORT ASTGenerator : public Tokenizer {
public:
    class K_LANG_EXPORT Context : public Tokenizer::Context {
    public:
        Context(bool parseargs, int vertex);
        ~Context();
    private:
        friend class K::Lang::ASTGenerator;
        void * d;
    };
    enum class RC {
        CONTINUE,
        SUCCESS,
        ERROR,
        INTERNAL_ERROR,
        BLOCKED,
        INTERRUPTED,
        IGNORE,
    };

    typedef K::function<RC (const InterruptTest&)> Callback;

    ASTGenerator(const KW * kws);
    virtual ~ASTGenerator();
protected:
    RC iteration(Context * context,
                 const String* s,
                 const InterruptTest& itest);
    int  VERTEX(Callback cb = Callback());
    void EDGES(int v, int to, std::initializer_list<int> toks, Callback chk = Callback());
    void CALL(int v, int to, int after, std::initializer_list<int> toks, Callback chk = Callback());
    void CALL(int v, int to, int after, int token, Callback chk = Callback());
    void NEXT(int v, int to, Callback chk = Callback());
    void RET(int v, Callback chk = Callback());
    int  TOK(int v, int token, Callback verify = Callback(), Callback chk = Callback());
    int  TOKS(int v, std::initializer_list<int> toks, Callback verify = Callback(), Callback chk = Callback());
private:
    Q_DISABLE_COPY(ASTGenerator)
    ASTGeneratorPrivate * d;
};

} //namespace Lang
} //namespace K
