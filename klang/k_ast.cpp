#include "k_ast.h"
using namespace K::Lang;

KGenerator::KGenerator()
    : ASTGenerator(keywords)
    , d(new KGeneratorPrivate)
{
    //forward declaration:
    d->expr         = VERTEX();
    d->resolve_elt  = VERTEX();
    d->cns_root     = VERTEX();

    EDGES(cns_root, cns_root, {KEYWORD_PUBLIC,KEYWORD_PROTECTED,KEYWORD_PRIVATE},
          [=](){return access_modifier();});

    /*expression*/ {
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

        //apply any prefix operators
        EDGE(expr, expr, TOKEN_OPERATOR, [=](){return expr_prefix_operator();});
        //element
        int elt = VERTEX([=](){return expr_element();});
        CALL_RESOLVE_ELT(expr, elt);
        //subexpression in brackets
        EDGE(expr, expr, TOKEN_LBR, [=](){return expr_subexpr_start();});
        //"xxx.yyy" or "xxx->yyy"
        int sel = TOKS(elt, {TOKEN_OPERATOR,TOKEN_DOT}, [=](){return expr_member_operator();});
        CALL_RESOLVE_ELT(sel, elt);
        //calls or indexes
        int op_bracket = VERTEX();
        int op_index   = VERTEX();
        EDGE(elt, op_bracket, TOKEN_LBR, [=](){return expr_call_operator();});
        EDGE(elt, op_index, TOKEN_LIDX, [=](){return expr_call_operator();});
        int op_end     = VERTEX();
        EDGE(op_bracket, op_end, TOKEN_RBR, [=](){return expr_call_end();});
        EDGE(op_index, op_end, TOKEN_RIDX, [=](){return expr_call_end();});
        //post operators
        int post = TOKS(expr, {TOKEN_INT,TOKEN_OPERATOR,TOKEN_FLOAT,TOKEN_STR1,TOKEN_STR2},
                       [=](){return expr_immediate();});
        int attrs = TOKS(expr, {KEYWORD_ALIGNOF,KEYWORD_OFFSETOF,KEYWORD_SIZEOF,KEYWORD_TYPEOF},
                       [=](){return expr_attrib_operator();});
        CALL_SIMPLE_EXPR(attrs, post);
        NEXT(elt, post);
        //(....)
        EDGE(post, expr, TOKEN_RBR, [=](){return expr_subexpr_end();});
        EDGE(post, post,TOKEN_OPERATOR, [=](){return expr_postfix_operator();});
        EDGE(post, expr, TOKEN_OPERATOR, [=](){return expr_operator();});
        EDGE(post, expr, TOKEN_ASSIGN, [=](){return expr_assign();});
        RET(post, [=](){return expr_end();});
    }

    /*resolve element*/ {
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
        int this_elt  = TOK(resolve_elt, KEYWORD_THIS, [=](){return resolve_this();});
        RET(this_elt, [=](){return resolve_end();});

        int st_elt    = TOKS(resolve_elt, {
                                 KEYWORD_TYPE,
                                 KEYWORD_INT,
                                 KEYWORD_UINT,
                                 KEYWORD_BITVEC,
                             }, [=](){return resolve_simple_type();});
        RET(st_elt, [=](){return resolve_end();});

        int find_elt  = VERTEX();
        EDGE(resolve_elt, find_elt, TOKEN_SC2, [=](){return resolve_to_root();});
        NEXT(resolve_elt, find_elt);

        int id   = TOK(resolve_elt, TOKEN_IDENT, [=](){return resolve_element();});
        EDGE(id, find_elt, TOKEN_SC2);//id::...
        //templates: X[Y::Z, FOO]
        int templ = VERTEX();
        EDGE(id, templ, TOKEN_LIDX, [=](){return resolve_template_begin();});
        //X[ident:value]
        int namedarg = TOK(templ, TOKEN_IDENT, [=](){return resolve_template_arg_name();},
                                               [=](){return resolve_template_contains_arg();});
        namedarg  = TOK(namedarg, TOKEN_SC);
        //X[value]
        int templarg = VERTEX([=](){return resolve_template_arg();});
        CALL_SIMPLE_EXPR(namedarg, templarg);
        CALL_SIMPLE_EXPR(templ, templarg);
        EDGE(templarg, templ, TOKEN_COMMA);
        int tend  = TOK(templarg, TOKEN_RIDX, [=](){return resolve_template_end();});
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
        int op2  = TOK(op1, TOKEN_OPERATOR, [=](){return resolve_operator();});
        RET(op2);
        //operator ()
        int op_brk = TOK(op1, TOKEN_LBR);
            op_brk = TOK(op_brk, TOKEN_RBR, [=](){return resolve_operator_brackets();});
        //operator []
        int op_idx = TOK(op1, TOKEN_LIDX);
            op_idx = TOK(op_idx, TOKEN_RIDX, [=](){return resolve_operator_index();});

        //resolve by parameters
        int find_param = VERTEX();
        EDGE(id, find_param, TOKEN_LIDX, [=](){return resolve_arg_begin();});
        EDGE(op2, find_param, TOKEN_LIDX, [=](){return resolve_arg_begin();});
        EDGE(op_brk, find_param, TOKEN_LIDX, [=](){return resolve_arg_begin();});
        EDGE(op_idx, find_param, TOKEN_LIDX, [=](){return resolve_arg_begin();});
        //xxx()
        int find_param_end = TOK(find_param, TOKEN_RIDX, [=](){return resolve_arg_end();});
        //xxx(const volatile ...
        int find_param2 =TOKS(find_param, {
                  KEYWORD_CONST,
                  KEYWORD_VOLATILE,
                  KEYWORD_IN,
                  KEYWORD_INOUT,
                  KEYWORD_OUT
              }, [=](){return resolve_arg_modifier();});
        EDGES(find_param2,find_param2,{
                  KEYWORD_CONST,
                  KEYWORD_VOLATILE,
                  KEYWORD_IN,
                  KEYWORD_INOUT,
                  KEYWORD_OUT
              }, [=](){return resolve_arg_modifier();});
        //xxxx( [modifiers] type ...
        int param_type = VERTEX([=](){return resolve_arg_type();});
        CALL_RESOLVE_ELT(find_param, param_type);
        CALL_RESOLVE_ELT(find_param2, param_type);
        //xxx( [modifiers] type, moreargs...
        EDGE(param_type, find_param2, TOKEN_COMMA, [=](){return resolve_arg_next();});
        //xxx( param1, param2 )
        EDGE(param_type, find_param_end, TOKEN_RBR, [=](){return resolve_arg_end();});
        //xxx( param1, param2 ) const|static|volatile|final
        int postmod = VERTEX();
        EDGES(find_param_end, postmod, {
                  KEYWORD_CONST,
                  KEYWORD_VOLATILE,
                  KEYWORD_STATIC
              }, [=](){return resolve_postmod();});
        EDGES(postmod, postmod, {
                  KEYWORD_CONST,
                  KEYWORD_VOLATILE,
                  KEYWORD_STATIC
              }, [=](){return resolve_postmod();});

        RET(id, [=](){return resolve_end();});
        RET(tend, [=](){return resolve_end();});
        RET(postmod, [=](){return resolve_end();});
        RET(find_param_end, [=](){return resolve_end();});
    }

    /*classes*/ {
        //class X, struct X
        int klass = VERTEX();
        EDGE(cns_root, klass, KEYWORD_CLASS, [=](){return gen_class();});
        EDGE(cns_root, klass, KEYWORD_STRUCT, [=](){return gen_struct();});

        klass = TOK(klass, TOKEN_IDENT, [=](){return element_name();});

        //class F[A,B,...]
        int templ = TOK(klass, TOKEN_LIDX);
        //class F[class A...
        int ta_cls= TOK(templ, KEYWORD_CLASS);
        ta_cls    = TOK(ta_cls, TOKEN_IDENT, [=](){return cls_add_parameter_cls();});
        //class F[var A...
        int ta_imm= TOK(templ, KEYWORD_VAR);
        ta_imm    = TOK(ta_imm, TOKEN_IDENT, [=](){return cls_add_parameter_imm();});
        //class F[var A=...
        //class F[class A=...
        int tv_def= TOK(ta_cls, TOKEN_ASSIGN);
        EDGE(ta_imm, tv_def, TOKEN_ASSIGN);
        //class F[var A=smth...
        int tv_chk= VERTEX([=](){return cls_def_parameter();});
        CALL_RESOLVE_ELT_SIMPLE(tv_def, tv_chk);
        //class F[var A,...
        EDGE(tv_chk, templ, TOKEN_COMMA);
        EDGE(ta_cls, templ, TOKEN_COMMA);
        EDGE(ta_imm, templ, TOKEN_COMMA);
        //class F[class A=foo,var B]
        //cls_end_template() must be called to generate parameters
        //for later use(class A[class B] : public C[B])
        int t_end = VERTEX([=](){return cls_end_template();});
        EDGE(tv_chk, t_end, TOKEN_RIDX);
        EDGE(ta_cls, t_end, TOKEN_RIDX);
        EDGE(ta_imm, t_end, TOKEN_RIDX);

        //class X : public Y, private Z
        //class X[A,B] : protected W[B]
        int derive = VERTEX();
        EDGE(klass, derive, TOKEN_SC);
        EDGE(t_end, derive, TOKEN_SC);
        int access = TOKS(derive, {KEYWORD_PUBLIC, KEYWORD_PROTECTED, KEYWORD_PRIVATE},
                      [=](){return cls_derive_access();});
        int refobj = VERTEX([=](){return cls_add_parent();});
        CALL_RESOLVE_ELT_SIMPLE(access, refobj);
        //class X : public A,...
        EDGE(refobj, derive, TOKEN_COMMA);
        //class X;
        //class X[class T];
        //class X : public Y;
        //class X[class T] : public Y;
        RET(klass, [=](){return element_completed();});
        RET(t_end, [=](){return element_completed();});
        RET(refobj,[=](){return element_completed();});
    }

    /*namespace*/ {
        int subns = TOK(cns_root, KEYWORD_NAMESPACE, [=](){return gen_namespace();});
        subns = TOK(subns, TOKEN_IDENT, [=](){return element_name();});
        RET(subns, [=](){return element_completed();});
    }

    /*enum, flags*/ {
        int en1 = VERTEX();
        EDGE(cns_root, en1, KEYWORD_ENUM, [=](){return gen_enum();});
        EDGE(cns_root, en1, KEYWORD_FLAGS, [=](){return gen_flags();});
        int en2 = TOK(en1, TOKEN_IDENT, [=](){return element_name();});
        //unnamed enum: enum {}
        RET(en1, [=](){return element_completed();});
        //named enum: enum A{}
        RET(en2, [=](){return element_completed();});
    }

    /*function*/ {
        int fn1 = TOK(cns_root, fn, KEYWORD_FUNCTION, [=](){return gen_function();});
        int fn2 = TOK(fn1, TOKEN_IDENT, [=](){return element_name();});
        int ftype1 = TOK(fn1, TOKEN_LIDX);
        int ftype2 = VERTEX([=](){return element_type();});
        CALL_RESOLVE_ELT_SIMPLE(ftype1, ftype2);
        int ftype3 = TOK(ftype2, TOKEN_RIDX);
        NEXT(ftype3, fn2);

        fn2 = TOK(fn2, TOKEN_LBR);
        int fn3 = TOK(fn3, TOKEN_RBR);

        int argtype = VERTEX([=](){return func_arg_type();});
        CALL_RESOLVE_ELT_SIMPLE(fn2, argtype);
        int argname = TOK(argtype, TOKEN_IDENT, [=](){return func_arg_name();});
        EDGE(argname, fn2, TOKEN_COMMA);
        EDGE(argname, fn3, TOKEN_RBR);
        RET(fn3, [=](){return element_completed();});
    }

}
void ASTGeneratorK::CALL_RESOLVE_ELT(int v, int after) {
    CALL(v, resolve_elt, after,
    {TOKEN_SC2, TOKEN_IDENT, KEYWORD_OPERATOR},
         [=](){return resolve_begin_full();});
}
void ASTGeneratorK::CALL_RESOLVE_ELT_SIMPLE(int v, int after) {
    CALL(v, resolve_elt, after, {TOKEN_SC2, TOKEN_IDENT, KEYWORD_OPERATOR},
         [=](){return resolve_begin_simple();});
}
//simple expression(must be computable at compile time)
//resolves to constant or type reference
void ASTGeneratorK::CALL_COMPLEX_EXPR(int v, int after) {
    CALL(v, expr, after, {-1},
         [=](){return expr_begin();});
}
void ASTGeneratorK::CALL_SIMPLE_EXPR(int v, int after) {
    ASTGenerator::CALL(v, expr, after,
    {TOKEN_INT, TOKEN_OPERATOR,
                          TOKEN_FLOAT,TOKEN_STR1,TOKEN_STR2,
                          TOKEN_SC2, TOKEN_IDENT},
         [=](){return expr_begin_simple();});
}
int ASTGeneratorK::expr_begin() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_end() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_begin_simple() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_prefix_operator() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_postfix_operator() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_member_operator() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_operator() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_attrib_operator() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_call_operator() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_call_arg_name() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}
int ASTGeneratorK::expr_call_arg_value() {
    qDebug("%s", __FUNCTION__);
    return CONTINUE;
}


