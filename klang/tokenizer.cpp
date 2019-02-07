#include "tokenizer.h"
#include <cmath>
#include <cstdint>
#include <climits>
#include <cfloat>

using namespace K::Lang;
using namespace K::Lang::S;
using namespace K::Lang::T;

namespace {
    struct ContextP {
        uint token_out;
        uint detail;
        uint start;
        uint end;
        QVariant variant;
        QString error;
    };
}
Tokenizer::Context::Context() {
    auto p = new ContextP;
    p->token_out = 0;
    p->detail    = 0;
    p->start     = 0;
    p->end       = 0;
    d = p;
}
Tokenizer::Context::~Context() {
    delete (ContextP*)d;
}
uint Tokenizer::Context::token() const
{
    return ((ContextP*)d)->token_out;
}
uint Tokenizer::Context::detail() const
{
    return ((ContextP*)d)->detail;
}
const QString& Tokenizer::Context::error() const
{
    return ((ContextP*)d)->error;

}
void Tokenizer::Context::setError(const QString &e) {
    auto p = (ContextP*)d;
    p->error = e;
}
uint Tokenizer::Context::start() const
{
    return ((ContextP*)d)->start;
}
uint Tokenizer::Context::end() const
{
    return ((ContextP*)d)->end;
}
void Tokenizer::Context::next()
{
    auto p = (ContextP*)d;
    p->start = p->end;
}
const QVariant& Tokenizer::Context::data() const
{
    return ((ContextP*)d)->variant;
}
static bool xstrcmp(const String *s, uint at, uint n, const char * text, uint& detail);

