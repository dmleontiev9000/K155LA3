#pragma once

#include "klang_global.h"
#include "ast.h"
namespace K {
namespace Lang {
namespace Internal {

class TokenizerK : protected Tokenizer {

};
class ASTGenerator_K : public ASTGenerator {
public:
    ASTGenerator_K();
protected:
    /*
     * element resolver
     */
    virtual int resolve_begin_simple()=0;
    virtual int resolve_begin_full()=0;
    virtual int resolve_this()=0;
    virtual int resolve_simple_type()=0;
    virtual int resolve_end()=0;
    virtual int resolve_to_root()=0;
    virtual int resolve_element()=0;
    virtual int resolve_operator()=0;
    virtual int resolve_operator_brackets()=0;
    virtual int resolve_operator_index()=0;
    virtual int resolve_template_begin()=0;
    virtual int resolve_template_arg_name()=0;
    virtual int resolve_template_contains_arg()=0;
    virtual int resolve_template_arg()=0;
    virtual int resolve_template_end()=0;
    virtual int resolve_arg_begin()=0;
    virtual int resolve_arg_modifier()=0;
    virtual int resolve_arg_type()=0;
    virtual int resolve_arg_next()=0;
    virtual int resolve_arg_end()=0;
    virtual int resolve_postmod()=0;
    /*
     * expression compiler
     */
protected:
    int expr_begin();
    int expr_end();
    int expr_begin_simple();
    int expr_prefix_operator();
    int expr_postfix_operator();
    int expr_member_operator();
    int expr_operator();
    int expr_attrib_operator();
    int expr_call_operator();
    int expr_call_arg_name();
    int expr_call_arg_value();
    int expr_call_end();
    int expr_assign();
    int expr_subexpr_start();
    int expr_subexpr_end();
    int expr_immediate();
    int expr_element();
private:
    std::vector<bool> mSimpleFlag;
    struct {

    }

protected:
    /*
     * element generators
     */
    virtual int access_modifier()=0;
    virtual int gen_class()=0;
    virtual int gen_struct()=0;
    virtual int gen_enum()=0;
    virtual int gen_flags()=0;
    virtual int gen_namespace()=0;
    virtual int gen_unk()=0;
    virtual int element_name()=0;
    virtual int element_completed()=0;
    /*
     * class parameter generators
     */
    virtual int cls_add_parameter_cls()=0;
    virtual int cls_add_parameter_imm()=0;
    virtual int cls_def_parameter()=0;
    virtual int cls_end_template()=0;
    virtual int cls_derive_access()=0;
    virtual int cls_add_parent()=0;
private:
    //element resolution(A::B::C, ::X::operator Y, foo)
    int resolve_elt;
    void CALL_RESOLVE_ELT(int v, int after);
    void CALL_RESOLVE_ELT_SIMPLE(int v, int after);
    //simple expression(must be computable at compile time)
    //resolves to constant or type reference
    int expr;
    void CALL_COMPLEX_EXPR(int v, int after);
    void CALL_SIMPLE_EXPR(int v, int after);
    int cns_root;
};


} //namespace Internal
} //namespace Lang
} //namespace K
