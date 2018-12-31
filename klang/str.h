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
    
    void colorify(uint from, uint to, uint attr);
    void decolorify();
};
inline quint32 sym(String::Symbol s) { return s & 0xffffu; }
inline quint32 symtype(String::Symbol s) { return (s>>16) & 0x1fu; }
inline quint32 symattr(String::Symbol s) { return (s>>21) & 0x7ffu; }
String::Symbol sym(QChar c);
inline String::Symbol sym(String::Symbol s, quint32 attr)
{ return (String::Symbol)((s & 0x001fffff) | (attr<<21)); }
/*
 * Tokenizer which parses strings
 * Tokenizer must be
 */
struct TokenizerContext {
    String * str;
    uint token_out;
    uint detail;
    uint start;
    uint end;
    bool parseargs;
    QVariant variant;
    void init() {
        token_out = 0;//TOKEN_INVALID
        detail    = 0;
        start     = 0 ;
        end       = 0;
        parseargs = false;
        variant.clear();
    }
};
struct Tokenizer {
public:
    enum Tokens {
        TOKEN_INVALID = 0,
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
        TOKEN_MAX,
    };
    struct KW { uint len; uint first; const char * text; uint result; };
    #define _KW(xxxxx, yyyyy) {strlen(xxxxx), xxxxx[0], xxxxx, yyyyy}

    Tokenizer(const KW * kws);
    ~Tokenizer();

    inline constexpr int tstr(char c1, char c2 = 0, char c3 = 0) {
        int q = c1;
        if (c2) q = (q<<8)|int(c2);
        if (c3) q = (q<<8)|int(c3);
        return q;
    }
    bool tokenize(TokenizerContext * __restrict__ ctx);
private:
    const KW * keywords;
};

} //namespace Internal
} //namespace Lang
} //namespace K
