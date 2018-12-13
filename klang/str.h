#pragma once

#include "klang_global.h"
#include <assetpool.h>
#include <QVector>
#include <QElapsedTimer>
#include <QVariant>

namespace K {
namespace Lang {
namespace Internal {

enum {
    TOKEN_INVALID,
    TOKEN_ERROR,
    TOKEN_LBR,     //(
    TOKEN_RBR,     //)
    TOKEN_LIDX,    //[
    TOKEN_RIDX,    //]
    TOKEN_COMMA,   //,
    TOKEN_DOT,     //.
    TOKEN_DOT3,    //...
    TOKEN_IDENT,   //bla_4
    TOKEN_INT,     //0x213, 50
    TOKEN_FLOAT,   //1.34f
    TOKEN_SC,      //:
    TOKEN_SC2,     //::
    TOKEN_STR1,    //'aaa'
    TOKEN_STR2,    //"aaa"
    TOKEN_ASSIGN,  //=
    TOKEN_OPERATOR,//+ - *= == (but not =)
    
    //keywords
    KEYWORD_CONST,//+
    KEYWORD_VOLATILE,//+
    KEYWORD_STATIC,//+
    KEYWORD_FINAL,
    KEYWORD_MUTABLE,
    KEYWORD_VIRTUAL,

    KEYWORD_IN,
    KEYWORD_OUT,
    KEYWORD_INOUT,

    KEYWORD_INT,
    KEYWORD_UINT,
    KEYWORD_BITVEC,
    KEYWORD_TYPE,
    KEYWORD_THIS,//+

    KEYWORD_PRIVATE,//+
    KEYWORD_PUBLIC,//+
    KEYWORD_PROTECTED,//+

    KEYWORD_ENUM,//+
    KEYWORD_FLAGS,//+
    KEYWORD_TYPEDEF,
    KEYWORD_NAMESPACE,//+
    KEYWORD_STRUCT,//+
    KEYWORD_CLASS,//+

    KEYWORD_CONSTRUCTOR,//+
    KEYWORD_DESTRUCTOR,//+
    KEYWORD_OPERATOR,//+
    KEYWORD_OPERATOR_MEM,
    KEYWORD_OPERATOR_POST,
    KEYWORD_OPERATOR_PREF,
    KEYWORD_NEW,
    KEYWORD_DELETE,
    KEYWORD_MOVE,
    KEYWORD_COPY,
    KEYWORD_PROPERTY,
    KEYWORD_FUNCTION,
    KEYWORD_RETURN,//+
    KEYWORD_VAR,//+
    KEYWORD_SLOT,
    KEYWORD_SIGNAL,

    KEYWORD_SIZEOF,
    KEYWORD_TYPEOF,
    KEYWORD_OFFSETOF,
    KEYWORD_ALIGNOF,

    KEYWORD_ASSERT,

    KEYWORD_IF,//+
    KEYWORD_ELSE,//+
    
    KEYWORD_FOR,//+
    KEYWORD_WHILE,//+
    KEYWORD_BREAK,//+
    KEYWORD_CONTINUE,//+

    KEYWORD_SWITCH,//+
    KEYWORD_CASE,//+
    KEYWORD_DEFAULT,//+
};
enum {
    COLOR_DEFAULT=0,
    COLOR_BAD,

    COLOR_KEYWORD,
    COLOR_IMMEDIATE,
    COLOR_STRING,

    COLOR_TYPE,
    COLOR_MEMBER,
    COLOR_METHOD,
    COLOR_VIRTMETHOD,
    COLOR_COMMENT,
    COLOR_BRACKET,
    COLOR_INDEX,

    CERROR     =0x400,
    CWARNING   =0x200,
    CFOUND     =0x100,
    UNUSED     =0x080,
};

struct KW { uint len; uint first; const char * text; uint result; };
#define _KW(xxxxx, yyyyy) {strlen(xxxxx), xxxxx[0], xxxxx, yyyyy}

/*
 * do not attempt to store pointers to anywhere except the node it belongs to.
 * there must be exactly one live pointer to string while context is not bound
 * and it must reside in the node. once context is locked, you can make temporary
 * copies of this pointer
 */
struct String : public K::EditUtils::Element {
    typedef quint32 Symbol;
    quint32 string_length;
    Symbol  symbols[0];
    
    void decolorify();
};

inline quint32 sym(String::Symbol s) { return s & 0xffffu; }
inline quint32 symtype(String::Symbol s) { return (s>>16) & 0x1fu; }
inline quint32 symattr(String::Symbol s) { return (s>>21) & 0x7ffu; }
String::Symbol sym(QChar c);
inline String::Symbol sym(String::Symbol s, quint32 attr)
{ return (String::Symbol)((s & 0x001fffff) | (attr<<21)); }

struct Tokenizer {
public:
    Tokenizer();
    ~Tokenizer();

    inline constexpr int tstr(char c1, char c2 = 0, char c3 = 0) {
        int q = c1;
        if (c2) q = (q<<8)|int(c2);
        if (c3) q = (q<<8)|int(c3);
        return q;
    }
    bool tokenize(String * s,
                  int& token_out, int& error,
                  int& start, int& end,
                  QVariant * variant);
private:
    int   last_token = TOKEN_INVALID;
    uint  stoppoint = 0;

};

} //namespace Internal
} //namespace Lang
} //namespace K
