#include "ast.h"
#include <QVector>
#include <QStack>

namespace K {
namespace Lang {

class ASTGeneratorPrivate {
public:
    ASTGeneratorPrivate(){}

    typedef struct { int input, target, next, push;
                     ASTGenerator::Callback chk;} Edge;
    typedef struct { int edg, edg_last;
                     ASTGenerator::Callback cb; } Vertex;
    Edge& EDGE(int v, int to, int tok,
               ASTGenerator::Callback chk =
               ASTGenerator::Callback());
    int  VERTEX(ASTGenerator::Callback cb =
                ASTGenerator::Callback());
    void EDGES(int v, int to, std::initializer_list<int> toks,
               ASTGenerator::Callback chk =
               ASTGenerator::Callback());
    void CALL(int v, int to, int after, std::initializer_list<int> toks,
              ASTGenerator::Callback chk =
              ASTGenerator::Callback());
    void CALL(int v, int to, int after, int token,
              ASTGenerator::Callback chk =
              ASTGenerator::Callback());
    int  TOK(int v, int token,
              ASTGenerator::Callback verify =
              ASTGenerator::Callback(),
              ASTGenerator::Callback chk =
              ASTGenerator::Callback());
    int  TOKS(int v, std::initializer_list<int> toks,
              ASTGenerator::Callback verify =
              ASTGenerator::Callback(),
              ASTGenerator::Callback chk =
              ASTGenerator::Callback());

    typedef QVector<Edge>   EdgeMap;
    typedef QVector<Vertex> VertexMap;
    EdgeMap   edges;
    VertexMap vertices;
};
} //namespace Lang
} //namespace K
using namespace K::Lang;

namespace {
    enum class State { VERTEX, ENTRY, EDGE };
    struct ContextP {
        QStack<int>      stack;
        State            current_state;
        int              current_vertex;
        int              current_edge;
        int              last_token;
    };
}
ASTGenerator::Context::Context(bool parseargs, int vertex)
    : Tokenizer::Context(parseargs)
{
    auto p = new ContextP;
    p->current_state = State::ENTRY;
    p->current_vertex = vertex;
    p->current_edge = -1;
    p->last_token = -1;
    d = p;
}
ASTGenerator::Context::~Context()
{
    delete (ContextP*)d;
}
ASTGenerator::ASTGenerator(const KW *kws)
    : Tokenizer(kws)
    , d(new ASTGeneratorPrivate)
{

}
ASTGenerator::~ASTGenerator()
{
    delete d;
}

