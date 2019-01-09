#include "k_ast.h"
#include "k_ast_p.h"
#include "syntaxerrors.h"
using namespace K::Lang;
using namespace K::Lang::T;

KContextPrivate::State::~State() {}

enum { NOP,
       PREFIX_OP,
       POSTFIX_OP,
       BINARY_OP,
       MEMBER_OP,
       LEFT_BRACKET,
       RIGHT_BRACKET,
       IMMEDIATE,
     };

I(expr_begin) {
    /*
     * begin complex expression
     * complex expression may not be
     * evaluated at compile time,
     * but has a resulting type
     */
    exprs.push_back(Expression());
    auto& e = exprs.top();
    e.expr_mode = true;
    e.expr_is_simple = false;
    return RC::CONTINUE;
}
I(expr_begin_simple) {
    /*
     * simple expression evaluates
     * at compile time(if not, it
     * is an error). it results in
     * either immediate value(integer,
     * boolean, float, string...) or
     * in type.
     */
    exprs.push_back(Expression());
    auto& e = exprs.top();
    e.expr_mode = true;
    e.expr_is_simple = true;
    return RC::CONTINUE;
}
I(expr_immediate) {
    if (exprs.isEmpty())
        return RC::INTERNAL_ERROR;
    auto& e = exprs.top();
    e.expr_elements.append(IMMEDIATE);
    e.expr_elements.append(start());
    e.expr_elements.append(end());
    e.expr_elements.append(e.expr_imms.size());
    e.expr_imms.push_back(data());
    return RC::CONTINUE;
}
I(expr_prefix_operator) {
    if (exprs.isEmpty())
        return RC::INTERNAL_ERROR;
    auto& e = exprs.top();
    uint d = detail();
    uint priority = 1;

    switch(d) {
    case Tokenizer::tstr('+','+'):
    case Tokenizer::tstr('-','-'):
        priority = 100;
    case Tokenizer::tstr('!'):
    case Tokenizer::tstr('~'):
        e.expr_elements.push_back(PREFIX_OP);
        e.expr_elements.push_back(start());
        e.expr_elements.push_back(end());
        e.expr_elements.push_back(d);
        e.expr_elements.push_back(priority);
        return RC::CONTINUE;
    default:
        set_error(SyntaxErrors::invalid_operator);
        return RC::IGNORE;
    }
}
I(expr_postfix_operator) {
    if (exprs.isEmpty())
        return RC::INTERNAL_ERROR;
    auto& e = exprs.top();
    uint d = detail();
    uint priority = 100;

    switch(d) {
    case Tokenizer::tstr('?'):
        priority = 0;//conditional operator has lowest priority
    case Tokenizer::tstr('-','>'):
    case Tokenizer::tstr('+','+'):
    case Tokenizer::tstr('-','-'):
        e.expr_elements.push_back(POSTFIX_OP);
        e.expr_elements.push_back(start());
        e.expr_elements.push_back(end());
        e.expr_elements.push_back(d);
        e.expr_elements.push_back(priority);
        return RC::CONTINUE;
    default:
        set_error(SyntaxErrors::invalid_operator);
        return RC::IGNORE;
    }
}
I(expr_binary_operator) {
    if (exprs.isEmpty())
        return RC::INTERNAL_ERROR;
    auto& e = exprs.top();
    uint d = detail();
    uint priority = 100;

    switch(d) {
    //simple binary operators
    case Tokenizer::tstr('+'):
    case Tokenizer::tstr('-'):
    case Tokenizer::tstr('+','='):
    case Tokenizer::tstr('-','='):
        priority = 50; break;
    case Tokenizer::tstr('*'):
    case Tokenizer::tstr('/'):
    case Tokenizer::tstr('%'):
    case Tokenizer::tstr('*','='):
    case Tokenizer::tstr('/','='):
        priority = 60; break;
    case Tokenizer::tstr('<','<'):
    case Tokenizer::tstr('>','>'):
    case Tokenizer::tstr('<','<','='):
    case Tokenizer::tstr('>','>','='):
        priority = 30; break;
    case Tokenizer::tstr('&'):
    case Tokenizer::tstr('|'):
    case Tokenizer::tstr('^'):
    case Tokenizer::tstr('&','='):
    case Tokenizer::tstr('|','='):
    case Tokenizer::tstr('^','='):
        priority = 40; break;
    case Tokenizer::tstr('<'):
    case Tokenizer::tstr('>'):
    case Tokenizer::tstr('>','='):
    case Tokenizer::tstr('<','='):
    case Tokenizer::tstr('=','='):
    case Tokenizer::tstr('!','='):
        priority = 20; break;
    default:
        set_error(SyntaxErrors::invalid_operator);
        return RC::IGNORE;
    }
    e.expr_elements.push_back(BINARY_OP);
    e.expr_elements.push_back(start());
    e.expr_elements.push_back(end());
    e.expr_elements.push_back(d);
    return RC::CONTINUE;
}

