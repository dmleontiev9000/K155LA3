#include "k_ast.h"
#include "errors.h"
#include "k_ast_p.h"

using namespace K::Lang;
using namespace K::Lang::T;
/*
 * search algorithm:
 *
 * initial mode is U (up)
 *
 * *root
 *   |
 *   +--class A (11)
 *   |   |
 *   |   +--func lol() (8)
 *   |
 *   +--namespace B (10)
 *   |   |
 *   |   +--class C : public A (9)
 *   |   |   |
 *   |   |   +--[public A] (7)
 *   |   |   |
 *   |   |   +--int member1 (6)
 *   |   |   |
 *   |   |   +--func foo(int b) (5)
 *   |   |   |   |
 *   |   |   |   +--[int b] (3)
 *   |   |   |   |
 *   |   |   |   +--something; (2)
 *   |   |   |   |
 *   |   |   |   +--if (b > member1) (1)
 *   |   |   |   |   |
 *   |   |   |   |   +--member2=lol()+member1; <<we are here
 *   |   |   |
 *   |   |   +--float member2 (4)
 *   |   |   |
 *
 *  our search order:
 *     if reference starts with ::,
 *        iterator = last child of root
 *        state = ...
 *     else
 *        iterator = current node
 *        state = go_up|can_access_private
 *
 *     on first iteration we see member2
 *     and state=go_up|...
 *        we walk up to func foo().
 *        since func, or scope(if,else,for,while..)
 *        are executeable blocks, we
 */

KContextPrivate::Resolver::~Resolver() {
    clearPath();
}

bool  KContextPrivate::Resolver::check() {
    for(auto i = callstack.constBegin();
        i != callstack.constEnd(); ++i) {
        if (!i->get())
            return false;
    }
    for(auto i = matches.constBegin();
        i != matches.constEnd(); ++i) {
        if (!i->get())
            return false;
    }
    return iterator.get() != nullptr;
}
Node* KContextPrivate::Resolver::popPath() {
    if (callstack.isEmpty())
        return nullptr;
    auto ref = callstack.pop();
    auto q = ref->get();
    delete ref;
    return q;
}
void KContextPrivate::Resolver::pushPath(Node *node) {
    callstack.push(new Reference(node));
}
void KContextPrivate::Resolver::clearPath() {
    for(auto i = callstack.constBegin();
        i != callstack.constEnd(); ++i)
        delete *i;
    callstack.clear();
}
void KContextPrivate::Resolver::addMatch(Node *node) {
    matches.push(new Reference(node));    
}
void KContextPrivate::Resolver::clearMatches() {
    for(auto i = matches.constBegin();
        i != matches.constEnd(); ++i)
        delete *i;
    matches.clear();
}