K::Lang::RC ASTGenerator::iteration(Context *context, const String *s, const InterruptTest &itest)
{
    auto current = (ContextP*)context->d;
    const auto& vertices = d->vertices;
    const auto& edges    = d->edges;
    while(!itest || itest()) {
        switch(current->current_state)
        {
        case State::VERTEX:
            /*
             * we are about to visit a vertex
             */
            Q_ASSERT(current->current_vertex < vertices.size());
            if (current->current_vertex < 0) {
                if (current->stack.isEmpty()) {
                    if (context->end() < s->string_length) {
                        error("Unexpected tokens at end", context->start(), s->string_length);
                        return RC::ERROR;
                    }
                    return RC::SUCCESS;
                }
                current->current_vertex = current->stack.pop();
                Q_ASSERT(current->current_vertex < vertices.size());
                Q_ASSERT(current->current_vertex >= 0);
            }
            if (vertices[current->current_vertex].cb) {
                RC rc = vertices[current->current_vertex].cb(itest);
                if (rc != RC::CONTINUE)
                    return rc;
            }
        case State::ENTRY:
            /*
             * at vertex, get new token and begin searching
             * for edge
             */
            Q_ASSERT(current->current_vertex < vertices.size());
            Q_ASSERT(current->current_vertex >= 0);
            current->current_edge = vertices[current->current_vertex].edg;
            current->last_token = tokenize(s, context) ? context->token() : -1;
            current->current_state = State::EDGE;
        case State::EDGE:
            /*
             * searching for an edge
             */
            Q_ASSERT(current->current_edge < edges.size());
            //search for exit edges
            while(current->current_edge >= 0) {
                //input >= 0 && current_token == input
                //input <  0 && current_token >= 0
                auto input = edges[current->current_edge].input;
                bool match = (input < 0) |
                        (current->last_token == input);
                if (match) {
                    if (edges[current->current_edge].chk) {
                        RC rc = edges[current->current_edge].chk(itest);
                        if (rc == RC::IGNORE)
                            continue;
                        if (rc != RC::CONTINUE)
                            return rc;
                    }
                }
                current->current_edge = edges[current->current_edge].next;
                Q_ASSERT(current->current_edge < edges.size());
            }
            if (current->current_edge < 0) {
                if (current->last_token < 0)
                    error("Unexpected end of statement",
                          context->start(), s->string_length);
                else
                    error("Unexpected token",
                          context->start(), context->end());
                return RC::ERROR;
            }
            current->current_vertex = edges[current->current_edge].target;
            if (edges[current->current_edge].push >= 0) {
                current->stack.push(edges[current->current_edge].push);
            } else {
                current->current_state = State::VERTEX;
            }
            break;
        default:
            return RC::INTERNAL_ERROR;
        }
    }
    return RC::INTERRUPTED;
}
int ASTGeneratorPrivate::VERTEX(ASTGenerator::Callback cb) {
    Vertex v;
    v.edg  = -1;
    v.edg_last = -1;
    v.cb   = cb;
    vertices.append(v);
    return vertices.size();
}
int ASTGenerator::VERTEX(Callback cb) {
    return d->VERTEX(cb);
}
ASTGeneratorPrivate::Edge& ASTGeneratorPrivate::EDGE(int v, int to, int tok, ASTGenerator::Callback chk) {
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
void ASTGenerator::EDGE(int v, int to, int tok, Callback chk) {
    d->EDGE(v, to, tok, chk);
}
void ASTGeneratorPrivate::EDGES(int v, int to, std::initializer_list<int> toks, ASTGenerator::Callback chk) {
    for(auto i = toks.begin(); i != toks.end(); ++i)
        EDGE(v, to, *i, chk);
}
void ASTGenerator::EDGES(int v, int to, std::initializer_list<int> toks, Callback chk ) {
    d->EDGES(v, to, toks, chk);
}
void ASTGeneratorPrivate::CALL(int v, int to, int after, std::initializer_list<int> toks, ASTGenerator::Callback chk) {
    for(auto i = toks.begin(); i != toks.end(); ++i)
        EDGE(v, to, *i, chk).push=after;
}
void ASTGenerator::CALL(int v, int to, int after, std::initializer_list<int> toks, Callback chk) {
    d->CALL(v, to, after, toks, chk);
}
void ASTGeneratorPrivate::CALL(int v, int to, int after, int token, ASTGenerator::Callback chk){
    EDGE(v, to, token, chk).push = after;
}
void ASTGenerator::CALL(int v, int to, int after, int token, Callback chk) {
    d->CALL(v, to, after, token, chk);
}
void ASTGenerator::NEXT(int v, int to, Callback chk) {
    d->EDGE(v, to, -1, chk);
}
void ASTGenerator::RET(int v, Callback chk) {
    d->EDGE(v, -1, -1, chk);
}
int ASTGeneratorPrivate::TOK(int v, int token, ASTGenerator::Callback verify, ASTGenerator::Callback chk)
{
    int vx = VERTEX(verify);
    EDGE(v, vx, token, chk);
    return vx;
}
int ASTGenerator::TOK(int v, int token, Callback verify, Callback chk)
{
    return d->TOK(v, token, verify, chk);
}
int ASTGeneratorPrivate::TOKS(int v, std::initializer_list<int> toks, ASTGenerator::Callback verify, ASTGenerator::Callback chk) {
    int vx = VERTEX(verify);
    for(auto i = toks.begin(); i != toks.end(); ++i)
        EDGE(v, vx, *i, chk);
    return vx;
}
int ASTGenerator::TOKS(int v, std::initializer_list<int> toks, Callback verify, Callback chk) {
    return d->TOKS(v, toks, verify, chk);
}
