#pragma once

#include "klang_global.h"
#include "str.h"
#include <QVariant>

namespace K {
namespace Lang {
namespace T {
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
} //namespace T

/*
 * Tokenizer which parses strings
 * Tokenizer must be
 */
class K_LANG_EXPORT Tokenizer {
public:
    class K_LANG_EXPORT Context {
    public:
        Context();
        ~Context();
        uint token() const;
        uint detail() const;
        uint start() const;
        uint end() const;
        const QString& error() const;
        void next();
        void setError(const QString& e);
        const QVariant& data() const;
        QString getTokenAsQString(const String * s) const;
    private:
        Q_DISABLE_COPY(Context)
        friend class K::Lang::Tokenizer;
        void * d;
    };

    struct KW { uint len; uint first; const char * text; uint result; };
    #define _KW(xxxxx, yyyyy) {strlen(xxxxx), xxxxx[0], xxxxx, yyyyy}

    Tokenizer(const KW * kws);
    virtual ~Tokenizer();

    inline static constexpr uint tstr(char c1, char c2 = 0, char c3 = 0) {
        uint q = uint(c1);
        if (c2) q = (q<<8)|uint(c2);
        if (c3) q = (q<<8)|uint(c3);
        return q;
    }
    bool tokenize(const String * __restrict__ str, Tokenizer::Context * __restrict__ ctx);
    virtual void error(const char * msg, uint start, uint end);
    virtual void warning(const char * msg, uint start, uint end);
private:
    Q_DISABLE_COPY(Tokenizer)
    const KW * keywords;
};

} //namespace Lang
} //namespace K
