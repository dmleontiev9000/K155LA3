#include "str.h"
#include <string.h>

using namespace K::Lang;
using namespace K::Lang::S;
String * String::alloc(K::EditUtils::AssetPool * pool, const QString& text) {
    uint len = text.length();
    String * out = (String*)pool->alloc(sizeof(String)+sizeof(Symbol)*len);
    out->string_length = len;
    out->misc = 0;
    for(uint i = 0; i < len; ++i) {
        out->symbols[i] = S::sym(text[i]);
        out->misc = (out->misc<<11) ^ (out->misc>>21) ^ (out->symbols[i]&0xFFFF);
    }
    if (len) out->misc = (out->misc<<HKEY_LEN) | (text[0].unicode() & HKEY_MASK);
    return out;
}
String * String::alloc(K::EditUtils::AssetPool *pool, const QLatin1String &text) {
    uint len = text.size();
    String * out = (String*)pool->alloc(sizeof(String)+sizeof(Symbol)*len);
    out->string_length = len;
    out->misc = 0;
    for(uint i = 0; i < len; ++i) {
        out->symbols[i] = S::sym(text[i]);
        out->misc = (out->misc<<11) ^ (out->misc>>21) ^ (out->symbols[i]&0xFFFF);
    }
    if (len) out->misc = (out->misc<<HKEY_LEN) | (text[0].unicode() & HKEY_MASK);
    return out;
}
String * String::alloc(K::EditUtils::AssetPool * pool, uint len) {
    String * out = (String*)pool->alloc(sizeof(String)+sizeof(Symbol)*len);
    out->string_length = len;
    out->misc = 0;
    return out;
}
String * String::alloc(K::EditUtils::AssetPool * pool, const String * other,
                       quint32 from, quint32 to) {
    Q_ASSERT(from < to);
    Q_ASSERT(to < other->string_length);

    uint len = to - from;
    String * __restrict out = (String*)pool->alloc(sizeof(String)+sizeof(Symbol)*len);
    out->string_length = len;
    out->misc = 0;
    for(uint i = 0; i < len; ++i) {
        out->symbols[i] = S::symtype2(other->symbols[i+from]);
        out->misc = (out->misc<<11) ^ (out->misc>>21) ^ (out->symbols[i]&0xFFFF);
    }
    if (len) out->misc = (out->misc<<HKEY_LEN) | (other->symbols[from] & HKEY_MASK);
    return out;
}
uint String::hash(uint from, uint to) const {
    Q_ASSERT(from < to);
    Q_ASSERT(to < string_length);
    uint q = 0;
    for(uint i = from+1; i < to; ++i) {
        q = (q<<11) ^ (q>>21) ^ (symbols[i]&0xFFFF);
    }
    q = (q<<HKEY_LEN) | (symbols[from] & HKEY_MASK);
    return q;
}
String::Symbol S::sym(QChar c) {
    String::Symbol s = c.unicode();
    if (c >= '0' && c <= '9') {
        s |= (SYM_DIGIT<<16);
    } else if (c.isSpace()) {
        s |= (SYM_SPACE<<16);
    } else if (c.isLetter()) {
        s |= (SYM_LETTER<<16);
    } else switch (s) {
    case '~':
    case '@':
    case '%':
    case '^':
    case '&':
    case '*':
    case '-':
    case '+':
    case '<':
    case '>':
    case '/':
    case '|':
    case '!': s |= (SYM_PUNCT<<16);     break;
    case ':': s |= (SYM_SEMICOLON<<16); break;
    case '.': s |= (SYM_DOT<<16);       break;
    case ',': s |= (SYM_COMMA<<16);     break;
    case '=': s |= (SYM_EQUAL_SIGN<<16);break;
    case '\'':s |= (SYM_STR1<<16);      break;
    case '\"':s |= (SYM_STR2<<16);      break;
    case '$':
    case '_': s |= (SYM_LETTER<<16);    break;
    case '(': s |= (SYM_LBRACKET<<16);  break;
    case ')': s |= (SYM_RBRACKET<<16);  break;
    case '[': s |= (SYM_LINDEX<<16);    break;
    case ']': s |= (SYM_RINDEX<<16);    break;
    case '#': s |= (SYM_COMMENT<<16);   break;
    default:  s = 0;
    }
    return s;
}
void String::decolorify() {
    for(uint i = 0; i < string_length; ++i)
        symbols[i] &= 0x1fffffu;
}
void String::colorify(uint from, uint to, uint attr) {
    to = qMin(string_length, to);
    for(uint i = from; i < to; ++i) {
        symbols[i] &= 0x1fffffu;
        symbols[i] |= attr<<21;
    }
}
void String::set(uint n, Symbol s)
{
    Q_ASSERT(n < string_length);
    symbols[n] = s;
}
void String::set(uint n, QChar c)
{
    Q_ASSERT(n < string_length);
    symbols[n] = S::sym(c);
}