RC KContextPrivate::op(uint o)
{
    if (exprs.isEmpty())
        return RC::INTERNAL_ERROR;
    uint d = detail();
    switch (o) {
    case PREFIX_OP:
        switch(d) {
        case Tokenizer::tstr('!'):
        case Tokenizer::tstr('~'):
        case Tokenizer::tstr('+','+'):
        case Tokenizer::tstr('-','-'):
            break;
        default:
            return RC::IGNORE;
        }
        break;
    case POSTFIX_OP:
        switch(d) {
        case Tokenizer::tstr('-','>'):
        case Tokenizer::tstr('+','+'):
        case Tokenizer::tstr('-','-'):
        case Tokenizer::tstr('<','<','='):
        case Tokenizer::tstr('>','>','='):
        case Tokenizer::tstr('+','='):
        case Tokenizer::tstr('-','='):
        case Tokenizer::tstr('*','='):
        case Tokenizer::tstr('/','='):
            break;
        default:
            return RC::IGNORE;
        }
        break;
    case MEMBER_OP:
        switch(d) {
        case Tokenizer::tstr('-','>'):
        case Tokenizer::tstr('.'):
            break;
        default:
            return RC::IGNORE;
        }
    case OPER
        break;
    default:;
    }
    auto& e = exprs.top();
    e.expr_elements.push_back(o);
    e.expr_elements.push_back(start());
    e.expr_elements.push_back(end());
    e.expr_elements.push_back(d);
    return RC::CONTINUE;
}

void KGenerator::CALL_COMPLEX_EXPR(int v, int after) {
    Q_ASSERT(d->expr >= 0);
    CALL(v, d->expr, after,
         -1,
         F(expr_begin));
}
void KGenerator::CALL_SIMPLE_EXPR(int v, int after) {
    Q_ASSERT(d->expr >= 0);
    CALL(v, d->expr, after,
         {TOKEN_INT, TOKEN_OPERATOR,
          TOKEN_FLOAT,TOKEN_STR1,TOKEN_STR2,
          TOKEN_SC2, TOKEN_IDENT},
         F(expr_begin_simple));
}
void KGenerator::langAddExpr() {
    /*
     *               +--->[attrs]--->+--->[=]---->@
     *               |               |
     * @--+->[opp]*->+--->[imm]----->+--->[op2]-->@
     * |  |          |               |
     * |  |          |               +--->X
     * |  |          |               |
     * |  |          |               +--->[ ) ]-->|
     * |  |          |               |            |
     * |  |          |               +<-----------+
     * |  |          |               |            |
     * |  |          |           +-->+--->[ops]-->|
     * |  |          |           |
     * |  |          |           +<------------------------+
     * |  |          |           |                         |
     * |  +--------->+--->[elt]->+-->[ [ ]-->@@-->[ ] ]--->+
     * |             |           |                         |
     * |             |           +-->[ ( ]-->@@-->[ ) ]--->+
     * |             |           |
     * +<--[ ( ]<----+           +-->[ . ]-->+
     *                           |           |
     *                           +-->[opm]-->+-->[elt]-->+
     *                           |                       |
     *                           +<----------------------+
     */

    auto expr = d->expr;
    Q_ASSERT(expr >= 0);
    //apply any prefix operators
    EDGE(expr, expr, TOKEN_OPERATOR, F(expr_prefix_operator));
    //element
    int elt = VERTEX(F(expr_element));
    EDGES(expr, elt, {TOKEN_INT, TOKEN_FLOAT, TOKEN_STR1, TOKEN_STR2},
          F(expr_immediate));
    CALL_RESOLVE_ELT(expr, elt);
    //subexpression in brackets
    EDGE(expr, expr, TOKEN_LBR, F(expr_left_bracket));
    //"xxx.yyy" or "xxx->yyy"
    int sel = TOKS(elt, {TOKEN_OPERATOR,TOKEN_DOT}, F(expr_member_operator));
    CALL_RESOLVE_ELT(sel, elt);
    //calls or indexes
    int op_bracket = VERTEX();
    int op_index   = VERTEX();
    EDGE(elt, op_bracket, TOKEN_LBR, F(expr_call_operator));
    EDGE(elt, op_index, TOKEN_LIDX, F(expr_call_operator));
    int op_end     = VERTEX();
    EDGE(op_bracket, op_end, TOKEN_RBR, F(expr_call_end));
    EDGE(op_index, op_end, TOKEN_RIDX, F(expr_call_end));
    //post operators
    int post = TOKS(expr, {TOKEN_INT,TOKEN_OPERATOR,TOKEN_FLOAT,TOKEN_STR1,TOKEN_STR2},
                   F(expr_immediate));
    int attrs = TOKS(expr, {KEYWORD_ALIGNOF,KEYWORD_OFFSETOF,KEYWORD_SIZEOF,KEYWORD_TYPEOF},
                   F(expr_attrib_operator));
    CALL_SIMPLE_EXPR(attrs, post);
    NEXT(elt, post);
    //(....)
    EDGE(post, expr, TOKEN_RBR, F(expr_subexpr_end));
    EDGE(post, post,TOKEN_OPERATOR, F(expr_postfix_operator));
    EDGE(post, expr, TOKEN_OPERATOR, F(expr_operator));
    EDGE(post, expr, TOKEN_ASSIGN, F(expr_assign));
    RET(post, F(expr_end));
}
