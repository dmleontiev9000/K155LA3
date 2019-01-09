#include "k_ast.h"
#include "syntaxerrors.h"
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
 *   +--class A
 *   |   |
 *   |   +--func lol()
 *   |
 *   +--namespace B
 *   |   |
 *   |   +--class C : public A
 *   |   |   |
 *   |   |   +--[public A]
 *   |   |   |
 *   |   |   +--int member1
 *   |   |   |
 *   |   |   +--func foo(int b)
 *   |   |   |   |
 *   |   |   |   +--[int b]
 *   |   |   |   |
 *   |   |   |   +--something;
 *   |   |   |   |
 *   |   |   |   +--if (b > member1)
 *   |   |   |   |   |
 *   |   |   |   |   +--member2=lol()+member1; <<we are here
 *   |   |   |
 *   |   |   +--float member2
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
    if (scope) scope->unref();
    if (iterator) iterator->unref();
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
    r->state     = Resolver::U|Resolver::P;
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
    if (!(i->state && Resolver::U))
        return RC::INTERNAL_ERROR;

    //find root node
    auto node = mNode;
    for(auto par  = node->parent(); par;) {
        node = par;
        par  = par->parent();
    }
    r->state = Resolver::M;
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
    do {
        Node *j, *p;

        j = r->iterator.get();
        if (!j) return RC::RESET;
        if (!j->complete()) return RC::BLOCKED;

        bool skip = true;
        switch(j->type()) {
        case NODE_NAMESPACE:
        case NODE_TEMPLATE:
        case NODE_TYPE:
            if (matchNodeName(j)) {
                r->iterator = j->lastChild();
                colorify(S::COLOR_MEMBER);
                return RC::CONTINUE;
            }
            break;

        case NODE_PARENTTYPE:
            if (matchNodeName(j)) {
                r->iterator = j->lastChild();
                colorify(S::COLOR_MEMBER);
                return RC::CONTINUE;
            }

            //
            reference(Node(j))
            break;
        case NODE_PARAMETER:
            if (r->onlytypes & (j->declType() != NODE_TYPE))
                break;

            break;

        case NODE_FUNCTION:
        case NODE_VARIABLE:
        case NODE_ENUM:
        case NODE_CONST:
        case NODE_ARGUMENT:
            if (r->onlytypes)
                break;
            if (!matchNodeName(j))
                break;
            break;
        default:;
        }
        if (!skip) {

        }
        if (!skip) {
            r->variants.append(Reference::alloc(j));
            r->state &= ~Resolver::U;
        }

        auto pj = j->previous();
        if (!pj) {

        }
        switch(r->state) {
        case Resolver::U: {
            auto p = i->parent();
            if (!p) goto not_found;
            if (!p->valid()) goto reset;

            if (p->type() == NODE_CLASS ||
                p->type() == NODE_TEMPLATE ||
                p->type() == NODE_NAMESPACE)
                r->iterator.set(p->lastChild());
            r->state = Resolver::MU;}
        case Resolver::M: {
            if (i )
        }

        case Resolver::MU:


        }

        /*
         * if no more elements to search, either go up in
         * hierarchy or
         */
        if (!r->iterator) {
            if (!r->rsv_up) {
                set_error(SyntaxErrors::identifier_not_found);
                return RC::ERROR;
            }
            Node * p  = r->scope ? r->scope.target() : mNode;
            Node * pp = p->parent();
            if (!pp) {
                set_error(SyntaxErrors::identifier_not_found);
                return RC::ERROR;
            }
            Node * j = (pp->type() == NODE_SCOPE)
                ? p : pp->lastChild();
            if (!j) return RC::INTERNAL_ERROR;

            r->scope.set(pp);
            r->iterator.set(j);
            continue;
        }
        Node * j = r->iterator.target();
        if (!j->isReady())
            return RC::BLOCKED;

        bool skip = true;
        switch(j->type()) {
        case NODE_NAMESPACE:
        case NODE_TYPE:
            skip = false;
            break;
        case NODE_PARAMETER:
            if (j->declType())
        case NODE_FUNCTION:
        case NODE_VARIABLE:
        case NODE_ENUM:
        case NODE_CONST:
        case NODE_ARGUMENT:
            skip = !r->full;
            break;
        default:;
        }
        /*
         * check if element can be matched by identifier
         */
        if (!skip)
            skip = !j->matchName(identifier());
        if (!skip) {

        }
        r->iterator.set(j->prevNode());
    } while(!i || i());
    return RC::INTERRUPTED;
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
