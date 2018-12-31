#pragma once

#include "klang_global.h"
#include "str.h"
#include <../core/compat.h>
#include <QVector>
#include <QElapsedTimer>

namespace K {
namespace Lang {
namespace Internal {

class ASTGenerator;

class ASTContext{
public:
    ASTContext(bool parseargs, int vertex);

    uint getToken() { return tctx.token_out; }
    uint getTokenStart() { return tctx.start; }
    uint getTokenEnd() { return tctx.end; }
    uint getTokenDetail() { return tctx.detail; }

    QString getIdentAsQString() const;

    QVariant getImmediate() const { return tctx.variant; }
private:
    friend class ASTGenerator;
    enum { PRE_VERTEX, MID_VERTEX, POST_VERTEX, PRE_EDGE, MID_EDGE, POST_EDGE };

    TokenizerContext tctx;
    QVector<int>     stack;
    int              current_state;
    int              current_vertex;
    int              current_edge;
};
class ASTGenerator : public virtual Tokenizer {
public:
    ASTGenerator();
    int iteration(ASTContext * context, String * str,
                  K::function<bool ()> isinterrupted);

    typedef K::function<int ()> callback;


protected:
    enum {CONTINUE, STOP, IGNORE, BLOCKED, NEEDMORETIME};
    virtual void invalidTokensAtEnd() = 0;
    virtual void unexpectedTokens() = 0;

    typedef struct { int input, target, next, push; callback chk;} Edge;
    typedef struct { int edg, edg_last; callback cb; } Vertex;
    int VERTEX(callback cb = callback());
    Edge& EDGE(int v, int to, int tok, callback chk = callback());
    void EDGES(int v, int to, std::initializer_list<int> toks, callback chk = callback());
    void CALL(int v, int to, int after, std::initializer_list<int> toks, callback chk = callback());
    void CALL(int v, int to, int after, int token, callback chk = callback());
    void NEXT(int v, int to, callback chk = callback());
    void RET(int v, callback chk = callback());
    int TOK(int v, int token, callback verify = callback(), callback chk = callback());
    int TOKS(int v, std::initializer_list<int> toks, callback verify = callback(), callback chk = callback());
private:
    typedef QVector<Edge>   EdgeMap;
    typedef QVector<Vertex> VertexMap;
    EdgeMap   edges;
    VertexMap vertices;

    ASTContext * current = nullptr;
    String     * curstr  = nullptr;
};

} //namespace Internal
} //namespace Lang
} //namespace K
