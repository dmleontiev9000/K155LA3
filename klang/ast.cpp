#include "ast.h"

using namespace K::Lang::Internal;

ASTGenerator::ASTGenerator() {

}
ASTContext::ASTContext(bool parseargs, int vertex) {
    tctx.init();
    tctx.parseargs = parseargs;
    current_state  = POST_VERTEX;
    current_vertex = vertex;
}
int ASTGenerator::iteration(ASTContext *context, String *str,
                            K::function<bool ()> isinterrupted)
{
    Q_ASSERT(current == nullptr);
    Q_ASSERT(curstr  == nullptr);

    current = context;
    curstr  = str;

    do {
        int rc = STOP;
        switch(current->current_state)
        {
        case ASTContext::PRE_VERTEX:
            /*
             * PRE_VERTEX: when we are about to visit a vertex
             */
            Q_ASSERT(current->current_vertex < vertices.size());
            if (current->current_vertex < 0) {
                if (current->stack.isEmpty()) {
                    invalidTokensAtEnd();
                    break;
                } else {
                    current->current_vertex = current->stack.takeLast();
                    rc = CONTINUE;
                    break;
                }
            }
            current->current_state = ASTContext::MID_VERTEX;
        case ASTContext::MID_VERTEX:
            Q_ASSERT(current->current_vertex < vertices.size());
            Q_ASSERT(current->current_vertex >= 0);
            if (vertices[current->current_vertex].cb) {
                rc = vertices[current->current_vertex].cb();
                if (rc == NEEDMORETIME)
                    break;
                if (rc != CONTINUE)
                    break;
            }
            current->current_state = ASTContext::POST_VERTEX;
        case ASTContext::POST_VERTEX:
            Q_ASSERT(current->current_vertex < vertices.size());
            Q_ASSERT(current->current_vertex >= 0);
            current->current_edge = vertices[current->current_vertex].edg;
            current->current_state = ASTContext::PRE_EDGE;
        case ASTContext::PRE_EDGE:
            Q_ASSERT(current->current_edge < edges.size());
            Q_ASSERT(current->current_edge >= 0);
            if (!tokenize(&current->tctx)) {
                //search for exit edges
                do {
                    if (edges[current->current_edge].input >= 0) {
                        current->current_edge = edges[current->current_edge].next;
                        Q_ASSERT(current->current_edge < edges.size());
                        Q_ASSERT(current->current_edge >= 0);
                        continue;
                    }


                }
                if ()
            }
            if (current->current_edge < 0) {

            }
        case ASTContext::MID_EDGE:
            if ()
        case ASTContext::POST_EDGE:

        default:
            Q_UNREACHABLE;
        }
        int rc;
        if (current->current_func) {
            rc = current->current_func();
            if (rc == NEEDMORETIME)
                continue;
            current->current_func = K::function<bool ()>();
        }


        Q_ASSERT(current->current_vertex < vertices.size());

        bool have_token = tokenize(&current->tctx);
        if (have_token) {
            if (current->current_vertex < 0) {
                invalidTokensAtEnd();
                return STOP;
            }
            int n = vertices[current->current_vertex].edg;
            while(n >= 0) {
                Q_ASSERT(n < edges.size());

                if (edges[n].input >= 0 &&
                    edges[n].input != current->tctx.token_out)
                {
                    n = edges[n].next;
                    continue;
                }
                if (edges[n].chk) {
                    int ret = edges[n].chk();
                    if (ret == NEEDMORETIME) {

                    }
                    if (ret == IGNORE) {
                        n = edges[n].next;
                        continue;
                    }
                    if (ret != CONTINUE) {
                        return ret;
                    }
                }
                if (edges[n].push >= 0)
                current->stack.push_back(edges[n].push);
                current->current_vertex = edges[n].target;
                if (current->current_vertex < 0) {
                    if (!current->stack.isEmpty())
                        current->current_vertex = current->stack.takeLast();
                }
                Q_ASSERT(current->current_vertex < vertices.size());
                if (current->current_vertex >= 0) {
                    if (vertices[current->current_vertex].cb)
                }
            }
        }

    } while (!isinterrupted());

    current = nullptr;
    curstr  = nullptr;
}
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