IV(resolve_begin, bool onlytypes) {
    /*
     * full: resolves every element
     * simple: resolves only type names
     */
    auto p = mNode->parent();
    if (!p) return RC::INTERNAL_ERROR;

    auto r       = new Resolver();
    r->onlytypes = onlytypes;
    r->cangoup   = true;
    r->cangocls  = true;
    r->isfinal   = false;
    switch(p->type()) {
    case NODE_NAMESPACE:
    case NODE_CLASS:
    case NODE_TEMPLATE:
        r->iterator = p->lastChild();
        break;
    default:
        r->iterator = mNode->previous();
    }

    stack.push(r);
    return RC::CONTINUE;
}
I(resolve_to_root) {
    if (stack.isEmpty())
        return RC::INTERNAL_ERROR;
    if (stack.top()->type != RESOLVER)
        return RC::INTERNAL_ERROR;
    auto r = static_cast<Resolver*>(stack.top());
    //find root node
    auto node = mNode;
    for(auto par  = node->parent(); par;) {
        node = par;
        par  = par->parent();
    }
    r->state = 0;
    r->iterator = node->lastChild();
    return RC::CONTINUE;
}
I(resolve_element) {
    if (stack.isEmpty())
        return RC::INTERNAL_ERROR;
    if (stack.top()->type != RESOLVER)
        return RC::INTERNAL_ERROR;
    auto r = static_cast<Resolver*>(stack.top());
    int rep_count = 0;
    for(;;) {
        Node *j, *p;

        //check for events every 16th cycle
        if ((++rep_count & 15)==0)
            if (itest && !itest()) return RC::INTERRUPTED;
        if (!r->check())
            return RC::RESET;

        j = r->iterator.get();
        if (!j->complete()) return RC::BLOCKED;

        switch(j->type()) {
        case NODE_ENUM:
            if (r->onlytypes)
                break;
        case NODE_NAMESPACE:
        case NODE_TYPE:
            if (matchNodeName(j)) {
                r->cangoup  = false;
                r->iterator = j->lastChild();
                r->clearPath();
                r->addMatch(j);
                colorify(S::COLOR_MEMBER);
                return RC::CONTINUE;
            }
            break;
        case NODE_PARENTTYPE:
            if (matchNodeName(j)) {
                r->cangocls = false;
                r->cangoup  = false;
                r->clearPath();
                r->addMatch(j);
                colorify(S::COLOR_MEMBER, j);
                r->iterator = j->declType();
            } else if (r->cangocls) {
                r->pushPath(j);
                r->iterator = j->declType();
            }
            break;
        case NODE_PARAMETER:
            if (r->onlytypes & !j->isType())
                break;
            if (matchNodeName(j)) {
                r->cangoup  = false;
                r->cangocls = false;
                r->clearPath();
                r->addMatch(j);
                colorify(S::COLOR_MEMBER, j);
                return RC::CONTINUE;
            }
            break;
        case NODE_VARIABLE:
        case NODE_CONST:
        case NODE_ARGUMENT:
            if (r->onlytypes)
                break;
            if (!matchNodeName(j))
                break;
            r->cangoup  = false;
            r->cangocls = false;
            r->isfinal  = true;
            r->clearPath();
            r->addMatch(j);
            colorify(j->type == NODE_ARGUMENT ? 
                         S::COLOR_ARGUMENT: 
                         S::COLOR_MEMBER, j);
            return RC::CONTINUE;
        case NODE_TEMPLATE:
        case NODE_FUNCTION:
            //many functions can be found using the same name
            //because of overloading
            if (r->onlytypes && matchNodeName(j)) 
                r->addMatch(j);
            break;
        default:;
        }
        /*
         * go to previous element in list, if any
         */
        auto pj = j->previous();
        if (!pj && !r->callstack.isEmpty()) {
            //don't go up if callstack is not empty
            //return to last item in stack
            auto ref = r->callstack.pop();
            auto j = ref.get();
            Q_ASSERT(j);
            delete ref;
            pj = j->previous();
        }
        if (!pj && r->cangoup) {
            auto pp = j->parent();
            if (pp) {
                if (!pp->valid()) 
                    return RC::RESET;
                pj = pp;
                if (pp->type() == NODE_CLASS ||
                    pp->type() == NODE_TEMPLATE ||
                    pp->type() == NODE_NAMESPACE)
                    pj = pp->lastChild();
            }
        }
        if (!pj) {
            if (r->matches.isEmpty()) {
                set_error(SyntaxErrors::identifier_not_found);
                return RC::ERROR;
            }
            return RC::CONTINUE;
        }
        r->iterator = pj;
    }
}
I(resolve_this) {
    if (stack.isEmpty())
        return RC::INTERNAL_ERROR;
    if (stack.top()->type != RESOLVER)
        return RC::INTERNAL_ERROR;
    auto r = static_cast<Resolver*>(stack.top());
    if (!r->full) {
        set_error(SyntaxErrors::this_cannot_be_used_here);
        return RC::ERROR;
    }
    auto node = mNode;
    do {
        switch(node->type()) {
        case NODE_CLASS:
            //FIXME: emit 'this' instruction
            return RC::CONTINUE;
        case NODE_SCOPE:
            node = node->parent();
            break;
        case NODE_FUNCTION:
            if (!node->isStatic()) {
                node = node->parent();
                break;
            }
        default:
            node = nullptr;
        }
    } while(node);
    set_error(SyntaxErrors::this_not_available_here);
    return RC::ERROR;
}
I(resolve_simple_type) {
    if (stack.isEmpty())
        return RC::INTERNAL_ERROR;
    if (stack.top()->type != RESOLVER)
        return RC::INTERNAL_ERROR;
    auto r = static_cast<Resolver*>(stack.top());
    if (r->scope || r->iterator)
        return RC::INTERNAL_ERROR;
    //FIXME: check if r is empty
    switch(token()) {
    case KEYWORD_TYPE:
        //FIXME: emit 'type' instruction
    case KEYWORD_INT:
        //FIXME: emit 'int' instruction with detail
    case KEYWORD_UINT:
        //FIXME: emit 'uint' instruction with detail
    case KEYWORD_FLOAT:
        //FIXME: emit 'float' instruction with detail
    case KEYWORD_BITVEC:
        //FIXME: emit 'float' instruction with detail
    default:
        return RC::INTERNAL_ERROR;
    }
}



