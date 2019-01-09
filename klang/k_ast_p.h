#pragma once
#include "lang.h"
#include "k_ast.h"
#include <QStack>

#define F(func) \
    [=](const InterruptTest& i) \
    { return d->mContext->func(i);}
#define FV(func, ...) \
    [=](const InterruptTest&) \
    { return d->mContext->func(, __VA_ARGS__);}
#define D(func) \
    K::Lang::RC func\
        (const K::Lang::InterruptTest& i)
#define DV(func, ...) \
    K::Lang::RC func\
        (const K::Lang::InterruptTest& i, __VA_ARGS__)
#define I(func) \
    K::Lang::RC \
    K::Lang::KContextPrivate::func\
        (const K::Lang::InterruptTest& i)
#define IV(func, ...) \
    K::Lang::RC \
    K::Lang::KContextPrivate::func\
        (const K::Lang::InterruptTest& i, __VA_ARGS__)

namespace K {
namespace Lang {

class KContextPrivate : public K::Lang::KGenerator::Context {
private:
    Node * mNode;
private:
    enum { RESOLVER, EXPR };
    struct State {
        int type;
        State(int t) : type(t) {}
        virtual ~State();
    };
    QStack<State*> stack;

    struct Resolver {
        enum { U = 0x1, //search for entry from current scope
               P = 0x2, //search in parent class (may include class name)
               M = 0x4, //search for member
               T = 0x8, //this is available
             };
        Resolver() : State(RESOLVER) {}
        ~Resolver();
        bool        onlytypes;
        int         state = U;
        Reference   scope;
        Reference   iterator;
        QVector<Reference*> variants;
    };

    /*
     * EXPR
     */
    struct Expression {
        bool          expr_mode;
        bool          expr_is_simple;
        QVector<uint> expr_elements;
        QVector<QVariant> expr_imms;
        QVector<Reference*> expr_refs;
    };

    bool expr_still_valid();
public:
    DV(resolve_begin, bool onlytypes);
    D(resolve_this);
    D(resolve_simple_type);
    D(resolve_to_root);

    D(expr_begin);
    D(expr_begin_simple);
    D(expr_end);

    D(expr_immediate);
    D(expr_prefix_operator);
    D(expr_postfix_operator);
    D(expr_binary_operator);
    D(expr_attrib_operator);
    D(expr_call_operator);
    D(expr_call_arg_name);
    D(expr_call_arg_value);
    D(expr_call_end);
    D(expr_immediate);
    D(expr_element);


    D(access_modifier);
    D(gen_class);
    D(gen_struct);
    D(gen_enum);
    D(gen_flags);
    D(gen_namespace);
    D(gen_unk);
    D(gen_function);
    D(element_name);
    D(element_completed);
    D(element_type);
    D(func_arg_type);
    D(func_arg_name);
};

class KGeneratorPrivate {
public:
    KGeneratorPrivate() {}

    KContextPrivate * mContext = nullptr;
    //element resolution(A::B::C, ::X::operator Y, foo)
    int resolve_elt = -1;
    //simple expression(must be computable at compile time)
    //resolves to constant or type reference
    int expr        = -1;
    //
    int cns_root    = -1;

};

}//namespace Lang
}//namespace K
