#pragma once

#include "klang_global.h"
#include "str.h"
#include <QVariant>

namespace K {
namespace Lang {
/*
 * Tokenizer which parses strings
 * Tokenizer must be
 */
struct TokenizerContext {
    uint token_out;
    uint detail;
    uint start;
    uint end;
    bool parseargs;
    bool have_token;
    QVariant variant;
    void init() {
        token_out = 0;//TOKEN_INVALID
        detail    = 0;
        start     = 0 ;
        end       = 0;
        parseargs = false;
        have_token= false;
        variant.clear();
    }
    void next() {
        start = end;
        token_out = 0;
        detail    = 0;
        have_token= false;
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

    inline static constexpr int tstr(char c1, char c2 = 0, char c3 = 0) {
        int q = c1;
        if (c2) q = (q<<8)|int(c2);
        if (c3) q = (q<<8)|int(c3);
        return q;
    }
    bool tokenize(String * __restrict__ str, TokenizerContext * __restrict__ ctx);
private:
    const KW * keywords;
};

} //namespace Lang
} //namespace K