void KGenerator::CALL_RESOLVE_ELT(int v, int after) {
    Q_ASSERT(d->resolve_elt >= 0);
    CALL(v, d->resolve_elt, after,
         {TOKEN_SC2, TOKEN_IDENT, KEYWORD_OPERATOR},
         F(resolve_begin_full));
}
void KGenerator::CALL_RESOLVE_ELT_SIMPLE(int v, int after) {
    Q_ASSERT(d->resolve_elt >= 0);
    CALL(v, d->resolve_elt, after,
         {TOKEN_SC2, TOKEN_IDENT, KEYWORD_OPERATOR},
         F(resolve_begin_simple));
}
void KGenerator::langAddResolve() {
    /*
     * @-->|-->[this]------>X
     *     |
     *     |-->[stdtype]--->X
     *     |
     *     |-->[imm]------->X
     *     |
     *     |-->[::]-->|-->[elt]-------------->|-->X
     *     |          |                       |
     *     |--------->|<--[::]<---------------|-->[-->|-->[id]-->[:]-->|-->@@--|-->]-->|
     *                |                               |                |       |       |
     *                |                               |--------------->|       |       |
     *                |                               |                        |       |
     *                |-->[operator]-->[op]-->|-->[-->|-->|<--[,]--------------|       |
     *                |                       |
     *                |                       |-->X
     *                |
     *                |
     *
     *
     *
     */
    auto resolve_elt = d->resolve_elt;
    Q_ASSERT(resolve_elt >= 0);

    int this_elt  = TOK(resolve_elt, KEYWORD_THIS, F(resolve_this));
    RET(this_elt, F(resolve_end));

    int st_elt    = TOKS(resolve_elt, {
                             KEYWORD_TYPE,
                             KEYWORD_INT,
                             KEYWORD_UINT,
                             KEYWORD_BITVEC,
                             KEYWORD_FLOAT,
                         }, F(resolve_simple_type));
    RET(st_elt, [=](){return resolve_end();});

    int find_elt  = VERTEX();
    EDGE(resolve_elt, find_elt, TOKEN_SC2, F(resolve_to_root));
    NEXT(resolve_elt, find_elt);

    int id   = TOK(resolve_elt, TOKEN_IDENT, F(resolve_element));
    EDGE(id, find_elt, TOKEN_SC2);//id::...
    //templates: X[Y::Z, FOO]
    int templ = VERTEX();
    EDGE(id, templ, TOKEN_LIDX, F(resolve_template_begin));
    //X[ident:value]
    int namedarg = TOK(templ, TOKEN_IDENT, F(resolve_template_arg_name),
                                           F(resolve_template_contains_arg));
    namedarg  = TOK(namedarg, TOKEN_SC);
    //X[value]
    int templarg = VERTEX(F(resolve_template_arg));
    CALL_SIMPLE_EXPR(namedarg, templarg);
    CALL_SIMPLE_EXPR(templ, templarg);
    EDGE(templarg, templ, TOKEN_COMMA);
    int tend  = TOK(templarg, TOKEN_RIDX, F(resolve_template_end));
    EDGE(tend, find_elt, TOKEN_SC2);//id::...

    //X::operator +
    int op1  = TOKS(find_elt, {KEYWORD_OPERATOR,
                               KEYWORD_OPERATOR_MEM,
                               KEYWORD_OPERATOR_POST,
                               KEYWORD_OPERATOR_PREF,
                               KEYWORD_NEW,
                               KEYWORD_DELETE,
                               KEYWORD_MOVE,
                               KEYWORD_COPY});
    int op2  = TOK(op1, TOKEN_OPERATOR, F(resolve_operator));
    RET(op2);
    //operator ()
    int op_brk = TOK(op1, TOKEN_LBR);
        op_brk = TOK(op_brk, TOKEN_RBR, F(resolve_operator_brackets));
    //operator []
    int op_idx = TOK(op1, TOKEN_LIDX);
        op_idx = TOK(op_idx, TOKEN_RIDX, F(resolve_operator_index));

    //resolve by parameters
    int find_param = VERTEX();
    EDGE(id, find_param, TOKEN_LIDX, F(resolve_arg_begin));
    EDGE(op2, find_param, TOKEN_LIDX, F(resolve_arg_begin);
    EDGE(op_brk, find_param, TOKEN_LIDX, F(resolve_arg_begin));
    EDGE(op_idx, find_param, TOKEN_LIDX, F(resolve_arg_begin));
    //xxx()
    int find_param_end = TOK(find_param, TOKEN_RIDX, F(resolve_arg_end));
    //xxx(const volatile ...
    int find_param2 =TOKS(find_param, {
              KEYWORD_CONST,
              KEYWORD_VOLATILE,
              KEYWORD_IN,
              KEYWORD_INOUT,
              KEYWORD_OUT
          }, F(resolve_arg_modifier));
    EDGES(find_param2,find_param2,{
              KEYWORD_CONST,
              KEYWORD_VOLATILE,
              KEYWORD_IN,
              KEYWORD_INOUT,
              KEYWORD_OUT
          }, F(resolve_arg_modifier));
    //xxxx( [modifiers] type ...
    int param_type = VERTEX(F(resolve_arg_type));
    CALL_RESOLVE_ELT(find_param, param_type);
    CALL_RESOLVE_ELT(find_param2, param_type);
    //xxx( [modifiers] type, moreargs...
    EDGE(param_type, find_param2, TOKEN_COMMA, F(resolve_arg_next));
    //xxx( param1, param2 )
    EDGE(param_type, find_param_end, TOKEN_RBR, F(resolve_arg_end));
    //xxx( param1, param2 ) const|static|volatile|final
    int postmod = VERTEX();
    EDGES(find_param_end, postmod, {
              KEYWORD_CONST,
              KEYWORD_VOLATILE,
              KEYWORD_STATIC
          }, F(resolve_postmod));
    EDGES(postmod, postmod, {
              KEYWORD_CONST,
              KEYWORD_VOLATILE,
              KEYWORD_STATIC
          }, F(resolve_postmod));

    RET(id, F(resolve_end));
    RET(tend, F(resolve_end));
    RET(postmod, F(resolve_end));
    RET(find_param_end, F(resolve_end));
}
