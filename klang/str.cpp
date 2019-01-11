#include "str.h"
#include "syntaxerrors.h"
#include <string.h>

using namespace K::Lang;
using namespace K::Lang::S;

String * String::alloc(K::EditUtils::AssetPool * pool, uint len) {
    String * out = (String*)pool->alloc(sizeof(String)+sizeof(Symbol)*len);
    out->string_length = len;
    return out;
}
String::Symbol sym(QChar c) {
    String::Symbol s = c.unicode();
    if (c.isDigit()) {
        s |= (SYM_DIGIT<<16);
    } else if (c.isSpace()) {
        s |= (SYM_SPACE<<16);
    } else if (c.isLetter()) {
        s |= (SYM_LETTER<<16);
    } else switch (s) {
    case '.': s |= (SYM_DOT<<16);       break;
    case ',': s |= (SYM_COMMA<<16);     break;
    case '=': s |= (SYM_EQUAL_SIGN<<16);  break;
    case '\'':s |= (SYM_STR1<<16);      break;
    case '\"':s |= (SYM_STR2<<16);      break;
    case '$':
    case '_': s |= (SYM_LETTER<<16);    break;
    case '(': s |= (SYM_LBRACKET<<16);  break;
    case ')': s |= (SYM_RBRACKET<<16);  break;
    case '[': s |= (SYM_LINDEX<<16);    break;
    case ']': s |= (SYM_RINDEX<<16);    break;
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
