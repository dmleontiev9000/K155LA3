#include "ast.h"

using namespace K::Lang::Internal;

int ASTGenerator::VERTEX(callback cb) {
    Vertex v;
    v.edg  = -1;
    v.edg_last = -1;
    v.cb   = cb;
    vertices.append(v);
    return vertices.size();
}
ASTGenerator::Edge& ASTGenerator::EDGE(int v, int to, int tok, callback chk) {
    Q_ASSERT(v  >= 0 && v  < vertices.size());
    Q_ASSERT(to >= 0 && to < vertices.size());

    Edge e;
    e.input = tok;
    e.target= to;
    e.chk   = chk;
    e.push  = -1;
    e.next  = -1;
    int n = edges.size();
    if (vertices[v].edg < 0) {
        vertices[v].edg = n;
    } else {
        edges[vertices[v].edg_last].next = n;
    }
    edges.append(e);
    vertices[v].edg_last = n;
    return edges.last();
}
void ASTGenerator::EDGES(int v, int to, std::initializer_list<int> toks, callback chk ) {
    for(auto i = toks.begin(); i != toks.end(); ++i)
        EDGE(v, to, *i, chk);
}
void ASTGenerator::CALL(int v, int to, int after, std::initializer_list<int> toks, callback chk) {
    for(auto i = toks.begin(); i != toks.end(); ++i)
        EDGE(v, to, *i, chk).push=after;
}
void ASTGenerator::CALL(int v, int to, int after, int token, callback chk)
{
    EDGE(v, to, token, chk).push = after;
}
void ASTGenerator::NEXT(int v, int to, callback chk) {
    EDGE(v, to, -1, chk);
}
void ASTGenerator::RET(int v, callback chk) {
    EDGE(v, -1, -1, chk);
}
int ASTGenerator::TOK(int v, int token, callback verify, callback chk)
{
    int vx = VERTEX(verify);
    EDGE(v, vx, token, chk);
    return vx;
}
int ASTGenerator::TOKS(int v, std::initializer_list<int> toks, callback verify, callback chk) {
    int vx = VERTEX(verify);
    for(auto i = toks.begin(); i != toks.end(); ++i)
        EDGE(v, vx, *i, chk);
    return vx;
}
