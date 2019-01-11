#include "tokenizer.h"

using namespace K::Lang;
using namespace K::Lang::S;
using namespace K::Lang::T;

namespace {
    struct ContextP {
        uint token_out;
        uint detail;
        uint start;
        uint end;
        bool parseargs;
        QVariant variant;
    };
}
Tokenizer::Context::Context(bool pa) {
    auto p = new ContextP;
    p->token_out = 0;
    p->detail    = 0;
    p->start     = 0;
    p->end       = 0;
    p->parseargs = pa;
    d = p;
}
Tokenizer::Context::~Context() {
    delete (ContextP*)d;
}
void Tokenizer::Context::set_error(uint err) {
    auto p = (ContextP*)d;
    p->token_out = TOKEN_ERROR;
    p->detail = err;
}

uint Tokenizer::Context::token() const
{
    return ((ContextP*)d)->token_out;
}
uint Tokenizer::Context::detail() const
{
    return ((ContextP*)d)->detail;
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
QString Tokenizer::Context::getTokenAsQString(const String * s) const {
    auto p = (ContextP*)d;
    Q_ASSERT(p->end >= p->start);
    Q_ASSERT(p->end < s->string_length);
    QString ret;
    ret.reserve(p->end - p->start);
    for(auto i = p->start; i < p->end; ++i) {
        ret.append(QChar(sym(s->symbols[i])));
    }
    return ret;
}
static bool xstrcmp(const String::Symbol * s, uint n, const char * text, uint &detail);

Tokenizer::Tokenizer(const KW * kws)
    : keywords(kws)
{}
Tokenizer::~Tokenizer()
{}
bool Tokenizer::tokenize(const String * __restrict__ str, Tokenizer::Context * __restrict__ context)
{
    static const char digits[]={"0123456789abcdef"};
    ContextP * ctx = (ContextP*)context->d;
    if (ctx->start >= str->string_length)
        return false;

    uint i    = ctx->start;
    auto s    = sym(str->symbols[i]);
    auto st   = symtype(str->symbols[i]);
    bool minus= false;

    ctx->detail    = 0;

    if (st == SYM_SPACE) {
        for(++i; i < str->string_length; ++i) {
            if (symtype(str->symbols[i]) != st)
                break;
        }
        ctx->start = ctx->end = i;
        if (ctx->start >= str->string_length)
            return false;
        s    = sym(str->symbols[i]);
        st   = symtype(str->symbols[i]);
    }

    switch(st) {
    case SYM_LETTER:{
        for(++i; i < str->string_length; ++i) {
            int s2 = symtype(str->symbols[i]);
            if (s2 != SYM_LETTER && s2 != SYM_DIGIT)
                break;
        }
        ctx->token_out = TOKEN_IDENT;

        uint L = i - ctx->start;
        for(int i = 0; keywords[i].len > 0; ++i) {
            if (keywords[i].first != s)
                continue;
            if (keywords[i].len != L)
                continue;
            if (!xstrcmp(str->symbols+ctx->start, L, keywords[i].text, ctx->detail))
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
                if (i+1<str->string_length && symtype(str->symbols[i+1])==SYM_DIGIT)
                {
                    minus = true;
                    ++i;
                    s    = sym(str->symbols[i]);
                    st   = symtype(str->symbols[i]);
                    goto __digit;
                }
            }
            else if (i+1 < str->string_length) {
                auto c = sym(str->symbols[i+1]);
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
        if (++i < str->string_length) {
            auto c = sym(str->symbols[i]);
            if (c == s) {
                det = (det<<8)|int(c);
                if (++i < str->string_length)
                    c = sym(str->symbols[i]);
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

        if (s == '0' && (str->string_length-i > 1)) {
            //'0x' , '0b', '0o'
            auto c = sym(str->symbols[++i]) | 0x20u;
            if (c == 'x') {
                if (++i == str->string_length)
                    break;
                mul = 16;
            } else if (c == 'o') {
                if (++i == str->string_length)
                    break;
                mul = 8;
            } else if (c == 'b') {
                if (++i == str->string_length)
                    break;
                mul = 2;
            }
        }

        bool     err   = false;
        bool     null  = true; //no digits (like "0x\EOL" or "0b->")
        unsigned c;

        quint64  sum   = 0;
        for(; i < str->string_length; ++i) {
            c = sym(str->symbols[i])|0x20;
            auto indexofc = strchr(digits, int(c));
            if (!indexofc) break;
            null = false;

            uint n = indexofc-digits;
            if (n >= mul) {
                error("Invalid characters in constant", i, i+1);
                err = true;
                break;
            }
            auto sum2 = sum*mul + n;
            if (sum2 < sum) {
                error("Integer contant too large", ctx->start, i+1);
                err = true;
                break;
            }
            sum = sum2;
        }

        if (null) {
            error("Expected integer constant", ctx->start, i+1);
            err = true;
        }

        if (!err) {
            //floating point number
            if (i < str->string_length && c == '.' && mul == 10) {
                //.123
                for(++i; i < str->string_length; ++i) {
                    c = sym(str->symbols[i]);
                    if (c < '0' || c > '9') break;
                }
                if (i < str->string_length) {
                    c = sym(str->symbols[i]);
                    //e+123
                    if ((c | 0x20) == 'e') do {
                        auto j = i+1;
                        if (j >= str->string_length)
                            break;
                        c = sym(str->symbols[j]);
                        if ((c == '+') || (c == '-'))
                        if (++j >= str->string_length)
                            break;
                        c = sym(str->symbols[j]);
                        if ((c < '0') || (c > '9')) break;
                        for(++j; j < str->string_length; ++j) {
                            c = sym(str->symbols[j]);
                            if ((c < '0') || (c > '9')) break;
                        }
                        i = j;
                    } while(0);
                }
                //copy of value
                char tmpbuf[i-ctx->start+0];
                for(uint j = 0; j < i-ctx->start; ++j)
                    tmpbuf[j] = char(str->symbols[ctx->start+j]);
                tmpbuf[i-ctx->start] = 0;

                //f suffix
                errno = 0;
                if (i < str->string_length &&
                    (sym(str->symbols[i])| 0x20) == 'f')
                {
                    ++i;
                    if (ctx->parseargs)
                        ctx->variant = QVariant(strtof(tmpbuf, NULL));
                } else {
                    if (ctx->parseargs)
                        ctx->variant = QVariant(strtod(tmpbuf, NULL));
                }
                if (errno == ERANGE) {
                    error("Floating point constant out of range", ctx->start, i);
                    err = true;
                } else if (errno) {
                    error("Invalid characters in constant", ctx->start, i);
                    err = true;
                }
                ctx->token_out = TOKEN_FLOAT;
            } else {
                //check for l and u suffixes
                bool unsigned_int = false;
                bool long_int     = false;
                if (i < str->string_length) {
                    c = sym(str->symbols[i]);
                    if ((c | 0x20) == 'l') { ++i; long_int = true; }
                }
                if (i < str->string_length) {
                    c = sym(str->symbols[i]);
                    if ((c | 0x20) == 'u') { ++i; unsigned_int = true; }
                }

                quint64 maxint = ~0u;
                if (!long_int) maxint >>= 32;

                if (mul == 10 && !unsigned_int)
                    maxint = (minus ? (maxint+1):(maxint))>>1;

                if (maxint > sum) {
                    error("Integer contant too large", ctx->start, i);
                    err = true;
                }

                if (!err && ctx->parseargs) {
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

        if (!err) {
            while (i < str->string_length) {
                auto st = symtype(str->symbols[i]);
                if (st != SYM_LETTER && st != SYM_DIGIT && st != SYM_DOT)
                    break;

                error("Invalid characters in constant", ctx->start, i);
                err = true;
                ++i;
            }
        }

        if (err)
            ctx->token_out = TOKEN_ERROR;
        break;}
    case SYM_DOT:{
        for(++i; i < str->string_length; ++i) {
            if (sym(str->symbols[i]) != '.')
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
        for(++i; i < str->string_length; ++i) {
            if (sym(str->symbols[i]) != '=')
                break;
        }
        switch (i - ctx->start) {
        case 1:
            ctx->token_out = TOKEN_ASSIGN;
            break;
        case 2:
            ctx->token_out = TOKEN_OPERATOR;
            ctx->detail    = '=';
            break;
        default:
            ctx->token_out = TOKEN_ERROR;
            error("invalid operator", ctx->start, i);
        }
        break;}
    case SYM_SEMICOLON:{
        for(++i; i < str->string_length; ++i) {
            if (sym(str->symbols[i]) != ':')
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
        if (ctx->parseargs) {
            QString sout;
            for(++i; i < str->string_length; ++i) {
                auto c = sym(str->symbols[i]);
                if (slash) {
                    switch(c) {
                    case 'n': sout.append(QChar('\n'));break;
                    case 'r': sout.append(QChar('\r'));break;
                    case 't': sout.append(QChar('\t'));break;
                    case 'v': sout.append(QChar('\v'));break;
                    case 'b': sout.append(QChar('\b'));break;
                    case 'x': {
                        //2 hex sumbols: \x3F
                        if ((i+2) < str->string_length) {
                            int c0 = sym(str->symbols[i+1]) | 0x20;
                            int c1 = sym(str->symbols[i+2]) | 0x20;
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
                        if ((i+4) < str->string_length) {
                            int c0 = sym(str->symbols[i+1]) | 0x20;
                            int c1 = sym(str->symbols[i+2]) | 0x20;
                            int c2 = sym(str->symbols[i+2]) | 0x20;
                            int c3 = sym(str->symbols[i+3]) | 0x20;
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
        } else {
            for(++i; i < str->string_length; ++i) {
                auto c = sym(str->symbols[i]);
                if (slash) {slash = false; continue; }
                if (c == s) {
                    ++i;
                    fin = true;
                    break;
                }
                slash = (c == '\\');
            }
        }
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
static bool xstrcmp(const String::Symbol *s, uint n, const char * text, uint& detail) {
    uint i = 1;
    for (;i < n;++i)
    {
        if (sym(s[i])!=uint(text[i]))
            return false;
    }
    if (text[i] == '?') {
        if (i == n) return false;
        if (symtype(s[i])!=SYM_DIGIT) return false;

        do {
            detail = detail*10+(sym(s[i])-'0');
            ++i;
            if (detail > 1024*1024)
                return false;
        } while(i < n && symtype(s[i])==SYM_DIGIT);
        return i==n;
    } else
        return text[i] == 0;
}
