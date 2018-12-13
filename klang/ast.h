#pragma once

#include "klang_global.h"
#include "str.h"
#include <../core/compat.h>
#include <QVector>

namespace K {
namespace Lang {
namespace Internal {

class ASTGenerator {
public:
    ASTGenerator();

    enum {CONTINUE, STOP, IGNORE, BLOCKED};
    typedef K::function<int ()> callback;
    typedef struct { int input, target, next, push; callback chk;} Edge;
    typedef struct { int edg, edg_last; callback cb; } Vertex;
protected:
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
};

} //namespace Internal
} //namespace Lang
} //namespace K
