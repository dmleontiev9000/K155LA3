#include "k_tokenizer.h"

using namespace K::Lang;

Tokenizer::KW K::KPP::Tokenizer::keywords[] = {
    _KW("int?",     KEYWORD_INT),
    _KW("uint?",    KEYWORD_UINT),
    _KW("bitvec?",  KEYWORD_BITVEC),
    _KW("type",     KEYWORD_TYPE),
    _KW("this",     KEYWORD_THIS),

    _KW("sizeof",   KEYWORD_SIZEOF),
    _KW("typeof",   KEYWORD_TYPEOF),
    _KW("alignof",  KEYWORD_ALIGNOF),
    _KW("offsetof", KEYWORD_OFFSETOF),

    _KW("static",   KEYWORD_STATIC),
    _KW("virtual",  KEYWORD_VIRTUAL),
    _KW("final",    KEYWORD_FINAL),
    _KW("const",    KEYWORD_CONST),
    _KW("mutable",  KEYWORD_MUTABLE),

    _KW("in",       KEYWORD_IN),
    _KW("out",      KEYWORD_OUT),
    _KW("inout",    KEYWORD_INOUT),

    _KW("public",   KEYWORD_PUBLIC),
    _KW("protected",KEYWORD_PROTECTED),
    _KW("private",  KEYWORD_PRIVATE),

    _KW("enum",     KEYWORD_ENUM),
    _KW("flags",    KEYWORD_FLAGS),
    _KW("typedef",  KEYWORD_TYPEDEF),
    _KW("namespace",KEYWORD_NAMESPACE),
    _KW("struct",   KEYWORD_STRUCT),
    _KW("class",    KEYWORD_CLASS),

    _KW("constructor", KEYWORD_CONSTRUCTOR),
    _KW("destructor", KEYWORD_DESTRUCTOR),
    _KW("signal",   KEYWORD_SIGNAL),
    _KW("slot",     KEYWORD_SLOT),
    _KW("property", KEYWORD_PROPERTY),
    _KW("operator", KEYWORD_OPERATOR),
    _KW("prefix",   KEYWORD_OPERATOR_PREF),
    _KW("postfix",  KEYWORD_OPERATOR_POST),
    _KW("member",   KEYWORD_OPERATOR_MEM),
    _KW("new",      KEYWORD_NEW),
    _KW("delete",   KEYWORD_DELETE),
    _KW("move",     KEYWORD_MOVE),
    _KW("copy",     KEYWORD_COPY),
    _KW("function", KEYWORD_FUNCTION),
    _KW("var",      KEYWORD_VAR),
    _KW("assert",   KEYWORD_ASSERT),

    _KW("if",       KEYWORD_IF),
    _KW("else",     KEYWORD_ELSE),
    _KW("while",    KEYWORD_WHILE),
    _KW("for",      KEYWORD_FOR),
    _KW("break",    KEYWORD_BREAK),
    _KW("continue", KEYWORD_CONTINUE),
    _KW("switch",   KEYWORD_SWITCH),
    _KW("case",     KEYWORD_CASE),
    _KW("default",  KEYWORD_DEFAULT),

    {0,0,0,0}
};
K::KPP::Tokenizer::Tokenizer()
    : K::Lang::Tokenizer(keywords)
{}
K::KPP::Tokenizer::~Tokenizer()
{}