Tokenizer::Tokenizer(const QVector<KW> &kws)
    : keywords(kws)
{}
Tokenizer::~Tokenizer()
{}
uint Tokenizer::firstToken(const String *str)
{
    uint i     = 0;
    for(; i < str->length(); ++i) {
        if (symtype(str->at(i)) != SYM_SPACE)
            break;
    }
    if (i == str->length())
        return TOKEN_INVALID;

    auto s    = sym(str->at(i));
    auto st   = symtype(str->at(i));
    if (st == SYM_COMMENT)
        return TOKEN_COMMENT;

    if (st != SYM_LETTER)
        return TOKEN_INVALID;

    uint start = i;
    for(++i; i < str->length(); ++i) {
        uint s2 = symtype(str->at(i));
        if (s2 != SYM_LETTER && s2 != SYM_DIGIT)
            break;
    }
    uint L = i - start, detail;
    for(int i = 0; keywords[i].len > 0; ++i) {
        if (keywords[i].first != s)
            continue;
        if (xstrcmp(str, start, L, keywords[i].text, detail))
            return keywords[i].result;
    }
    return TOKEN_INVALID;
}
bool Tokenizer::tokenize(const String * __restrict__ str, Tokenizer::Context * __restrict__ context)
{
    static const char digits[]={"0123456789abcdef"};
    ContextP * ctx = (ContextP*)context->d;
    if (ctx->start >= str->length())
        return false;

    uint i    = ctx->start;
    auto s    = sym(str->at(i));
    auto st   = symtype(str->at(i));
    bool minus= false;

    ctx->detail    = 0;

    if (st == SYM_SPACE) {
        for(++i; i < str->length(); ++i) {
            if (symtype(str->at(i)) != st)
                break;
        }
        ctx->start = ctx->end = i;
        if (ctx->start >= str->length())
            return false;
        s    = sym(str->at(i));
        st   = symtype(str->at(i));
    }

    switch(st) {
    case SYM_COMMENT:
        ctx->token_out = TOKEN_COMMENT;
        ++i;
        break;
    case SYM_LETTER:{
        for(++i; i < str->length(); ++i) {
            uint s2 = symtype(str->at(i));
            if (s2 != SYM_LETTER && s2 != SYM_DIGIT)
                break;
        }
        ctx->token_out = TOKEN_IDENT;

        uint L = i - ctx->start;
        for(int i = 0; keywords[i].len > 0; ++i) {
            if (keywords[i].first != s)
                continue;
            if (!xstrcmp(str, ctx->start, L, keywords[i].text, ctx->detail))
                continue;

            ctx->token_out = keywords[i].result;
            break;
        }
        break;}
    case SYM_PUNCT:{
        static const char symset[]={"+-~*&%|^<>!@/"};
        //1 or 2 symbols from symset and optional =
        //or ->
        if (s == '-')
        {
            if (ctx->token_out == TOKEN_ASSIGN ||
                ctx->token_out == TOKEN_COMMA  ||
                ctx->token_out == TOKEN_LBR    ||
                ctx->token_out == TOKEN_LIDX   ||
                ctx->token_out == TOKEN_DOT3   ||
                ctx->token_out == TOKEN_INVALID||
                ctx->token_out == TOKEN_OPERATOR)
            {
                if (i+1<str->length() && symtype(str->at(i+1))==SYM_DIGIT)
                {
                    minus = true;
                    ++i;
                    s    = sym(str->at(i));
                    st   = symtype(str->at(i));
                    goto __digit;
                }
            }
            else if (i+1 < str->length()) {
                auto c = sym(str->at(i+1));
                if (c=='>') {
                    ctx->token_out = TOKEN_OPERATOR;
                    ctx->detail = tstr('-','>');
                    i+=2;
                    break;
                }
            }
        }

        if (s > 127 || !strchr(symset, int(s))) {
            ++i;
            ctx->token_out = TOKEN_INVALID;
            break;
        }

        int det=s;
        if (++i < str->length()) {
            auto c = sym(str->at(i));
            if (c == s) {
                det = (det<<8)|int(c);
                if (++i < str->length())
                    c = sym(str->at(i));
            }
            if (c == '=') {
                det = (det<<8)|int(c);
                ++i;
            }
        }
        ctx->detail    = det;
        ctx->token_out = TOKEN_OPERATOR;
        break;}
    case SYM_DIGIT:__digit:{
        unsigned mul = 10;

        if (s == '0' && (str->length()-i > 1)) {
            //'0x' , '0b', '0o'
            auto c = sym(str->at(i+1)) | 0x20u;
            if (c == 'x') {
                if (++i == str->length())
                    break;
                mul = 16;
            } else if (c == 'o') {
                if (++i == str->length())
                    break;
                mul = 8;
            } else if (c == 'b') {
                if (++i == str->length())
                    break;
                mul = 2;
            }
        }

        bool     iow   = false;
        bool     err   = false;
        bool     null  = true; //no digits (like "0x\EOL" or "0b->")
        unsigned c;

        quint64  sum   = 0;
        for(; i < str->length(); ++i) {
            c = sym(str->at(i))|0x20;
            auto indexofc = strchr(digits, int(c));
            if (!indexofc) break;
            null = false;

            uint n = indexofc-digits;
            //some characters
            if (n >= mul) {
                error("Invalid characters in constant", i, i+1);
                err = true;
                break;
            }
            auto sum2 = sum*mul + n;
            if (sum2 < sum) iow = true;
            sum = sum2;
        }

        if (null) {
            error("Expected integer constant", ctx->start, i+1);
            err = true;
        }

        if (!err) {
            //floating point number
            if (i < str->length() && c == '.' && mul == 10) {
                //we can't use strtod or strtof because they use locale and thus suck
                auto dot   = i;
                auto start = ctx->start + (minus?1:0);

                //.123
                for(++i; i < str->length(); ++i) {
                    c = sym(str->at(i));
                    if (c < '0' || c > '9') break;
                }

                double result = 0.0;
                for(auto j = i-1; j > dot; --j) {
                    result = 0.1*result + 0.1*(char(str->at(j))-'0');
                }
                double scale = 1.0;
                for(auto j = dot-1;; --j)
                {
                    result = result + scale*(char(str->at(j))-'0');
                    scale *= 10.0;
                    if (j == start) break;
                }
                if (minus) result = -result;

                int sexp = 0;
                bool oor = false;
                if (i < str->length()) {
                    c = sym(str->at(i));
                    //e+123
                    if (tolower(c) == 'e') do {
                        err = true;

                        if (++i >= str->length())
                            break;
                        auto cs = sym(str->at(i));
                        if ((cs == '+') || (cs == '-'))
                            if (++i >= str->length())
                                break;
                        c = sym(str->at(i));
                        if ((c < '0') || (c > '9'))
                            break;

                        uint exp = c - '0';
                        err = false;

                        for(++i; i < str->length(); ++i) {
                            c = sym(str->at(i));
                            if ((c < '0') || (c > '9'))
                                break;
                            exp = exp*10+(c-'0');
                            if (exp > std::numeric_limits<int>::max())
                                oor = true;
                        }
                        sexp = (cs=='-') ? -int(exp) : int(exp);
                    } while(0);
                }

                if (i < str->length() &&
                    sym(tolower(sym(str->at(i))) == 'f'))
                {
                    ++i;
                    oor |= (sexp < -FLT_MAX_10_EXP) | (sexp > FLT_MAX_10_EXP);
                    if (!err && !oor) {
                        oor |= result > FLT_MAX;
                        if (sexp) result *= pow(10, sexp);

                        ctx->variant = QVariant(float(result));
                    }
                } else {
                    oor |= (sexp < -DBL_MAX_10_EXP) | (sexp > DBL_MAX_10_EXP);
                    if (!err && !oor) {
                        if (sexp) result *= pow(10, sexp);
                        oor |= result > DBL_MAX;
                        ctx->variant = QVariant(result);
                    }
                }
                if (oor) {
                    error("Floating point constant out of range", ctx->start, i);
                    err = true;
                } else if (err) {
                    error("Invalid floating point constant", ctx->start, i);
                }
                ctx->token_out = TOKEN_FLOAT;
            } else {
                //check for l and u suffixes
                bool unsigned_int = false;
                bool long_int     = false;
                if (i < str->length()) {
                    c = tolower(sym(str->at(i)));
                    if (c == 'l') { ++i; long_int = true; }
                }
                if (i < str->length()) {
                    c = tolower(sym(str->at(i)));
                    if (c  == 'u') { ++i; unsigned_int = true; }
                }
                if (iow) {
                    error("Integer contant too large", ctx->start, i+1);
                    err = true;
                    break;
                }

                quint64 maxint = ~0u;
                if (!long_int) maxint >>= 32;

                if (mul == 10 && !unsigned_int)
                    maxint = (minus ? (maxint+1):(maxint))>>1;

                if (maxint > sum) {
                    error("Integer contant too large", ctx->start, i);
                    err = true;
                }

                if (!err) {
                    if (long_int) {
                        if (unsigned_int)
                            ctx->variant = QVariant(qulonglong(sum));
                        else
                            ctx->variant = QVariant(qlonglong(sum));
                    } else {
                        if (unsigned_int)
                            ctx->variant = QVariant(uint(sum));
                        else
                            ctx->variant = QVariant(int(sum));
                    }
                }
                ctx->token_out = TOKEN_INT;
            }
        }

        while (i < str->length()) {
            auto st = symtype(str->at(i));
            if (st != SYM_LETTER && st != SYM_DIGIT && st != SYM_DOT)
                break;

            error("Invalid characters in constant", ctx->start, i);
            err = true;
            ++i;
        }

        if (err)
            ctx->token_out = TOKEN_ERROR;
        break;}
    case SYM_COMMA: {
        ctx->token_out = TOKEN_COMMA; ++i;
        break;}
    case SYM_DOT:{
        for(++i; i < str->length(); ++i) {
            if (sym(str->at(i)) != '.')
                break;
        }
        switch (i - ctx->start) {
        case 1:
            ctx->token_out = TOKEN_DOT;
            ctx->detail = tstr('.');
            break;
        case 3:
            ctx->token_out = TOKEN_DOT3;
            ctx->detail = tstr('.','.','.');
            break;
        default:
            ctx->token_out = TOKEN_ERROR;
            error("invalid operator", ctx->start, i);
        }
        break;}
    case SYM_EQUAL_SIGN:{
        for(++i; i < str->length(); ++i) {
            if (sym(str->at(i)) != '=')
                break;
        }
        switch (i - ctx->start) {
        case 1:
            ctx->token_out = TOKEN_ASSIGN;
            break;
        case 2:
            ctx->token_out = TOKEN_OPERATOR;
            ctx->detail    = tstr('=');
            break;
        default:
            ctx->token_out = TOKEN_ERROR;
            error("invalid operator", ctx->start, i);
        }
        break;}
    case SYM_SEMICOLON:{
        for(++i; i < str->length(); ++i) {
            if (sym(str->at(i)) != ':')
                break;
        }
        switch (i - ctx->start) {
        case 1:
            ctx->token_out = TOKEN_SC;
            break;
        case 2:
            ctx->token_out = TOKEN_SC2;
            break;
        default:
            ctx->token_out = TOKEN_ERROR;
            error("invalid operator", ctx->start, i);
        }
        break;}
    case SYM_LBRACKET:{
        ctx->token_out = TOKEN_LBR; ++i;
        break;}
    case SYM_RBRACKET:{
        ctx->token_out = TOKEN_RBR; ++i;
        break;}
    case SYM_LINDEX:{
        ctx->token_out = TOKEN_LIDX; ++i;
        break;}
    case SYM_RINDEX:{
        ctx->token_out = TOKEN_RIDX; ++i;
        break;}
    case SYM_STR1: case SYM_STR2: {
        bool slash = false, fin = false;
        QString sout;
        for(++i; i < str->length(); ++i) {
            auto c = sym(str->at(i));
            if (slash) {
                switch(c) {
                case 'n': sout.append(QChar('\n'));break;
                case 'r': sout.append(QChar('\r'));break;
                case 't': sout.append(QChar('\t'));break;
                case 'v': sout.append(QChar('\v'));break;
                case 'b': sout.append(QChar('\b'));break;
                case 'x': {
                    //2 hex sumbols: \x3F
                    if ((i+2) < str->length()) {
                        int c0 = sym(str->at(i+1)) | 0x20;
                        int c1 = sym(str->at(i+2)) | 0x20;
                        auto pc0 = strchr(digits, c0);
                        auto pc1 = strchr(digits, c1);
                        if (!pc0 || !pc1) {
                            sout.append(QChar('x'));break;
                        }
                        int code = (pc0-digits)*16 + (pc1-digits);
                        sout.append(QChar(code));
                        i+=2;
                    }
                    break;}
                case 'u':{
                    //4 hex symbols: \u2501
                    if ((i+4) < str->length()) {
                        int c0 = sym(str->at(i+1)) | 0x20;
                        int c1 = sym(str->at(i+2)) | 0x20;
                        int c2 = sym(str->at(i+3)) | 0x20;
                        int c3 = sym(str->at(i+4)) | 0x20;
                        auto pc0 = strchr(digits, c0);
                        auto pc1 = strchr(digits, c1);
                        auto pc2 = strchr(digits, c2);
                        auto pc3 = strchr(digits, c3);
                        if (!pc0 || !pc1 || !pc2 || !pc3) {
                            sout.append(QChar('u'));break;
                        }
                        int code = (pc0-digits)*4096 + (pc1-digits)*256 + (pc2-digits)*16 + (pc3-digits);
                        sout.append(QChar(code));
                        i+=4;
                    }
                    break;}
                default:
                    warning("unknown escape sequence", i-1, i+1);
                    sout.append(QChar(c));break;
                }
                slash = false;
            } else if (c == '\\') {
                slash = true;
            } else if (c == s){
                ++i;
                fin = true;
                break;
            } else {
                sout.append(QChar(c));
            }
        }
        ctx->variant = QVariant(sout);
        if (!fin) {
            ctx->token_out = TOKEN_ERROR;
            error("String not terminated", ctx->start, i);
        } else {
            ctx->token_out = (st == SYM_STR2 ? TOKEN_STR2:TOKEN_STR1);
        }
        break;}
    default:
        ctx->token_out = TOKEN_ERROR;
        error("invalid symbol", ctx->start, ctx->start+1);
        ++i;
    }
    ctx->end = i;
    return true;
}
static bool xstrcmp(const String *s, uint at, uint n, const char * text, uint& detail) {
    uint i = 1;
    for (;i < n;++i)
    {
        if (text[i] == '?')
            break;
        if (sym(s->at(i+at))!=uint(text[i]))
            return false;
    }
    if (text[i] == '?') {
        if (i == n) return false;
        if (symtype(s->at(i+at))!=SYM_DIGIT) return false;

        do {
            detail = detail*10+(sym(s->at(i))-'0');
            ++i;
            if (detail > 1024*1024)
                return false;
        } while(i < n && symtype(s->at(i+at))==SYM_DIGIT);
        return i==n;
    } else
        return text[i] == 0;
}
void Tokenizer::error(const char * , uint , uint ) {
//    qDebug("ERROR: %s", msg);
}
void Tokenizer::warning(const char * , uint , uint ) {
//    qDebug("WARINIG: %s", msg);
}
