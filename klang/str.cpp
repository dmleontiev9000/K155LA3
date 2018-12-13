#include "str.h"
#include "syntaxerrors.h"
#include <string.h>

using namespace K::Lang::Internal;

enum {
    BAD = 0,
    SPACE,LETTER,DIGIT,
    DOT,COMMA,SEMICOLON,EQU_SIGN,
    PUNCT,
    LBRACKET,RBRACKET,LINDEX,RINDEX,STR1,STR2,MAXTOK
};
Q_STATIC_ASSERT(MAXTOK < 32);

static bool xstrcmp(String::Symbol * s, uint n, const char * text, int& detail);
static KW keywords[] = {
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
    _KW("prefix", KEYWORD_OPERATOR_PREF),
    _KW("postfix", KEYWORD_OPERATOR_POST),
    _KW("member", KEYWORD_OPERATOR_MEM),
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


void String::decolorify() {
    for(uint i = 0; i < string_length; ++i)
        symbols[i] &= 0x1fffffu;
}

bool Tokenizer::tokenize(String *text,
                         int& token_out, int& detail,
                         int& start, int& end,
                         QVariant * variant)
{
    static const char digits[]={"0123456789abcdef"};

    if (stoppoint >= text->string_length) 
        return false;

    uint i    = stoppoint;
    auto s    = sym(text->symbols[i]);
    auto st   = symtype(text->symbols[i]);
    bool minus= false;

    detail    = 0;

    if (st == SPACE) {
        for(++i; i < text->string_length; ++i) {
            if (symtype(text->symbols[i]) != st)
                break;
        }
        stoppoint = i;
        if (stoppoint >= text->string_length) 
            return false;
        s    = sym(text->symbols[i]);
        st   = symtype(text->symbols[i]);
    }
    
    switch(st) {
    case LETTER:{
        for(++i; i < text->string_length; ++i) {
            int s2 = symtype(text->symbols[i]);
            if (s2 != LETTER && s2 != DIGIT)
                break;
        }
        token_out = TOKEN_IDENT;

        uint L = i-start;
        for(int i = 0; keywords[i].len > 0; ++i) {
            if (keywords[i].first != s)
                continue;
            if (keywords[i].len != L)
                continue;
            if (!xstrcmp(text->symbols+start, L, keywords[i].text, detail))
                continue;

            token_out = keywords[i].result;
            break;
        }
        break;}
    case PUNCT:{
        static const char symset[]={"+-~*&%|^<>!@/"};
        //1 or 2 symbols from symset and optional =
        //or ->
        if (s == '-')
        {
            if (last_token == TOKEN_ASSIGN ||
                last_token == TOKEN_COMMA  ||
                last_token == TOKEN_LBR    ||
                last_token == TOKEN_LIDX   ||
                last_token == TOKEN_DOT3   ||
                last_token == TOKEN_INVALID||
                last_token == TOKEN_OPERATOR)
            {
                if (i+1<text->string_length && symtype(text->symbols[i+1])==DIGIT)
                {
                    minus = true;
                    ++i;
                    s    = sym(text->symbols[i]);
                    st   = symtype(text->symbols[i]);
                    goto __digit;
                }
            }
        }

        if (s > 127 || !strchr(symset, int(s))) {
            ++i;
            token_out = TOKEN_INVALID;
            break;
        }

        int det=s;
        if (++i < text->string_length) {
            auto c = sym(text->symbols[i]);
            if (c == s) {
                det = (det<<8)|int(c);
                if (++i < text->string_length)
                    c = sym(text->symbols[i]);
            }
            if (c == '=') {
                det = (det<<8)|int(c);
                ++i;
            }
        }
        detail    = det;
        token_out = TOKEN_OPERATOR;
        break;}
    case DIGIT:__digit:{
        unsigned mul = 10;

        if (s == '0' && (text->string_length-i > 1)) {
            //'0x' , '0b', '0o'
            auto c = sym(text->symbols[++i]) | 0x20u;
            if (c == 'x') {
                if (++i == text->string_length)
                    break;
                mul = 16;
            } else if (c == 'o') {
                if (++i == text->string_length)
                    break;
                mul = 8;
            } else if (c == 'b') {
                if (++i == text->string_length)
                    break;
                mul = 2;
            }
        }

        bool     null  = true; //no digits (like "0x\EOL" or "0b->")
        unsigned c;
    
        quint64  sum   = 0;
        for(; i < text->string_length; ++i) {
            c = sym(text->symbols[i])|0x20;
            auto indexofc = strchr(digits, int(c));
            if (!indexofc) break;
            null = false;
    
            uint n = indexofc-digits;
            if (n >= mul) {
                detail = SyntaxErrors::invalid_characters_in_constant;
                break;
            }
            auto sum2 = sum*mul + n;
            if (sum2 < sum) {
                detail = SyntaxErrors::integer_contant_is_too_large;
                break;
            }
            sum = sum2;
        }
    
        if (null) detail = SyntaxErrors::expected_integer_constant;
    
        if (!detail) {
            //floating point number
            if (i < text->string_length && c == '.' && mul == 10) {
                //.123
                for(++i; i < text->string_length; ++i) {
                    c = sym(text->symbols[i]);
                    if (c < '0' || c > '9') break;
                }
                if (i < text->string_length) {
                    c = sym(text->symbols[i]);
                    //e+123
                    if ((c | 0x20) == 'e') do {
                        auto j = i+1;
                        if (j >= text->string_length)
                            break;
                        c = sym(text->symbols[j]);
                        if ((c == '+') || (c == '-'))
                        if (++j >= text->string_length)
                            break;
                        c = sym(text->symbols[j]);
                        if ((c < '0') || (c > '9')) break;
                        for(++j; j < text->string_length; ++j) {
                            c = sym(text->symbols[j]);
                            if ((c < '0') || (c > '9')) break;
                        }
                        i = j;
                    } while(0);
                }
                //copy of value
                char tmpbuf[i-stoppoint+0];
                for(uint j = 0; j < i-stoppoint; ++j)
                    tmpbuf[j] = char(text->symbols[stoppoint+j]);
                tmpbuf[i-stoppoint] = 0;
    
                //f suffix
                errno = 0;
                if (i < text->string_length &&
                    (sym(text->symbols[i])| 0x20) == 'f')
                {
                    ++i;
                    if (Q_UNLIKELY (variant))
                        *variant = QVariant(strtof(tmpbuf, NULL));
                } else {
                    if (Q_UNLIKELY (variant))
                        *variant = QVariant(strtod(tmpbuf, NULL));
                }
                if (errno == ERANGE)
                    detail = SyntaxErrors::floating_point_constant_out_of_range;
                else if (errno)
                    detail = SyntaxErrors::invalid_characters_in_constant;
                token_out = TOKEN_FLOAT;
            } else {
                //check for l and u suffixes
                bool unsigned_int = false;
                bool long_int     = false;
                if (i < text->string_length) {
                    c = sym(text->symbols[i]);
                    if ((c | 0x20) == 'l') { ++i; long_int = true; }
                }
                if (i < text->string_length) {
                    c = sym(text->symbols[i]);
                    if ((c | 0x20) == 'u') { ++i; unsigned_int = true; }
                }
    
                quint64 maxint = ~0u;
                if (!long_int) maxint >>= 32;
    
                if (mul == 10 && !unsigned_int)
                    maxint = (minus ? (maxint+1):(maxint))>>1;
    
                if (maxint > sum)
                    detail = SyntaxErrors::integer_contant_is_too_large;
    
                if (Q_UNLIKELY (variant)) {
                    if (long_int) {
                        if (unsigned_int)
                            *variant = QVariant(qulonglong(sum));
                        else
                            *variant = QVariant(qlonglong(sum));
                    } else {
                        if (unsigned_int)
                            *variant = QVariant(uint(sum));
                        else
                            *variant = QVariant(int(sum));
                    }
                }
                token_out = TOKEN_INT;
            }
        }
    
        while (i < text->string_length) {
            auto st = symtype(text->symbols[i]);
            if (st != LETTER && st != DIGIT && st != DOT)
                break;
    
            detail = SyntaxErrors::invalid_characters_in_constant;
            ++i;
        }
    
        if (detail)
            token_out = TOKEN_ERROR;
        break;}
    case DOT:{
        for(++i; i < text->string_length; ++i) {
            if (sym(text->symbols[i]) != '.')
                break;
        }
        switch (i - stoppoint) {
        case 1:
            token_out = TOKEN_DOT;
            break;
        case 3:
            token_out = TOKEN_DOT3;
            break;
        default:
            token_out = TOKEN_ERROR;
            detail = SyntaxErrors::invalid_operator;
        }
        break;}
    case EQU_SIGN:{
        for(++i; i < text->string_length; ++i) {
            if (sym(text->symbols[i]) != '=')
                break;
        }
        switch (i - stoppoint) {
        case 1:
            token_out = TOKEN_ASSIGN;
            break;
        case 2:
            token_out = TOKEN_OPERATOR;
            detail    = '=';
            break;
        default:
            token_out = TOKEN_ERROR;
            detail = SyntaxErrors::invalid_operator;
        }
        break;}
    case SEMICOLON:{
        for(++i; i < text->string_length; ++i) {
            if (sym(text->symbols[i]) != ':')
                break;
        }
        switch (i - stoppoint) {
        case 1:
            token_out = TOKEN_SC;
            break;
        case 2:
            token_out = TOKEN_SC2;
            break;
        default:
            token_out = TOKEN_ERROR;
            detail = SyntaxErrors::invalid_operator;
        }
        break;}
    case LBRACKET:{
        token_out = TOKEN_LBR; ++i;
        break;}
    case RBRACKET:{
        token_out = TOKEN_RBR; ++i;
        break;}
    case LINDEX:{
        token_out = TOKEN_LIDX; ++i;
        break;}
    case RINDEX:{
        token_out = TOKEN_RIDX; ++i;
        break;}
    case STR1: case STR2: {
        bool slash = false, fin = false;
        if (variant) {
            QString str;
            for(++i; i < text->string_length; ++i) {
                auto c = sym(text->symbols[i]);
                if (slash) {
                    switch(c) {
                    case 'n': str.append(QChar('\n'));break;
                    case 'r': str.append(QChar('\r'));break;
                    case 't': str.append(QChar('\t'));break;
                    case 'v': str.append(QChar('\v'));break;
                    case 'b': str.append(QChar('\b'));break;
                    case 'x': {
                        //2 hex sumbols: \x3F
                        if ((i+2) < text->string_length) {
                            int c0 = sym(text->symbols[i+1]) | 0x20;
                            int c1 = sym(text->symbols[i+2]) | 0x20;
                            auto pc0 = strchr(digits, c0);
                            auto pc1 = strchr(digits, c1);
                            if (!pc0 || !pc1) {
                                str.append(QChar('x'));break;
                            }
                            int code = (pc0-digits)*16 + (pc1-digits);
                            str.append(QChar(code));
                            i+=2;
                        }
                        break;}
                    case 'u':{
                        //4 hex symbols: \u2501
                        if ((i+4) < text->string_length) {
                            int c0 = sym(text->symbols[i+1]) | 0x20;
                            int c1 = sym(text->symbols[i+2]) | 0x20;
                            int c2 = sym(text->symbols[i+2]) | 0x20;
                            int c3 = sym(text->symbols[i+3]) | 0x20;
                            auto pc0 = strchr(digits, c0);
                            auto pc1 = strchr(digits, c1);
                            auto pc2 = strchr(digits, c2);
                            auto pc3 = strchr(digits, c3);
                            if (!pc0 || !pc1 || !pc2 || !pc3) {
                                str.append(QChar('u'));break;
                            }
                            int code = (pc0-digits)*4096 + (pc1-digits)*256 + (pc2-digits)*16 + (pc3-digits);
                            str.append(QChar(code));
                            i+=4;
                        }
                        break;}
                    default:
                        str.append(QChar(c));break;
                    }
                    slash = false;
                } else if (c == '\\') {
                    slash = true;
                } else if (c == s){
                    ++i;
                    fin = true;
                    break;
                } else {
                    str.append(QChar(c));
                }
            }
            *variant = QVariant(str);
        } else {
            for(++i; i < text->string_length; ++i) {
                auto c = sym(text->symbols[i]);
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
            token_out = TOKEN_ERROR;
            detail = SyntaxErrors::string_not_terminated;
        } else {
            token_out = (st == STR2 ? TOKEN_STR2:TOKEN_STR1);
        }
        break;}
    default:
        token_out = TOKEN_ERROR;
        detail = SyntaxErrors::invalid_symbol;
        ++i;
    }
    stoppoint = i;
    end = i;
    last_token = token_out;
    return true;
}

String::Symbol sym(QChar c) {
    String::Symbol s = c.unicode();
    if (c.isDigit()) {
        s |= (DIGIT<<16);
    } else if (c.isSpace()) {
        s |= (SPACE<<16);
    } else if (c.isLetter()) {
        s |= (LETTER<<16);
    } else switch (s) {
    case '.': s |= (DOT<<16);       break;
    case ',': s |= (COMMA<<16);     break;
    case '=': s |= (EQU_SIGN<<16);  break;
    case '\'':s |= (STR1<<16);      break;
    case '\"':s |= (STR2<<16);      break;
    case '$':
    case '_': s |= (LETTER<<16);    break;
    case '(': s |= (LBRACKET<<16);  break;
    case ')': s |= (RBRACKET<<16);  break;
    case '[': s |= (LINDEX<<16);    break;
    case ']': s |= (RINDEX<<16);    break;
    default:  s = 0;
    }
    return s;
}
static bool xstrcmp(String::Symbol * s, uint n, const char * text, int& detail) {
    uint i = 1;
    for (;i < n;++i)
    {
        if (sym(s[i])!=uint(text[i]))
            return false;
    }
    if (text[i] == '?') {
        if (i == n) return false;
        if (symtype(s[i])!=DIGIT) return false;

        do {
            detail = detail*10+(sym(s[i])-'0');
            ++i;
            if (detail > 1024*1024)
                return false;
        } while(i < n && symtype(s[i])==DIGIT);
        return i==n;
    } else
        return text[i] == 0;
}
